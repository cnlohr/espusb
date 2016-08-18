#define NEED_CONSTANTS

#include "usb.h"

#define TABLE_OFFSET 36

#define GPIO_BASE_OFFSET 0
#define LOOP_COUNT_OFFSET 8
#define ANDING_MASK_OFFSET 12
#define RUNNING_TIMEOUT_OFFSET  16
#define USB_INTERNAL_STATE_OFFSET  20
#define CRC16_INIT_OFFSET 24
#define CRC16_POLY_OFFSET 28
#define CRC16_CHECK_OFFSET 32
#define CRC5_INITIAL 0x1f
#define CRC5_POLY    0x14
#define CRC5_CHECK   0x06
#ifndef _INASM_

uint32_t usb_ramtable[31] __attribute__((aligned(16))) = {
	GPIO_BASE_REG, // Base reg for all GPIO (Offset 0)
	0, // Reserved (For debug)              (Offset 1)
	100, // Loop Count                      (Offset 2)
	0x3c, // Anding mask for table          (Offset 3)
	136, //Timeout (in bits)                (Offset 4)
	(uint32_t)&usb_internal_state,  //State (Offset 5)
	0xffff, //Initial value for CRC         (Offset 6)
	0xA001, //CRC Poly                      (Offset 7)
	0xB001, //CRC Check                     (Offset 8)

	0x80060b81, 0x800f0281, 0x80061381, 0x80170281, 
	0x80061b81, 0x801f0281, 0x80062381, 0x80270281, 
	0x80062b81, 0x802f0281, 0x80063381, 0x80370281, 
	0x80048081, 0x80840081, 0x803cb880, 0x80043880, };
#endif
