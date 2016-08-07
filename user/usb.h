#ifndef _USB_H
#define _USB_H


#define USB_BUFFERSIZE 16

#define USB_OFFSET_BUFFER		0
#define USB_OFFSET_DSTATUS		(USB_BUFFERSIZE)
#define USB_OFFSET_PACKET_SIZE  (USB_OFFSET_DSTATUS+4)
#define USB_OFFSET_LAST_TOKEN   (USB_OFFSET_DSTATUS+8)
#define USB_OFFSET_DEBUG        (USB_OFFSET_DSTATUS+12)

#ifndef _INASM_
#include <c_types.h>

struct usb_internal_state_struct
{
	uint8_t usb_buffer[USB_BUFFERSIZE];

	//////////////NOOOOOPEEEE:::Bit 0: Expecting from host.  DATA0 = 0, DATA1 = 1
	//Bit 1: Next we sent to host. DATA0 = 0, DATA1 = 1
	uint32_t usb_buffer_status;
	uint32_t packet_size;
	uint32_t last_token;
	uint32_t debug;

	//Things past here are addressable by C.

	uint32_t my_address;
	uint32_t setup_request; //1 if needing setup packet.

	const uint8_t  * usb_bufferout;
	uint32_t   usb_bufferout_len;
	int last_sent_qty;
};

extern struct usb_internal_state_struct usb_internal_state __attribute__((aligned(4)));
extern void gpio_intr();

//poly_function = 0 to include CRC.
//poly_function = 2 to exclude CRC.
extern void usb_send_data( uint8_t * data, uint32_t length, uint32_t poly_function );

void handle_setup( uint32_t this_token, struct usb_internal_state_struct * ist );
void handle_sof( uint32_t this_token, struct usb_internal_state_struct * ist );
void handle_in( uint32_t this_token, struct usb_internal_state_struct * ist );
void handle_out( uint32_t this_token, struct usb_internal_state_struct * ist );

void handle_data( uint32_t this_token, struct usb_internal_state_struct * ist, uint32_t which_data );

void handle_ack( uint32_t this_token, struct usb_internal_state_struct * ist );

extern uint32_t usb_reinstate;

#else

.global usb_send_data;
.global usb_internal_state;
.global usb_reinstate;
.global handle_setup
.global handle_sof
.global handle_in
.global handle_out
.global handle_data
.global handle_ack
#endif





#define USB_LOW_SPEED
//#define USB_FULL_SPEED

#define DEBUGPIN 5

#define DPLUS 13
#define DMINUS 12
#define DMINUSBASE 12  //Must be D- first, then D+ second.

#define _BV(x) ((1)<<(x))

#if defined( _INASM_ ) || defined( NEED_CONSTANTS ) 

#define GPIO_BASE_REG       0x60000300
#define GPIO_OFFSET_OUT     0x00 
#define GPIO_OFFSET_SET     0x04
#define GPIO_OFFSET_CLEAR   0x08
#define GPIO_OFFSET_DIR     0x0c
#define GPIO_OFFSET_DIR_OUT 0x10
#define GPIO_OFFSET_DIR_IN  0x14
#define GPIO_OFFSET_INPUT   0x18

#define GPIO_OFFSET_GPIO_STATUS  0x1c
#define GPIO_OFFSET_GPIO_STATUS_W1TC 0x24


//These interrupts don't work well.
#define DISABLE_INTERRUPTS_LCL \
	rsil a0, 15; \
	s32i a0, a1, 60; \
	rsr a0, SAR; \
	s32i a0, a1, 64;

#define ENABLE_INTERRUPTS_LCL \
	l32i a0, a1, 64; \
	wsr a0, SAR; \
	isync; \
	l32i a0, a1, 60; \
	wsr a0, ps; \
	isync;

#endif


#if defined( _INASM_ )

.global usb_ramtable

#else


//Detailed analysis of some useful stuff and performance tweaking: http://naberius.de/2015/05/14/esp8266-gpio-output-performance/
//Reverse engineerd boot room can be helpful, too: http://cholla.mmto.org/esp8266/bootrom/boot.txt
//USB Protocol read from wikipedia: https://en.wikipedia.org/wiki/USB
// Neat stuff: http://www.usbmadesimple.co.uk/ums_3.htm
// Neat stuff: http://www.beyondlogic.org/usbnutshell/usb1.shtml

struct usb_urb
{
	uint8_t pktp;
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} __attribute__((packed));

#endif

#endif

