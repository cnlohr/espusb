#include "c_types.h"
#include "esp8266_auxrom.h"
#include "esp8266_rom.h"
#include "eagle_soc.h"
#include "ets_sys.h"
#include "gpio.h"
#include "nosdk8266.h"
#include "nosdki2s.h"

//This is a little more bloated than PICO66.  It has useful stuff like PRINTF and can call anything in
//romlib.c 

#define call_delay_us( time ) { asm volatile( "mov.n a2, %0\n_call0 delay4clk" : : "r"(time*13) : "a2" ); }

typedef enum {
    INUM_WDEV_FIQ = 0,
    INUM_SLC = 1,
    INUM_SPI = 2,
    INUM_RTC = 3,
    INUM_GPIO = 4,
    INUM_UART = 5,
    INUM_TICK = 6, /* RTOS timer tick, possibly xtensa CPU CCOMPARE0(?) */
    INUM_SOFT = 7,
    INUM_WDT = 8,
    INUM_TIMER_FRC1 = 9,

    /* FRC2 default handler. Configured by sdk_ets_timer_init, which
       runs as part of default libmain.a startup code, assigns
       interrupt handler to sdk_vApplicationTickHook+0x68
     */
    INUM_TIMER_FRC2 = 10,
} xt_isr_num_t;




int main()
{
	int i = 0;
	nosdk8266_init();

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U,FUNC_GPIO4);

	PIN_DIR_OUTPUT = _BV(2); //Enable GPIO2 light off.

	PIN_DIR_OUTPUT = _BV(4)|_BV(5);
	PIN_OUT_CLEAR = _BV(4)|_BV(5);
	call_delay_us(1000);
	PIN_OUT_SET = _BV(4)|_BV(5);
	PIN_DIR_INPUT = _BV(4)|_BV(5);

typedef enum {
    GPIO_INTTYPE_NONE       = 0,
    GPIO_INTTYPE_EDGE_POS   = 1,
    GPIO_INTTYPE_EDGE_NEG   = 2,
    GPIO_INTTYPE_EDGE_ANY   = 3,
    GPIO_INTTYPE_LEVEL_LOW  = 4,
    GPIO_INTTYPE_LEVEL_HIGH = 5,
} gpio_inttype_t;

for( i = 0; i < 10; i++ )
	{
//UserExceptionHandler();
//		printf( "Ok.\n" );
//		myfunc();
		PIN_OUT_SET = _BV(2); //Turn GPIO2 light on.
//		myfunc();
		PIN_OUT_CLEAR = _BV(2); //Turn GPIO2 light off.
	}

	#define GPIO_CONF_INTTYPE_S 7
	PIN_CONF[4] = GPIO_INTTYPE_EDGE_ANY<<GPIO_CONF_INTTYPE_S;
	PIN_CONF[5] = GPIO_INTTYPE_EDGE_ANY<<GPIO_CONF_INTTYPE_S;

	//Next, need to enable interrupts.


    asm volatile ("movi    a2, VecBase\nwsr     a2, vecbase" :  : : "a2" );
        

	uint32_t intenable;
    asm volatile ("rsr %0, intenable" : "=a" (intenable));
    intenable |= 1<<INUM_GPIO;
    asm volatile ("wsr %0, intenable; esync" :: "a" (intenable));

	printf( "Starting.\n" );

	while(1)
	{
//UserExceptionHandler();
//		printf( "Ok.\n" );
//		myfunc();
//		PIN_OUT_SET = _BV(2); //Turn GPIO2 light on.
//		myfunc();
//		PIN_OUT_CLEAR = _BV(2); //Turn GPIO2 light off.
		i++;
	}
}

