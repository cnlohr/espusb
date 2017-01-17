.global delay4clk
.align 4
delay4clk:
	_addi.n a2, a2, -1
	bnez a2, delay4clk
	_ret.n

//Look-alike function to rom_i2c_writeReg
#if MAIN_MHZ==52 || MAIN_MHZ==104

//No pico_i2c_writereg_asm isneeded if no PLL used.

#else

//This is similar to the i2c_writereg found in the ESP8266's ROM, however, we frontload a lot of the mathematics into the compiler.

.global pico_i2c_writereg_asm
.align 4
pico_i2c_writereg_asm:
	_s32i.n	a3, a2, 0
	_memw
	_l32i.n	a3, a2, 0
	bbci	a3, 25, .term_pico_writereg
.reloop_pico_writereg:
	_memw
	_l32i.n	a3, a2, 0
	bbsi	a3, 25, .reloop_pico_writereg
.term_pico_writereg:
	_ret.n

#endif



//An example function

.global myfunc

myfunc:
	addi a1, a1, -68
	s32i.n a0, a1, 0   // Working reg
	s32i.n a2, a1, 4   // Running byte
	s32i.n a3, a1, 8   // Running CRC
	s32i.n a4, a1, 12  // Anding mask

	movi a0, 320000
	movi a2, PIN_BASE
	l32i a4, a2, 0
keep_goingA:
	//At 80 MHz, this instruction takes 12 cycles
	//At 160 MHz, this instruction takes 12 cycles
	//At 320 MHz, this instruction takes 16 cycles
	//At 346.6666 MHz, this is 16 cycles.  -> We have 28.8888 cycles
	
	//l32i a3, a4, 0
//	l8ui a3, a2, 0  //1 cycle!!!
	l32i a3, a4, 7*4

	addi a0, a0, -1
	bnez a0, keep_goingA


	l32i.n a0, a1, 0
	l32i.n a2, a1, 4
	l32i.n a3, a1, 8
	l32i.n a4, a1, 12
	addi a1, a1, 68
	ret




//An example function

.global setreset

setreset:
	addi a1, a1, -68
	s32i.n a0, a1, 0   // Working reg
	s32i.n a2, a1, 4   // Running byte
	s32i.n a3, a1, 8   // Running CRC
	s32i.n a4, a1, 12  // Anding mask

	movi a0, 320000
	movi a2, PIN_BASE
	l32i a4, a2, 0
keep_going:
	//At 80 MHz, this instruction takes 12 cycles
	//At 160 MHz, this instruction takes 12 cycles
	//At 320 MHz, this instruction takes 16 cycles
	//At 346.6666 MHz, this is 16 cycles.  -> We have 28.8888 cycles
	
	l32i a3, a4, 7*4
	//l8ui a3, a2, 0  //1 cycle!!!

	addi a0, a0, -1
	bnez a0, keep_going


	l32i.n a0, a1, 0
	l32i.n a2, a1, 4
	l32i.n a3, a1, 8
	l32i.n a4, a1, 12
	addi a1, a1, 68
	ret







.global NMIExceptionHandler
NMIExceptionHandler:
	addi a1, a1, -68
	s32i.n a0, a1, 0
	s32i.n a2, a1, 4
	s32i.n a3, a1, 8
	s32i.n a4, a1, 12

	movi a0, 320
	movi a2, PIN_BASE
	l32i a4, a2, 0
	movi a3, (1<<2)
nmiloop:
	s32i a3, a4, 1*4  //"set"
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	s32i a3, a4, 2*4  //"clear"
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	addi a0, a0, -1
	bnez a0, nmiloop


	l32i.n a0, a1, 0
	l32i.n a2, a1, 4
	l32i.n a3, a1, 8
	l32i.n a4, a1, 12
	addi a1, a1, 68
	rfi     3

.global UserExceptionHandler
UserExceptionHandler:
	addi a1, a1, -68
	s32i.n a0, a1, 0
	s32i.n a2, a1, 4
	s32i.n a3, a1, 8
	s32i.n a4, a1, 12


/*
	rsil a0, 15;		//I don't think this is needed.  (Disable interrupts)
	s32i a0, a1, 60;
	rsr a0, SAR;
	s32i a0, a1, 64;
*/
	movi a0, 8
	movi a2, PIN_BASE
	l32i a4, a2, 0
	movi a3, (1<<2)

userloop:
	s32i a3, a4, 1*4  //"set"
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	s32i a3, a4, 2*4  //"clear"
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	addi a0, a0, -1
	bnez a0, userloop


#define GPIO_OFFSET_GPIO_STATUS  0x1c
#define GPIO_OFFSET_GPIO_STATUS_W1TC 0x24
	movi a2, 0x60000300

	_l32i.n a4, a2, GPIO_OFFSET_GPIO_STATUS
	_s32i.n a4, a2, GPIO_OFFSET_GPIO_STATUS_W1TC

/*
	//Enable interrupts
	l32i a0, a1, 64;
	wsr a0, SAR;
	isync;
	l32i a0, a1, 60;  //I think this is not needed, if we don't do the rsil at the beginning.
	wsr a0, ps;
	isync;
*/
	l32i.n a0, a1, 0
	l32i.n a2, a1, 4
	l32i.n a3, a1, 8
	l32i.n a4, a1, 12
	addi a1, a1, 68
//ret
	rfe

