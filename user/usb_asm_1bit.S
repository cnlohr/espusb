//This file is super trashy and was kind of my first whack at this.
//It should probably get cleaned up.

#include <common.h>

#define _INASM_
#include "usb_table_1bit.h"

#define PIN_OUT        0x60000300
#define PIN_OUT_SET    0x60000304
#define PIN_OUT_CLEAR  0x60000308
#define PIN_DIR        0x6000030C
#define PIN_DIR_OUTPUT 0x60000310
#define PIN_DIR_INPUT  0x60000314
#define PIN_IN         0x60000318
#define _BV(x) ((1)<<(x))


#define DEBUG_HIGH    _s32i.n	a13, a11, GPIO_OFFSET_SET
#define DEBUG_LOW     _s32i.n	a13, a11, GPIO_OFFSET_CLEAR

//Detailed analysis of some useful stuff and performance tweaking: http://naberius.de/2015/05/14/esp8266-gpio-output-performance/
//Reverse engineerd boot room can be helpful, too: http://cholla.mmto.org/esp8266/bootrom/boot.txt
//USB Protocol read from wikipedia: https://en.wikipedia.org/wiki/USB
//Useful information: http://www.usbmadesimple.co.uk/ums_3.htm


#define SIZE_OF_BUFFER 24

.global gpio_intr

gpio_intr:
	_addi a1, a1, -68
	_s32i.n a0, a1, 0
	_s32i.n a2, a1, 4   // Working reg
	_s32i.n a3, a1, 8   //  Running CRC
	_s32i.n a4, a1, 12  // Anding mask
	_s32i.n a5, a1, 16  // Status Word (for table)
	_s32i.n a6, a1, 20  // Current Pins
	_s32i.n a7, a1, 24  // The current byte ready to be written out.
	_s32i.n a8, a1, 28  // Buffer Output Offset
	_s32i.n a9, a1, 32  // Timing
	_s32i.n a10, a1, 36 // Timing-off-by-three (For doing the 53/54 dance)
	_s32i.n a11, a1, 40 // GPIO_BASE
	_s32i.n a12, a1, 44 //  CRC Polynomial
	_s32i.n a13, a1, 48 // Debug Output Pin
	_s32i.n a14, a1, 52 // Main Ramtable
	_s32i.n a15, a1, 56 // Loop Amount

	DISABLE_INTERRUPTS_LCL  	//Uses stack space 60 and 64 

	movi       a14, usb_ramtable	//This is actually very slow.


usb_reinstate: //Come back up here if we are expecting more data.

	//These are for debug.
	_movi.n    a13, 1<<DEBUGPIN
	_movi.n		a0, 0
	ssl a0
	_l32i.n    a15, a14, LOOP_COUNT_OFFSET
	_l32i.n    a11, a14, GPIO_BASE_OFFSET

	_movi      a7, 0 //Set nibble+1, so if we ever get a full byte, it'll breach the 4th bit in the 2nd byte of the register
	_movi      a5, (0x38+4)   //Initial Status Word
	_l32i.n    a4, a14, ANDING_MASK_OFFSET

	_l32i.n    a8, a14, USB_INTERNAL_STATE_OFFSET
	_l32i.n	   a15, a14, RUNNING_TIMEOUT_OFFSET  //# of times to loop.

	DEBUG_HIGH
find_high:
    l32i.n a2, a11, GPIO_OFFSET_INPUT
	addi.n a15, a15, -1
	bbsi a2, DPLUS, done_high
	bnez a15, find_high
	j end_gpio_intr
done_high:
find_low:
    l32i.n a2, a11, GPIO_OFFSET_INPUT
	addi.n a15, a15, -1
	bbci a2, DPLUS, done_low
	bnez a15, find_low
	j end_gpio_intr
done_low:
	DEBUG_LOW
	DEBUG_HIGH

	movi a10, 0
	_rsr a9, ccount

	//Phase shift?  (-53 is OKAY)
	//-12 almost great!... why???
	//-16 better yet...
	//-23 ALMOST works
	//-33 sads a lot
	//-39 sads less
	_addi a9, a9, -52 //TODO: Optimize!!!

	l32i a15, a14, RUNNING_TIMEOUT_OFFSET  //# of times to loop.

looper:
	DEBUG_HIGH
	addi a15, a15, -1
	beqz a15, end_gpio_intr
	addi a9, a9, 53
	addi a10, a10, 1
	blti a10, 3, delayer
	addi a9, a9, 1
	movi a10, 0
delayer:
	rsr a2, ccount
	sub a2, a2, a9
	bbsi a2, 31, delayer


	_l32i.n a6, a11, GPIO_OFFSET_INPUT           //Read pins in.
	DEBUG_LOW
	_and a5, a5, a4
	_extui a2, a6, DMINUSBASE, 2  //Extract two bits.
	_or a5, a5, a2
	_add a2, a14, a5			 //Offset the table
	_l8ui a5, a2, TABLE_OFFSET //Read the data back
	_ssl a0			//  << This is a "free" instruction - we can stick this here.
	_bbsi a5, 7, term            //If terminate bit set, exit.
	_bbci a5, 1, skip_set
		_extui a2, a5, 0, 1
		_sll   a2, a2  			//Shift that bit up to where it needs to be.
		_or a7, a7, a2          //Or it in.
		_addi a0, a0, 1

		//Do our CRC
		xor a2, a5, a3
		_srli   a3, a3, 1
		_bbci  a2, 0, skip_set
		xor a3, a3, a12

	skip_set:

	//See if we have a full byte?
	_blti a0, 8, looper
		_l32i.n    a2, a14, USB_INTERNAL_STATE_OFFSET
		_bne       a2, a8,  write_cont

		//If we're at the first byte, initialize the CRC stuff.
		bbci a7, 1, not_crc16
			_l32i.n		a12,a14, CRC16_POLY_OFFSET
			_l32i.n		a3, a14, CRC16_INIT_OFFSET
			j write_cont
		not_crc16:
			movi.n		a12, CRC5_POLY
			movi.n		a3, CRC5_INITIAL			
	write_cont:
		_s8i a7, a8, USB_OFFSET_BUFFER
		_addi a8, a8, 1
		_addi a0, a0, -8
		_srli a7, a7, 8

	//Jump back to looper anyway.
	j looper
term:
	_l32i.n    a9, a14, USB_INTERNAL_STATE_OFFSET
	l8ui       a7, a9, 0

	movi a5, CRC5_CHECK
	bbci a7, 1, not_crc16_check
		_l32i.n 	a5,  a14, CRC16_CHECK_OFFSET
	not_crc16_check:

	sub a10, a8, a9

	//Check to make sure we have more than just a token.
	blti a10, 2, skip_crc_check

	//Check to see if CRCs match.
	bne a3, a5, end_gpio_intr
skip_crc_check:

	//CRCs Match.  Proceed with protocol.
	mov a3, a9

//	movi a2, 0xface  //Debug
//	s32i		a2, a9, USB_OFFSET_DEBUG

	_s32i.n    a10, a3, USB_OFFSET_PACKET_SIZE
	_l32i.n    a2, a3, USB_OFFSET_BUFFER


	//Set return address
	movi a0, end_gpio_intr  //XXX: TODO: We can play tricks with this to read it from RAM faster.

	bbsi a2, 0, token_or_data
		extui a9, a2, 1, 7
		addi a9, a9, -0b1101001  //Make sure this is an ack.
		beqz a9, handle_ack
		j end_gpio_intr
	token_or_data:
		bbsi a2, 1, data_msg
			extui a9, a2, 2, 6
			addi a8, a9, -0b001011
			beqz a8, handle_setup
			addi a8, a9, -0b101001
			beqz a8, handle_sof
			addi a8, a9, -0b011010
			beqz a8, handle_in
			addi a8, a9, -0b111000
			beqz a8, handle_out		
			j end_gpio_intr //We don't understand this message
		data_msg:
			extui a9, a2, 2, 6
			extui a4, a2, 3, 1
			addi a8, a9, -0b110000
			beqz a8, handle_data
			addi a8, a9, -0b010010
			beqz a8, handle_data
			j end_gpio_intr  //Unmatched message?

end_gpio_intr:


//Can't do this here, since GCC probably clobbered some registers.
//	DEBUG_HIGH
//	DEBUG_LOW

	//Acknowledge interrupt
	//This looks VERY WRONG!

	//UGH!!! GCC Clobbers these registers.  We should consider backing them up.
	movi       a14, usb_ramtable	//This is actually very slow.
	_l32i.n    a11, a14, GPIO_BASE_OFFSET
	_l32i.n a4, a11, GPIO_OFFSET_GPIO_STATUS
	_s32i.n a4, a11, GPIO_OFFSET_GPIO_STATUS_W1TC


	ENABLE_INTERRUPTS_LCL
	_l32i.n a0, a1, 0
	_l32i.n a2, a1, 4
	_l32i.n a3, a1, 8
	_l32i.n a4, a1, 12  // Anding mask
	_l32i.n a5, a1, 16  // Status Word (for table)
	_l32i.n a6, a1, 20  // Current Pins
	_l32i.n a7, a1, 24  // The current byte ready to be written out.
	_l32i.n a8, a1, 28
	_l32i.n a9, a1, 32  // Timing
	_l32i.n a10, a1, 36 // Timing-off-by-three (For doing the 53/54 dance)
	_l32i.n a11, a1, 40 // Pins in
	_l32i.n a12, a1, 44 // Shift Amount
	_l32i.n a13, a1, 48 // Debug Output Address
	_l32i.n a14, a1, 52 // Main Ramtable
	_l32i.n a15, a1, 56 // Loop Amount
	_addi a1, a1, 68
	ret.n


//############################################################################################
//############################################################################################
//############################################################################################
//############################################################################################

usb_send_data:            //A2 = pointer to data  //A3 = length of data,, A4  = (0, do normal CRC, 2, dont do CRC, 3 make empty CRC)
	_addi a1, a1, -68
	//56 = Temporary buffer for holding CRC
	_s32i.n a15, a1, 52   //Timer/Counter
	_s32i.n a14, a1, 48   //Core ramtable
	_s32i.n a13, a1, 44   //Debug
	_s32i.n a12, a1, 40   //[[Current Byte, padded with extra 1 on top]]
	_s32i.n a11, a1, 36   //GPIO Base
	_s32i.n a10, a1, 32   //Timer/Count (adder)
	_s32i.n a9, a1, 24    //Inverted State
	_s32i.n a8, a1, 20    //Positive State
	_s32i.n a7, a1, 16    //"Last Bit"
	_s32i.n a6, a1, 12    //"Number of 1's"
	_s32i.n a5, a1, 8    	//CRC Current
//	_s32i.n a4, a1, 4		//CRC Poly
	_s32i.n a0, a1, 0		//"Work" register

//	movi	a4, 0

	movi       a14, usb_ramtable	//This is actually very slow.
	_movi.n    a13, 1<<DEBUGPIN
	_l32i.n    a11, a14, GPIO_BASE_OFFSET

	movi a0, ~(3<<DMINUSBASE)
	_l32i.n a5, a11, GPIO_OFFSET_OUT
	and a0, a5, a0

	movi a8, (1<<DMINUSBASE)
	or a8, a0, a8
	movi a9, (2<<DMINUSBASE)
	or a9, a0, a9	

	_s32i.n a9, a11, GPIO_OFFSET_OUT

	rsr a15, ccount

	movi a0, (3<<DMINUSBASE)
	_s32i.n    a0, a11, GPIO_OFFSET_DIR_OUT //Set pins to output.

	movi a7, 0
	movi a6, 0

continue_send_data:

	l8ui   a12, a2, 0
	movi a0, 0x100
	or a12, a12, a0
	addi a2, a2, 1
	
	DEBUG_LOW
looper_data:

	//Examine current bit (will be bit0 of a12)

	//Stuff test
	blti a6, 6, skip_stuff_test_one
		//We need to bit stuff.
		movi a6, 0
		movi a0, 1
		xor a7, a0, a7
		j end_looper_bit
	skip_stuff_test_one:

	//If it's a 1, keep a7 "last bit" the same.
	//If not, flip A7... Unless our bit stuffing alarm goes off.


	//TODO: Maybe the CRCs could be condensed.
	bbci	a12, 0, data_bit_0
			addi a6, a6, 1
			xor a0, a12, a5
			srli a12, a12, 1

			//CRC
			_srli   a5, a5, 1 
			_bbci   a0, 0, end_looper_bit
			xor     a5, a5, a4
			j end_looper_bit

	data_bit_0:
			movi a6, 0
			movi a0, 1
			xor a7, a0, a7

			_xor a0, a12, a5
			_srli a12, a12, 1

			//CRC
			_srli   a5, a5, 1 
			_bbci   a0, 0, end_looper_bit
			xor     a5, a5, a4
			j end_looper_bit


end_looper_bit:
	addi a15, a15, 53
	addi a10, a10, 1
	blti a10, 3, emit_data_bit
	addi a15, a15, 1
	movi a10, 0
emit_data_bit:
	rsr a0, ccount
	sub a0, a0, a15
	bbsi a0, 31, emit_data_bit

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
		//slli a12, a12, 1
		l8ui a12, a2, 0
		addi a2, a2, 1
		movi a0, 0x100
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
	_s32i.n a4, a14, 4; //DEbug!!!
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


	addi a15, a15, 53
emit_data_bit_for_starting_end:
	rsr a0, ccount
	sub a0, a0, a15
	bbsi a0, 31, emit_data_bit_for_starting_end


	//Go low/low for two cycles.
	movi a0, (3<<DMINUSBASE)
	_s32i.n    a0, a11, GPIO_OFFSET_CLEAR //Set pins to output.

	addi a15, a15, 107
emit_data_bit_for_starting_end_final:
	rsr a0, ccount
	sub a0, a0, a15
	bbsi a0, 31, emit_data_bit_for_starting_end_final

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



/* Brainstorming... If we did ONE BIT per table pair...

	//Start with the running byte = to 0b00000001


	(l32i) read 32-bit word from pins
	and our "status word" from the table to clear out the 'send' bits
	extui 32-bit word to the two adjacent input bits
	or our two bits with the status word.
	add back in the table begin offset
	(l8ui) look up status word from table
	(OR the extracted bit into the running byte)
	escape jump if needed (bail)

	nop (for now)

	if bit_present
		extui the data bit
		sll running byte, 1

	//Then once in a while
	if running byte > 512
        s8i the running byte
		srli the running byte
		increment table out place
*/


//This is my first set of test speed functions.  Just for fun.
//This lets you test out and see how long code takes to execute in ccount's.

.global my_table
.global my_func

.align 16
my_func:
	_addi a1, a1, -48
	_s32i.n a0, a1, 0
	_s32i.n a2, a1, 4
	_s32i.n a3, a1, 8
	_s32i.n a4, a1, 12  // Anding mask
	_s32i.n a5, a1, 16  // Status Word (for table)
	_s32i.n a6, a1, 20  // Current Pins
	_s32i.n a7, a1, 24
	_s32i.n a8, a1, 28
	//a11 is free		// Pins address
	_s32i.n a12, a1, 32 // Debug Pin offset (don't use)
	_s32i.n a13, a1, 36 // Debug Output Address
	_s32i.n a14, a1, 40 // Main Ramtable
	_s32i.n a15, a1, 44 // Loop Amount

	movi       a14, usb_ramtable	//This is actually very slow.
	_movi.n    a13, 1<<DEBUGPIN
	//_l32i.n    a12, a14, PIN_OUT_SET_OFFSET
//	_l32i.n    a15, a14, LOOP_COUNT_OFFSET
	//_l32i.n    a11, a14, PIN_IN_OFFSET
	_movi      a7, 31
	_movi      a5, 0x3c
	_l32i.n    a4, a14, ANDING_MASK_OFFSET
	_movi      a3, 0
	//TODO: Find edge, set a5 to values on edge.

	//_nop
	//_nop.n

	movi a5, 0xa001
	_movi a6, 1

	_rsr a9, ccount


	mul16u a7, a5, a6
	bbsi a7, 0, skipper
		_nop.n
		_nop
	skipper:

#if 0
retry:
	//DEBUG_HIGH


	//This should be 11 cycles.
	_l32i.n a6, a11, 0           //Read pins in.
	_and a5, a5, a4
	_extui a2, a6, DMINUSBASE, 2  //Extract two bits.
	_or a5, a5, a2
	_add a2, a15, a5			 //Offset the table
	_l8ui a5, a2, TABLE_START_OFFSET //Read the data back
	_or a7, a7, a3               //Wacky that this is here.  Or in the or'ed bit
	_bbsi a5, 7, term            //If terminate bit set, exit.
	_bbci a5, 1, skip_set
		_extui a3, a5, 0, 1
		_slli   a7, a7, 1
	skip_bit:



	_nop;
	_nop;
	_nop;
skipper:
	_nop.n;
	_nop.n;

//	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; _nop.n;
//	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; 

	DEBUG_LOW
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; _nop.n;
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; 

	DEBUG_HIGH
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; _nop.n;
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; 

	DEBUG_LOW
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; _nop.n;
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n;           _nop.n; _nop.n; _nop.n;_nop.n;

	DEBUG_HIGH
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; _nop.n;
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n;

	DEBUG_LOW
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; _nop.n;
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n;

	_addi.n a15, a15, -1
	_bnez a15, retry
term:

#endif

rsr a11, ccount
addi a11, a11, -1

	_l32i.n a0, a1, 0
	_l32i.n a2, a1, 4
	_l32i.n a3, a1, 8
	_l32i.n a4, a1, 12
	_l32i.n a5, a1, 16
	_l32i.n a6, a1, 20
	_s32i.n a7, a1, 24
	_s32i.n a8, a1, 28
	_l32i.n a12, a1, 32 // Debug Pin offset (don't use)
	_l32i.n a13, a1, 36 // Debug Output Address
	_l32i.n a14, a1, 40 // Main Ramtable
	_l32i.n a15, a1, 44 // Loop Amount
	_addi a1, a1, 48
	ret.n











#if 0

//This is my first set of test speed functions.  Just for fun.
//This lets you test out and see how long code takes to execute in ccount's.

.global my_table
.global my_func

.align 16
my_func:
	_addi a1, a1, -32
	_s32i.n a0, a1, 0
	_s32i.n a2, a1, 4
	_s32i.n a3, a1, 8
	_s32i.n a12, a1, 12
	_s32i.n a13, a1, 16
	_s32i.n a14, a1, 20
	_s32i.n a15, a1, 24

	movi    a14, usb_ramtable	//This is actually very slow.
	_movi.n    a13, 1<<DEBUGPIN
	_l32i.n    a12, a14, PIN_OUT_SET_OFFSET
	movi a15, 0
//At 80
//49 = 1179
//50 = 1203  (12 cycles) (DBH + DBL)
//49 = 591 (6 cycles)
//50 = 603 (DBL only)
//49 = 425 (NOTHING 5 cycles)
//At 160
//49 = 1138 / 50 = 1162 (DBL only -> 12 cycles)
//Both
//49 = 2314 / 50 = 2362 (DBL+DBH = 24 cycles
	_movi.n a3, 50
	_slli a3, a3, 1
	_nop;_nop.n;
	_rsr a9, ccount
	_nop; 
retry:
	DEBUG_HIGH
	_l32i.n a11, a14, 0
	_beqz a14, term //a11 was created.
	_add.n a11, a14, a3
//	_l8ui a2, a15, TABLE_OFFSET
	_l8ui a2, a11, TABLE_OFFSET-4
	_l8ui a2, a11, TABLE_OFFSET
	_add.n a11, a11, a11
	//_nop.n;  //WARNING: THIS IS NEEDED
	_bbsi a2, 0, skipper
	_nop;
	_nop;
	_nop;
skipper:
	_nop.n;
	_nop.n;

//	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; _nop.n;
//	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; 

	DEBUG_LOW
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; _nop.n;
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; 

	DEBUG_HIGH
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; _nop.n;
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; 

	DEBUG_LOW
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; _nop.n;
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n;           _nop.n; _nop.n; _nop.n;_nop.n;

	DEBUG_HIGH
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; _nop.n;
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n;

	DEBUG_LOW
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n; _nop.n;
	_nop.n; _nop.n; _nop.n; _nop.n; _nop.n;

	_addi.n a3, a3, -1
	_bnez a3, retry
term:
rsr a11, ccount

	l32i.n a15, a1, 24
	l32i.n a14, a1, 20
	l32i.n a13, a1, 16
	l32i.n a12, a1, 12
	l32i.n a3, a1, 8
	l32i.n a2, a1, 4
	l32i.n a0, a1, 0
	addi a1, a1, 32
	ret.n

#endif


