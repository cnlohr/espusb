#include <esp8266_auxrom.h>
#include <uart_dev.h>
#include <eagle_soc.h>
#include <ets_sys.h>

#define UART0 0

extern UartDevice UartDev;

#define	sys_const_crystal_26m_en	48	// soc_param0: 0: 40MHz, 1: 26MHz, 2: 24MHz

//Thanks, https://github.com/pvvx/esp8266web/blob/2e25559bc489487747205db2ef171d48326b32d4/app/sdklib/system/app_main.c
void set_pll(void)
{
	if(rom_i2c_readReg(103,4,1) != 136) { // 8: 40MHz, 136: 26MHz
		//if(get_sys_const(sys_const_crystal_26m_en) == 1) { // soc_param0: 0: 40MHz, 1: 26MHz, 2: 24MHz
			// set 80MHz PLL CPU
			rom_i2c_writeReg(103,4,1,136);
			rom_i2c_writeReg(103,4,2,145);
		//}
	}
}

extern uint32_t _bss_start;
extern uint32_t _bss_end;

void romlib_init()
{
	uint32_t *addr;

	ets_update_cpu_frequency( MAIN_MHZ );
	rom_rfpll_reset();	//Reset PLL.
	set_pll();			//Set PLL to 80 MHz.

    for (addr = &_bss_start; addr < &_bss_end; addr++)
        *addr = 0;

	PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
	uart_div_modify(UART0, (MAIN_MHZ*1000000)/115200);

	//Is this needed?
	gpio_init();
}

