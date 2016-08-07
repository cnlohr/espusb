#include <common.h>

#define _INASM_
#include "usb_table.h"

#define PIN_OUT        0x60000300
#define PIN_OUT_SET    0x60000304
#define PIN_OUT_CLEAR  0x60000308
#define PIN_DIR        0x6000030C
#define PIN_DIR_OUTPUT 0x60000310
#define PIN_DIR_INPUT  0x60000314
#define PIN_IN         0x60000318
#define _BV(x) ((1)<<(x))


#define DEBUG_HIGH    _s32i.n	a13, a12, 0
#define DEBUG_LOW     _s32i.n	a13, a12, 4
//(PIN_OUT_CLEAR - PIN_OUT_SET)

//Long versions of the commands to help with byte-alignment? Maybe?
#define DEBUG_HIGH_L  .byte 0xd2; .byte 0x6c; .byte 0x00
#define DEBUG_LOW_L   .byte 0xd2; .byte 0x6c; .byte 0x01
//#define NOP_L			.byte 0xf0;	.byte 0x20;	.byte 0x00
//Don't use NOP_L use _nop

//4010113f:	1cd9         	1.n	a13, a12, 4
//40101141:	0cd9      	s32i.n	a13, a12, 0


//Detailed analysis of some useful stuff and performance tweaking: http://naberius.de/2015/05/14/esp8266-gpio-output-performance/
//Reverse engineerd boot room can be helpful, too: http://cholla.mmto.org/esp8266/bootrom/boot.txt
//USB Protocol read from wikipedia: https://en.wikipedia.org/wiki/USB

#define SIZE_OF_BUFFER 24
.global usb_buffer
#define usb_buffer_end (usb_buffer + SIZE_OF_BUFFER)

.global usb_buff_data
.global gpio_intr

gpio_intr:
	addi	a1, a1, -60  //Must be 4 higher than highest pusher
	s32i.n	a0, a1, 52
	s32i.n	a2, a1, 48
	s32i.n	a3, a1, 44
	s32i.n	a8, a1, 24
	s32i.n	a9, a1, 20
	s32i.n	a10, a1, 16
	s32i.n	a11, a1, 12 
	s32i.n	a12, a1, 8    //debug pin address
	s32i.n	a13, a1, 4    //debug pin
	s32i.n	a14, a1, 0    //usb_ramtable

	movi    a14, usb_ramtable	//This is actually very slow.
	movi    a13, 1<<DEBUGPIN

	DISABLE_INTERRUPTS_LCL

	l32i    a12, a14, PIN_OUT_SET_OFFSET

	//Acknowledge interrupt
	l32i.n a2, a14, GPIO_STATUS_OFFSET
	l32i.n a4, a2, 0
	s32i.n a4, a2, (GPIO_STATUS_W1TC - GPIO_STATUS)

	movi a9, 512
rkloop:
	nop
	DEBUG_HIGH
	nop
	addi a9, a9, -1
	nop
	DEBUG_LOW
	nop
	bnez a9, rkloop

/*
	movi a9, 1023 #timeout
find_high:
    l32i.n a2, a11, 0
	bbsi a2, DPLUS, done_high
	addi.n a9, a9, -1
	bnez a9, find_high
	j end_gpio_intr



	DEBUG_LOW

	//TODO: Make a mode that waits for data to finish out, despite there being an error.
	movi a0, usb_buff_data
	movi a2, 0xffffffff
	s32i a0, a2, 0
end_intr_with_extra_vars:
	l32i.n	a4, a1, 44
	l32i.n	a5, a1, 40
	l32i.n	a6, a1, 36
	l32i.n	a7, a1, 32
	l32i.n	a8, a1, 28

end_gpio_intr:
*/

	DEBUG_LOW
	DEBUG_LOW
	DEBUG_LOW
	DEBUG_LOW

	ENABLE_INTERRUPTS_LCL

	l32i.n	a0, a1, 52
	l32i.n	a2, a1, 48
	l32i.n	a3, a1, 44
	l32i.n	a8, a1, 24
	l32i.n	a9, a1, 20
	l32i.n	a10, a1, 16
	l32i.n	a11, a1, 12
	l32i.n	a12, a1, 8
	l32i.n	a13, a1, 4
	l32i.n	a14, a1, 0
	addi	a1, a1, 60
	ret.n

/* Brainstorming... Two bits per table?

	//Assume 26 cycles per bit pair.
	read in value
	[debug cycle]
	extui the bits into A2


	read in value
	or A2 with table bits
	extui the bits
	shift the data to A2
	or A2 with the table data


*/

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
	_l32i.n    a12, a14, PIN_OUT_SET_OFFSET
	_l32i.n    a15, a14, LOOP_COUNT_OFFSET
	_l32i.n    a11, a14, PIN_IN_OFFSET
	_movi      a7, 31
	_movi      a5, 0x3c
	_l32i.n    a4, a14, ANDING_MASK_OFFSET
	_movi      a3, 0
	//TODO: Find edge, set a5 to values on edge.

	//_nop
	//_nop.n

	_rsr a9, ccount
retry:
	//DEBUG_HIGH


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
rsr a11, ccount

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










/* Brainstorming... If we did ONE BIT per table pair...

	//Start with the running byte = to 0b00000001


	//This is 11 cycles.
	_l32i.n a6, a11, 0           //Read pins in.
	_and a5, a5, a4
	_extui a2, a6, __, 2         //Extract two bits.
	_or a5, a5, a2
	_add a2, a15, a5			 //Offset the table
	_l8ui a5, a2, TABLE_START_OFFSET //Read the data back
	_or a7, a7, a3               //Wacky that this is here.  Or in the or'ed bit
	_bbsi a5, 7, term            //If terminate bit set, exit.
	_bbci a5, 1, skip_set
		_extui a3, a5, 0, 1
		_slli   a7, a7, 1
	skip_bit:



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


#if 0
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
	_l32i.n    a12, a14, PIN_OUT_SET_OFFSET
	_l32i.n    a15, a14, LOOP_COUNT_OFFSET
	_l32i.n    a11, a14, PIN_IN_OFFSET
	_movi      a7, 31
	_movi      a5, 0x3c
	_l32i.n    a4, a14, ANDING_MASK_OFFSET
	_movi      a3, 0
	//TODO: Find edge, set a5 to values on edge.

	//_nop
	//_nop.n

	_rsr a9, ccount
retry:
	//DEBUG_HIGH


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
rsr a11, ccount

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

#endif

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


