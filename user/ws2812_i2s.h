//Copyright 2015 <>< Charles Lohr, See LICENSE file.
//WS2812 sender that abuses the I2S interface on the WS2812.

#ifndef _WS2812I2S_TEST
#define _WS2812I2S_TEST

//Stuff that should be for the header:

#include <c_types.h>

//Parameters for the I2S DMA behaviour
//#define I2SDMABUFCNT (2)			//Number of buffers in the I2S circular buffer
//#define I2SDMABUFLEN (32*2)		//Length of one buffer, in 32-bit words.

//NOTE: Blocksize MUST be divisible by 4.  Cannot exceed 4092
//Each LED takes up 12 block bytes in WS2812_FOUR_SAMPLE
//Or 9 block bytes in WS2812_THREE_SAMPLE
#define WS_BLOCKSIZE 4000

//You can either have 3 or 4 samples per bit for WS2812s.
//3 sample can't go quite as fast as 4.
//3 sample uses more processing when updating than 4.
//4 takes up more RAM per LED than 3.
//3 has slightly more restrictve timing requirements.
//4 has more DMA load when running.
#define WS2812_THREE_SAMPLE
//#define WS2812_FOUR_SAMPLE

// timing for SK6812 LEDs, always uses 4bit samples
//#define SK6812

void ICACHE_FLASH_ATTR ws2812_init();
void ws2812_push( uint8_t * buffer, uint16_t buffersize ); //Buffersize = Nr LEDs * 3

#endif

