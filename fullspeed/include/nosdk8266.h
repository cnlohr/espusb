#ifndef _ROMLIB_H
#define _ROMLIB_H

//Useful, more bloated ROM functions.

#if MAIN_MHZ == 52
	#define PERIPH_FREQ 52
#elif MAIN_MHZ == 104
	#define PERIPH_FREQ 52
#elif MAIN_MHZ == 80
	#define PERIPH_FREQ 80
#elif MAIN_MHZ == 115
	#define PERIPH_FREQ 115
#elif MAIN_MHZ == 173
	#define PERIPH_FREQ 173
#elif MAIN_MHZ == 189
	#define PERIPH_FREQ 189
#elif MAIN_MHZ == 160	
	#define PERIPH_FREQ 160
#elif MAIN_MHZ == 231
	#define PERIPH_FREQ 115
#elif MAIN_MHZ == 320	
	#define PERIPH_FREQ 160
#elif MAIN_MHZ == 346
	#define PERIPH_FREQ 173
#elif MAIN_MHZ == 378  //Won't boot on my ESP.  PLL@5.5 (how?)
	#define PERIPH_FREQ 189
#else
	#error System MHz must be 52, 80, or 160
#endif

extern uint32_t _bss_start;
extern uint32_t _bss_end;
extern volatile uint32_t * DPORT_BASEADDR;// = (volatile uint32_t *)0x3ff00000;
extern volatile uint32_t * PIN_BASE; // = (volatile uint32_t *)0x60000300;
extern volatile uint32_t * IO_BASE;  // = (volatile uint32_t *)0x60000000;
extern volatile uint32_t * IOMUX_BASE; // = (volatile uint32_t *)0x60000800
extern volatile uint32_t * SPI0_BASE; // = (volatile uint32_t *)0x60000200;
extern volatile uint8_t  * RTCRAM; //Pointer to RTC Ram (1024 bytes)

#define HWREG(BASE, OFF)       BASE[OFF>>2]


#ifdef PICO66
//#define pico_i2c_writereg( reg, hostid, par, val ) { asm volatile( "_movi a2, " #reg "\n_movi.n a3, " #hostid "\n_movi.n a4, " #par "\nmovi a5, " #val "\n_call0 pico_i2c_writereg_asm" : : : "a2", "a3", "a4", "a5", "a0" ); }  //Doesn't work.
void pico_i2c_writereg_asm( uint32_t a, uint32_t b);
#define pico_i2c_writereg( reg, hostid, par, val ) pico_i2c_writereg_asm( (hostid<<2) + 0x60000a00 + 0x300, (reg | (par<<8) | (val<<16) | 0x01000000 ) )

#else
#define pico_i2c_writereg rom_i2c_writeReg
void rom_i2c_writeReg( int reg, int hosid, int par, int val ); 
#endif

#define UART0 0




//Sets clock frequency, PLL and initializes BSS.
void nosdk8266_init();

//For pico, or if all you want is to update the clock, call this.
void nosdk8266_clock();

//Zero all global, uninitialized RAM
void nosdk8266_zerobss();

//Configure a GPIO
//GPIO 0: 		PERIPHS_IO_MUX_GPIO0_U    -- FUNC_GPIO0  or FUNC_SPICS2   or FUNC_CLK_OUT
//GPIO 1:		PERIPHS_IO_MUX_U0TXD_U    -- FUNC_GPIO1  or FUNC_U0TXD    or FUNC_SPICS1    or FUNC_CLK_RTC_BK
//GPIO 2:		PERIPHS_IO_MUX_GPIO2_U    -- FUNC_GPIO2  or FUNC_U1TXD_BK or FUNC_I2SO_WS   or FUNC_U0TXD_BK or FUNC_U1TXD_BK 
//GPIO 3:		PERIPHS_IO_MUX_U0RXD_U    -- FUNC_GPIO3  or FUNC_U0RXD    or FUNC_I2SO_DATA or FUNC_CLK_XTAL_BK
//GPIO 4:       PERIPHS_IO_MUX_GPIO4_U    -- FUNC_GPIO4  or FUNC_CLK_XTAL
//GPIO 5:       PERIPHS_IO_MUX_GPIO5_U    -- FUNC_GPIO5  or FUNC_CLK_RTC
//GPIO 6:		PERIPHS_IO_MUX_SD_CLK_U   -- FUNC_GPIO6  or FUNC_SDCLK    or FUNC_SPICLK    or UART1_CTS
//GPIO 7:		PERIPHS_IO_MUX_SD_DATA0_U -- FUNC_GPIO7  or FUNC_SDDATA0  or FUNC_SPIQ_MISO or FUNC_U1TXD or FUNC_UART1_TXD
//GPIO 8:		PERIPHS_IO_MUX_SD_DATA1_U -- FUNC_GPIO8  or FUNC_SDDATA1  or FUNC_SPID_MOSI or FUNC_U1RXD or FUNC_UART1_RXD
//GPIO 9:		PERIPHS_IO_MUX_SD_DATA2_U -- FUNC_GPIO9  or FUNC_SDDATA2  or FUNC_HSPIHD    or FUNC_SPIHD
//GPIO 10:		PERIPHS_IO_MUX_SD_DATA3_U -- FUNC_GPIO10 or FUNC_SDDATA3  or FUNC_SPIWP     or FUNC_HSPIWP
//GPIO 11:		PERIPHS_IO_MUX_SD_CMD_U   -- FUNC_GPIO11 or FUNC_SDCMD    or FUNC_SPICS0    or U1RTS or UART1_RTS
//GPIO 12:		PERIPHS_IO_MUX_MTDI_U     -- FUNC_GPIO12 or FUNC_MTDI     or FUNC_I2SI_DATA or FUNC_HSPIQ_MISO or FUNC_UART0_DTR
//GPIO 13: 		PERIPHS_IO_MUX_MTCK_U     -- FUNC_GPIO13 or FUNC_MTCK     or FUNC_I2SI_BCK  or FUNC_HSPID_MOSI or FUNC_UART0_CTS
//GPIO 14:		PERIPHS_IO_MUX_MTMS_U     -- FUNC_GPIO14 or FUNC_MTMS     or FUNC_I2SI_WS   or FUNC_HSPI_CLK   or FUNC_UART0_DSR
//GPIO 15:		PERIPHS_IO_MUX_MTDO_U     -- FUNC_GPIO15 or FUNC_MTDO     or FUNC_I2SO_BCK  or FUNC_HSPI_CS0   or FUNC_U0RTS or FUNC_UART0_RTS

//XXX TODO:  Do we need to worry about "output" here?
#define nosdk8266_configio( port, FUNC, pd, pu ) \
	IOMUX_BASE[(port-PERIPHS_IO_MUX)/4] = ((((FUNC&BIT2)<<2)|(FUNC&0x3))<<PERIPHS_IO_MUX_FUNC_S) | (pu<<7) | (pd<<6);




#endif
