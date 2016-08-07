#define NEED_CONSTANTS

#include "usb.h"

#define TABLE_OFFSET 32

#define PIN_OUT_SET_OFFSET 4
#define GPIO_STATUS_W1TC_OFFSET 16
#define GPIO_STATUS_OFFSET 20

#ifndef _INASM_

uint32_t usb_ramtable[72] __attribute__((aligned(16))) = {
	PIN_IN, // Reading Pins        (Offset 0)
	PIN_OUT_SET, // Debug Output   (Offset 1)
	PIN_OUT_CLEAR, // Debug Output (Offset 2)
	0x000000f0, // Anding mask     (Offset 3)
	GPIO_STATUS_W1TC, // For int   (Offset 4)
	GPIO_STATUS, // For int        (Offset 5)
	0, 0, 
	0x0c0c0c0c, 0xfe083a0e, 0xff4b190f, 0xfcfcfcfc, 
	0x0c0c0c0c, 0xff095b0f, 0xfe2a180e, 0xfcfcfcfc, 
	0x0c0c0c0c, 0xfe083a0e, 0xff6b190f, 0xfcfcfcfc, 
	0x0c0c0c0c, 0xff097b0f, 0xfe2a180e, 0xfcfcfcfc, 
	0x0c0c0c0c, 0xfe083a0e, 0xff8b190f, 0xfcfcfcfc, 
	0x0c0c0c0c, 0xff099b0f, 0xfe2a180e, 0xfcfcfcfc, 
	0x0c0c0c0c, 0xfe083a0e, 0xffab190f, 0xfcfcfcfc, 
	0x0c0c0c0c, 0xff09bb0f, 0xfe2a180e, 0xfcfcfcfc, 
	0x0c0c0c0c, 0xfe083a0e, 0xffcb190f, 0xfcfcfcfc, 
	0x0c0c0c0c, 0xff09db0f, 0xfe2a180e, 0xfcfcfcfc, 
	0x0c0c0c0c, 0xfe083a0e, 0xffef150f, 0xfcfcfcfc, 
	0x0c0c0c0c, 0xff05ff0f, 0xfe2a180e, 0xfcfcfcfc, 
	0x0c0c0c0c, 0xfc04350c, 0xecececec, 0xfcfcfcfc, 
	0x0c0c0c0c, 0xfcfcfcfc, 0xfc25140c, 0xfcfcfcfc, 
	0xecececec, 0xfce0fcec, 0xfc25140c, 0xfcfcfcfc, 
	0xecececec, 0xfcfcfcfc, 0xfc00f0ec, 0xfcfcfcfc, };
#endif
