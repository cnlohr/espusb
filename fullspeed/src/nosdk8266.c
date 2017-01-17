#include <esp8266_auxrom.h>
#include <uart_dev.h>
#include <eagle_soc.h>
#include <ets_sys.h>
#include "nosdk8266.h"

extern UartDevice UartDev;

#define	sys_const_crystal_26m_en	48	// soc_param0: 0: 40MHz, 1: 26MHz, 2: 24MHz

volatile uint32_t * DPORT_BASEADDR = (volatile uint32_t *)0x3ff00000;
volatile uint32_t * PIN_BASE = (volatile uint32_t *)0x60000300;
volatile uint32_t * IO_BASE = (volatile uint32_t *)0x60000000;
volatile uint32_t * IOMUX_BASE = (volatile uint32_t *)0x60000800;
volatile uint32_t * SPI0_BASE = (volatile uint32_t *)0x60000200;
volatile uint8_t  * RTCRAM = (volatile uint8_t *)0x60001000; //Pointer to RTC Ram (1024 bytes)

#ifdef PICO66
#define t_ets_update_cpu_frequency(x)
#else
#define t_ets_update_cpu_frequency ets_update_cpu_frequency
#endif

//Thanks, https://github.com/pvvx/esp8266web/blob/2e25559bc489487747205db2ef171d48326b32d4/app/sdklib/system/app_main.c
static void set_pll(void)
{
	//It is safe to assume the system is operating on a 26 MHz crystal instead of a 40 MHz crystal.
//	if(rom_i2c_readReg(103,4,1) != 136) { // 8: 40MHz, 136: 26MHz
		//if(get_sys_const(sys_const_crystal_26m_en) == 1) { // soc_param0: 0: 40MHz, 1: 26MHz, 2: 24MHz
	//Switch to 80 first.

	//This is actually 0x88.  You can set this to 0xC8 to double-overclock the low-end bus.
	//This can get you to a 160 MHz peripheral bus if setting it normally to 80 MHz.
#if PERIPH_FREQ == 160
	pico_i2c_writereg(103,4,1,0xc8);
#else
	pico_i2c_writereg(103,4,1,0x88);
#endif

	//	... Looks like the main PLL operates at 1040 MHz
	//	If the divisor is <9, it seems the chip cannot boot on its own, without reboot.
	//	It looks like when divisor goes less than 5, it somehow jumps to exactly 5.5.
	//
	//   0x0X: 190ISH MHz (Won't link)
	//   0x1X: 115.5ISH MHz
	//   0x2X: 189.0ISH MHz
	//   0x3X: 104.0ISH MHz?
	//   0x4X: 189.4ISH MHz?
	//   0x5X: 94.61 MHz?
	//   0x6X: 189.0ISH MHz?
	//   0x7X: 86.6ISH MHz?
	//   0x8X: 189.0ISH MHz?
	//   0x91: 80 MHz Stock
	//   0xA1: 173.3? MHz?  (Divisor 6)
	//   0xB1: 74.33 MHz?
	//   0xC1: 148.49 MHz?
	//   0xD1: 69.32 MHz?
	//   0xE1: 130 MHz?
	//	 0xF1: 65 MHz.
	//
	//Processor requires a hard reboot to link when peripheral bus is operating past 115 MHz
	//Not sure why.
	//
	//ESP8285, when operating at divisors less than 6 seems to go to close to 5, and will boot, but never reaches 5.

	//XXX TODO: Further tuning if we change the 3rd parameter to 4???
#if PERIPH_FREQ == 115
	pico_i2c_writereg(103,4,2,0x11);
#elif PERIPH_FREQ == 173
	pico_i2c_writereg(103,4,2,0xa1);
#elif PERIPH_FREQ == 189
	pico_i2c_writereg(103,4,2,0x81);
#else
	// set 80MHz/160MHz PLL CPU
	pico_i2c_writereg(103,4,2,0x91);
#endif
}

void nosdk8266_clock()
{
#if MAIN_MHZ == 52
	t_ets_update_cpu_frequency( 52 );
#elif MAIN_MHZ == 104
	DPORT_BASEADDR[0x14/4] |= 0x01; //Overclock bit.
	t_ets_update_cpu_frequency( 104 );
#elif MAIN_MHZ == 80 || MAIN_MHZ == 115 || MAIN_MHZ == 160 || MAIN_MHZ == 173 || MAIN_MHZ==189
	//rom_rfpll_reset();	//Reset PLL.
	set_pll();			//Set PLL
	//HWREG(DPORT_BASEADDR,0x14) &= 0x7E; //Regular clock bit.
 	t_ets_update_cpu_frequency(80);
#elif MAIN_MHZ == 231 || MAIN_MHZ == 346 || MAIN_MHZ==378 || MAIN_MHZ == 320 //Won't boot at 378 MHz.
	//rom_rfpll_reset();	//Reset PLL.
	set_pll();			//Set PLL
	DPORT_BASEADDR[0x14/4] |= 0x01; //Overclock bit.
	t_ets_update_cpu_frequency(160);
#else
	#error System MHz must be 52, 80, 120, 160 or 240
#endif

}

void nosdk8266_zerobss()
{
	uint32_t *addr = &_bss_start;
	for (addr = &_bss_start; addr < &_bss_end; addr++)  *addr = 0; //Safe, _bss_start doesn't have to == _bss_end
}

#ifndef PICO66

extern uint32_t _bss_start;
extern uint32_t _bss_end;

void nosdk8266_init()
{
	uint32_t *addr;
	nosdk8266_clock();

    for (addr = &_bss_start; addr < &_bss_end; addr++)
        *addr = 0;

	PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
	uart_div_modify(UART0, (PERIPH_FREQ*1000000)/115200);

	Cache_Read_Enable(0, 0, 1);
}


#endif
