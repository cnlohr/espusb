#ifndef _ESP_ROMAUX_H
#define _ESP_ROMAUX_H

#include <c_types.h>

void ets_delay_us( uint32_t us );

//////////////////////////////////////////////////////////////////////////////////////////////////////

#define PIN_OUT       ( *((uint32_t*)0x60000300) )
#define PIN_OUT_SET   ( *((uint32_t*)0x60000304) )
#define PIN_OUT_CLEAR ( *((uint32_t*)0x60000308) )
#define PIN_DIR       ( *((uint32_t*)0x6000030C) )
#define PIN_DIR_OUTPUT ( *((uint32_t*)0x60000310) )
#define PIN_DIR_INPUT ( *((uint32_t*)0x60000314) )
#define PIN_IN        ( *((volatile uint32_t*)0x60000318) )
#define _BV(x) ((1)<<(x))

/////////////////////////////////////////////////////////////////////////////////////////////////////

void gpio_init(void);
void ets_putc( char c );
void uart_div_modify( int uart, int divisor );
#define putc ets_putc
#define printf ets_uart_printf

//Part of the romlib
void romlib_init();

#endif

