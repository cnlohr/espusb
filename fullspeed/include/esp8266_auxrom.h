#ifndef _ESP_ROMAUX_H
#define _ESP_ROMAUX_H

#include <c_types.h>

typedef struct{
	uint32	deviceId;
	uint32	chip_size;    // chip size in byte
	uint32	block_size;
	uint32  sector_size;
	uint32  page_size;
	uint32  status_mask;
} SpiFlashChip;

extern SpiFlashChip * flashchip; //don't forget: flashchip->chip_size = 0x01000000;



void ets_delay_us( uint32_t us );

//////////////////////////////////////////////////////////////////////////////////////////////////////

//This is done a little differently than we normally do it.  That's so GCC can figure out how to
//re-use the PIN_BASE address.

extern volatile uint32_t * PIN_BASE;
extern volatile uint32_t * IO_BASE;

#define PIN_OUT       ( PIN_BASE[0] )
#define PIN_OUT_SET   ( PIN_BASE[1] )
#define PIN_OUT_CLEAR ( PIN_BASE[2] )
#define PIN_DIR       ( PIN_BASE[3] )
#define PIN_DIR_OUTPUT (PIN_BASE[4] )
#define PIN_DIR_INPUT ( PIN_BASE[5] )
#define PIN_IN        ( PIN_BASE[6] )
#define PIN_STATUS    ( PIN_BASE[7] )
#define PIN_STATUS_SET    ( PIN_BASE[8] )
#define PIN_STATUS_CLEAR    ( PIN_BASE[9] )
#define PIN_CONF      ( &PIN_BASE[10] )

#define _BV(x) ((1)<<(x))

/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PICONOPRINT
#define ets_putc( c  )
#define uart_div_modify( u, d )
#define printf(x...)
#define putc(c)		
#else
void ets_putc( char c );
void uart_div_modify( int uart, int divisor );
#define putc ets_putc
#define printf ets_uart_printf
#endif

//Part of the romlib
void romlib_init();

#endif

