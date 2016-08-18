/*
   (C) 2016 <>< Charles Lohr, Under the Espressif modified MIT license.

	This is the assembly file that drives the low level bits for the ESP8266.
	Global symbols used in this file:
		* usb_ramtable = Created by table (in the tabler) folder.  Must be loaded into dram to work.

	Global functions that must be implemented elsewhere.
		* handle_setup
		* handle_sof
		* handle_in
		* handle_out
		* handle_data

	Provided symbols include:
		* gpio_intr = Interrupt to be called on the rising edge of D-.
		* usb_send_data = Send a USB packet back to the host.
*/

#include <common.h>

#define _INASM_
#include "usb_table_1bit.h"


#ifdef DEBUGPIN
#define DEBUG_HIGH    _s32i.n	a13, a11, GPIO_OFFSET_SET
#define DEBUG_LOW     _s32i.n	a13, a11, GPIO_OFFSET_CLEAR
#else
#define DEBUG_HIGH
#define DEBUG_LOW
#endif

//				8 works, 9 is spotty at best.
//				-26 works, 27 is spotty at best.
//				Optimal spot: -9
#define PHASE_DELAY     -9
#define DELAY_ONE_USB_BIT	call0 util_wait_usb_ccount

//We handle advancing the timer by one tick in here.
//Because 80 MHz does not divide evenly into 1.5 MHz, we have to advance by
//53 ticks, 53 ticks, then 54 ticks.  we use a10 to keep track of which bit we're on.
//a15 is the actual time (in ccount (clock counts)) that we're at.  And, a6 is a trash
//variable.  Don't expect it to stick around.
.align 4
util_wait_usb_ccount:
	_addi a15, a15, 53  //Advance 53 ticks
	_addi.n a10, a10, 1
	_blti a10, 3, delayer_ccount //See if we need to add another tick
	_addi.n a15, a15, 1 //If so, add the tick and clear out the no-tick counter
	_movi.n a10, 0
delayer_ccount:
	_rsr a6, ccount
	_sub a6, a6, a15
	_bbsi a6, 31, delayer_ccount
	_ret.n


//############################################################################################
//############################################################################################
//############################################################################################
//############################################################################################


//Detailed analysis of some useful stuff and performance tweaking: http://naberius.de/2015/05/14/esp8266-gpio-output-performance/
//Reverse engineerd boot room can be helpful, too: http://cholla.mmto.org/esp8266/bootrom/boot.txt
//USB Protocol read from wikipedia: https://en.wikipedia.org/wiki/USB
//Useful information: http://www.usbmadesimple.co.uk/ums_3.htm


#define SIZE_OF_BUFFER 24

.global gpio_intr
.align 4
gpio_intr:
	_addi a1, a1, -68
	_s32i.n a0, a1, 0   // Working reg
	_s32i.n a2, a1, 4   // Running byte
	_s32i.n a3, a1, 8   // Running CRC
	_s32i.n a4, a1, 12  // Anding mask
	_s32i.n a5, a1, 16  // Status Word (for table)
	_s32i.n a6, a1, 20  // A Working register)
	_s32i.n a7, a1, 24  // The current byte ready to be written out.
	_s32i.n a8, a1, 28  // Buffer Output Offset
	_s32i.n a9, a1, 32  // Loop Amount
	_s32i.n a10, a1, 36 // Timing-off-by-three (For doing the 53/54 dance) (used in util_wait_usb_ccount)
	_s32i.n a11, a1, 40 // GPIO_BASE
	_s32i.n a12, a1, 44 // CRC Polynomial
	_s32i.n a13, a1, 48 // Debug Output Pin
	_s32i.n a14, a1, 52 // Main Ramtable
	_s32i.n a15, a1, 56 // Timing (used in util_wait_usb_ccount)

	//Disable Interrupts
	rsil a0, 15;		//I don't think this is needed.
	s32i a0, a1, 60;
	rsr a0, SAR;
	s32i a0, a1, 64;

	//Load the table.
	movi       a14, usb_ramtable	//This is actually very slow.


usb_reinstate: //Come back up here if we are expecting more data.

	//These are for debug.
#ifdef DEBUGPIN
	_movi.n    a13, 1<<DEBUGPIN
#endif
	_movi.n		a2, 0
	ssl a2

	//Here, we load values with their entries from the table.
	//We do this because it's so darned fast.  Each of these actually only takes one cycle.
	//We also do it this way because Xtensa doesn't have any movi, XXX for a 32-bit value.
	_l32i.n    a9, a14, LOOP_COUNT_OFFSET
	_l32i.n    a11, a14, GPIO_BASE_OFFSET

	_movi      a7, 0 //Set nibble+1, so if we ever get a full byte, it'll breach the 4th bit in the 2nd byte of the register
	_movi      a5, (0x38)   //Initial Status Word  (Add 4 if you want to change the initially-high or low value)
	_l32i.n    a4, a14, ANDING_MASK_OFFSET

	_l32i.n    a8, a14, USB_INTERNAL_STATE_OFFSET
	_l32i.n	   a9, a14, RUNNING_TIMEOUT_OFFSET  //# of times to loop.

	DEBUG_HIGH
find_high:	//Search for when the pin is high.
    l32i.n a6, a11, GPIO_OFFSET_INPUT
	addi.n a9, a9, -1
	bbsi a6, DPLUS, done_high
	bnez a9, find_high
	j end_gpio_intr
done_high:
find_low:	//Once we find it high, we need to look for the falling edge so we can sync our clocks.
    l32i.n a6, a11, GPIO_OFFSET_INPUT
	addi.n a9, a9, -1
	bbci a6, DPLUS, done_low
	bnez a9, find_low
	j end_gpio_intr
done_low:


	movi a10, 0
	_rsr a15, ccount

//	DEBUG_LOW
//	DEBUG_HIGH

	_addi a15, a15, PHASE_DELAY

	l32i a9, a14, RUNNING_TIMEOUT_OFFSET  //# of times to loop.

looper:
	DEBUG_HIGH
	addi a9, a9, -1
	beqz a9, end_gpio_intr


	DELAY_ONE_USB_BIT


	_l32i.n a6, a11, GPIO_OFFSET_INPUT           //Read pins in.
	DEBUG_LOW
	_and a5, a5, a4
	_extui a6, a6, DMINUSBASE, 2	//Extract two bits.
	_or a5, a5, a6
	_add a6, a14, a5				//Offset the table
	_l8ui a5, a6, TABLE_OFFSET		//Read the data back
	_ssl a2							//  << This is a "free" instruction - we can stick this here.
	_bbsi a5, 7, term				//If terminate bit set, exit.
	_bbci a5, 1, skip_set
		_extui a6, a5, 0, 1
		_sll   a6, a6				//Shift that bit up to where it needs to be in our temporary word register
		_or a7, a7, a6				//Or it in.
		_addi a2, a2, 1

		//Do our CRC
		xor a6, a5, a3
		_srli   a3, a3, 1
		_bbci  a6, 0, skip_set
		xor a3, a3, a12

	skip_set:

	//See if we have a full byte?
	_blti a2, 8, looper
		_l32i.n    a6, a14, USB_INTERNAL_STATE_OFFSET
		_beq       a6, a8,  first_byte //Check to see if this is the first byte.
		_addi      a6, a6, (USB_BUFFERSIZE-1)
		_blt	   a8, a6,  write_cont //Make sure there's not an overflow.
		_j 		end_gpio_intr

	first_byte:
		//If we're at the first byte, initialize the CRC stuff.
		bbci a7, 1, not_crc16  //Tricky: Look at bit 1 in the PID.  If it's 1, then its a DATA0 or DATA1 packet, each of which we have to calculate the CRC16 on.
			_l32i.n		a12,a14, CRC16_POLY_OFFSET
			_l32i.n		a3, a14, CRC16_INIT_OFFSET
			j write_cont
		not_crc16:
			movi.n		a12, CRC5_POLY
			movi.n		a3, CRC5_INITIAL

	write_cont:
		//TODO: Should we make sure we don't run off the end of the buffer?
		_s8i a7, a8, USB_OFFSET_BUFFER
		_addi a8, a8, 1
		_addi a2, a2, -8
		_srli a7, a7, 8

	//Jump back to looper anyway.
	j looper
term:
	_l32i.n    a15, a14, USB_INTERNAL_STATE_OFFSET
	l8ui       a7, a15, 0

	//The way USB works, if you run the packet and the CRC into the CRC algorithm,
	//the CRC will always be the same output.  Therefore we check against that new
	//CRC and if it matches we know we have a good packet!
	movi a5, CRC5_CHECK
	bbci a7, 1, not_crc16_check
		_l32i.n 	a5,  a14, CRC16_CHECK_OFFSET
	not_crc16_check:

	//Compute the total length of the message
	sub a10, a8, a15

	//Check to make sure we have more than just a token.
	blti a10, 2, skip_crc_check

	//Check to see if CRCs match.
	bne a3, a5, end_gpio_intr
skip_crc_check:

	//CRCs Match.  Proceed with protocol.
	mov a3, a15

//	movi a0, 0xface  //Debug
//	s32i		a0, a15, USB_OFFSET_DEBUG

	_s32i.n    a10, a3, USB_OFFSET_PACKET_SIZE
	_l32i.n    a2, a3, USB_OFFSET_BUFFER


	//Set return address for the following calls.  That way we can skip the rest of the code.
	movi a0, end_gpio_intr  //XXX: TODO: We can play tricks with this to read it from RAM faster.

	bbsi a2, 0, token_or_data
		extui a15, a2, 1, 7
		addi a15, a15, -0b1101001  //Make sure this is an ack.
		beqz a15, handle_ack
		j end_gpio_intr
	token_or_data:
		bbsi a2, 1, data_msg        //Jump to the correct C function, don't call!  We already put the return address in A0.
			extui a15, a2, 2, 6
			addi a8, a15, -0b001011
			beqz a8, handle_setup
			addi a8, a15, -0b101001
			beqz a8, handle_sof
			addi a8, a15, -0b011010
			beqz a8, handle_in
			addi a8, a15, -0b111000
			beqz a8, handle_out		
			j end_gpio_intr //We don't understand this message
		data_msg:
			extui a15, a2, 2, 6
			extui a4, a2, 3, 1
			addi a8, a15, -0b110000
			beqz a8, handle_data
			addi a8, a15, -0b010010
			beqz a8, handle_data
			j end_gpio_intr  //Unmatched message?

	//No code here should be called.

end_gpio_intr:

	//Warning: Right here, GCC likely has clobbered a bunch of our registers so
	//be careful what you do.

	//This code acknowledges the interrupt.  I think it looks wrong, but it seems to work...
	movi       a14, usb_ramtable
	_l32i.n    a11, a14, GPIO_BASE_OFFSET
	_l32i.n a4, a11, GPIO_OFFSET_GPIO_STATUS
	_s32i.n a4, a11, GPIO_OFFSET_GPIO_STATUS_W1TC


	//Enable interrupts
	l32i a0, a1, 64;
	wsr a0, SAR;
	isync;
	l32i a0, a1, 60;  //I think this is not needed, if we don't do the rsil at the beginning.
	wsr a0, ps;
	isync;

	//Return from the call.
	_l32i.n a0, a1, 0
	_l32i.n a2, a1, 4
	_l32i.n a3, a1, 8
	_l32i.n a4, a1, 12
	_l32i.n a5, a1, 16
	_l32i.n a6, a1, 20
	_l32i.n a7, a1, 24
	_l32i.n a8, a1, 28
	_l32i.n a9, a1, 32
	_l32i.n a10, a1, 36
	_l32i.n a11, a1, 40
	_l32i.n a12, a1, 44
	_l32i.n a13, a1, 48
	_l32i.n a14, a1, 52
	_l32i.n a15, a1, 56
	_addi a1, a1, 68
	ret.n

//############################################################################################
//############################################################################################
//############################################################################################
//############################################################################################

.global usb_send_data
.align 4
usb_send_data:            //A2 = pointer to data  //A3 = length of data,, A4  = (0, do normal CRC, 2, dont do CRC, 3 make empty CRC)
	_addi a1, a1, -68  //Extra room because we will store the CRC on the stack.
	_s32i.n a15, a1, 52   //Timer/Counter
	_s32i.n a14, a1, 48   //Core ramtable
	_s32i.n a13, a1, 44   //"Number of 1's"
	_s32i.n a12, a1, 40   //[[Current Byte, padded with extra 1 on top]]
	_s32i.n a11, a1, 36   //GPIO Base
	_s32i.n a10, a1, 32   //Timer/Count (adder)
	_s32i.n a9, a1, 24    //Inverted State
	_s32i.n a8, a1, 20    //Positive State
	_s32i.n a7, a1, 16    //"Last Bit"
	_s32i.n a6, a1, 12    //Work Register
	_s32i.n a5, a1, 8    	//CRC Current
//	_s32i.n a4, a1, 4		//CRC Poly  (This is actually passed in)
	_s32i.n a0, a1, 0		//"Work" register

	movi       a14, usb_ramtable	//This is actually very slow.
	_l32i.n    a11, a14, GPIO_BASE_OFFSET

	movi a0, ~(3<<DMINUSBASE)
	_l32i.n a5, a11, GPIO_OFFSET_OUT
	and a0, a5, a0

	movi a8, (2<<DMINUSBASE)		//WARNING: If flipping D- and D+ this must be updated.
	or a8, a0, a8
	movi a9, (1<<DMINUSBASE)
	or a9, a0, a9	

	_s32i.n a9, a11, GPIO_OFFSET_OUT

	rsr a15, ccount

	movi a0, (3<<DMINUSBASE)  //TODO: Pull these from the table.
	_s32i.n    a0, a11, GPIO_OFFSET_DIR_OUT //Set pins to output.

	movi a7, 0
	movi a13, 0

continue_send_data:

	l8ui   a12, a2, 0
	movi a0, 0x100
	or a12, a12, a0
	addi a2, a2, 1
	
	DEBUG_LOW
looper_data:

	//Examine current bit (will be bit0 of a12)

	//Stuff test
	blti a13, 6, skip_stuff_test_one
		//We need to bit stuff.
		movi a13, 0
		movi a0, 1
		xor a7, a0, a7
		j end_looper_bit
	skip_stuff_test_one:

	//If it's a 1, keep a7 "last bit" the same.
	//If not, flip A7... Unless our bit stuffing alarm goes off.

	bbci	a12, 0, data_bit_0
		addi a13, a13, 1
		xor a0, a12, a5

		srli a12, a12, 1

		j do_the_crc_bit

	data_bit_0:
		movi a13, 0
		movi a0, 1

		xor a7, a0, a7
		_xor a0, a12, a5

		_srli a12, a12, 1

do_the_crc_bit:
	//CRC
	_srli   a5, a5, 1 
	_bbci   a0, 0, end_looper_bit
	xor     a5, a5, a4


end_looper_bit:

	DELAY_ONE_USB_BIT

	//It seems odd, but we do this after the wait so we get precise timing.

	//Output the actual bit.
	bbsi a7, 0, is_high_usr_data
		_s32i.n a9, a11, GPIO_OFFSET_OUT
		j skip_high_usr_data
is_high_usr_data:
		_s32i.n a8, a11, GPIO_OFFSET_OUT
skip_high_usr_data:	

	//Check to see if we need to read another byte
	bnei a12, 1, looper_data
		//Yep, need another byte.
		addi a3, a3, -1
		beqz a3, done_data

		l8ui a12, a2, 0
		addi a2, a2, 1
		movi a0, 0x100  //Put a bit in the 256ths place so we can tell when we've run out of bits.  This way we can avoid a counter
		or a12, a12, a0
		bnei a4, 1, not_one
			_l32i.n		a4, a14, CRC16_POLY_OFFSET
			_l32i.n		a5, a14, CRC16_INIT_OFFSET
		not_one:
		bnei a4, 0, not_zero
			movi a4, 1
		not_zero:

	j looper_data
done_data:

	blti a4, 3, actually_done

	bgei a4, 4, dont_do_zero_crc
	movi a5, 0xffff
dont_do_zero_crc:

	//Make more data.
	//Move the CRC into the data we need to send
	movi a0, 0xffff
	xor a5, a0, a5
	_s32i.n a5, a1, 56
	addi a2, a1, 56  //Pointer on stack
	movi a3, 2		//Two-byte CRC
	movi a4, 2		//Tell it not to compute CRC on this data.

	j continue_send_data
actually_done:



	DELAY_ONE_USB_BIT

	//Super tricky: If we have to bit stuff the last bit, do it here.
	//http://www.ti.com/lit/an/spraat5a/spraat5a.pdf 7.1.9
	blti a13, 6, done_bit_stuff
		movi a0, -1
		xor a7, a0, a7
		bbsi a7, 0, is_high_usr_data_bit_stuff
			_s32i.n a9, a11, GPIO_OFFSET_OUT
			j skip_high_usr_data_bit_stuff
			is_high_usr_data_bit_stuff:
				_s32i.n a8, a11, GPIO_OFFSET_OUT
			skip_high_usr_data_bit_stuff:	
		DELAY_ONE_USB_BIT
	done_bit_stuff:

	//Go low/low for two cycles.
	movi a0, (3<<DMINUSBASE)
	_s32i.n    a0, a11, GPIO_OFFSET_CLEAR //Set pins to output.

	addi a15, a15, 53 //Wait an extra cycle, so our SE0 will be two bits.
	DELAY_ONE_USB_BIT

	_s32i.n a9, a11, GPIO_OFFSET_OUT


	addi a15, a15, 54
emit_data_bit_for_starting_end_final_final:
	rsr a0, ccount
	sub a0, a0, a15
	bbsi a0, 31, emit_data_bit_for_starting_end_final_final
DEBUG_HIGH

	movi a0, (3<<DMINUSBASE)
	_s32i.n    a0, a11, GPIO_OFFSET_DIR_IN //Set pins to output.

	//56 = Temporary buffer for holding CRC
	_l32i.n a15, a1, 52
	_l32i.n a14, a1, 48
	_l32i.n a13, a1, 44
	_l32i.n a12, a1, 40
	_l32i.n a11, a1, 36
	_l32i.n a10, a1, 32
	_l32i.n a9, a1, 24
	_l32i.n a8, a1, 20
	_l32i.n a7, a1, 16
	_l32i.n a6, a1, 12
	_l32i.n a5, a1, 8
//	_l32i.n a4, a1, 4
	_l32i.n a0, a1, 0
	_addi a1, a1, 68

	ret.n


