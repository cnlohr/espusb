#ifndef _USB_H
#define _USB_H

#define USB_LOW_SPEED
//#define USB_FULL_SPEED  (Not implemented)
#define DEBUGPIN 2

#define DPLUS 5
#define DMINUS 4
#define DMINUSBASE 4  //Must be D- first, then D+ second.

#define ENDPOINTS 3

#define PERIPHSDPLUS    PERIPHS_IO_MUX_GPIO5_U
#define PERIPHSDMINUS   PERIPHS_IO_MUX_GPIO4_U
#define FUNCDPLUS  FUNC_GPIO5
#define FUNCDMINUS FUNC_GPIO4


#define USB_BUFFERSIZE 16  //Must be big enough to hold PID + DATA + EOF (plus a bit just in case)

#define USB_OFFSET_BUFFER		0
#define USB_OFFSET_DSTATUS		(USB_BUFFERSIZE)
#define USB_OFFSET_PACKET_SIZE  (USB_OFFSET_DSTATUS)
#define USB_OFFSET_LAST_TOKEN   (USB_OFFSET_DSTATUS+4)
#define USB_OFFSET_DEBUG        (USB_OFFSET_DSTATUS+8)

#ifndef _INASM_
#include <c_types.h>

#define EMPTY_SEND_BUFFER (uint8_t*)1

struct usb_endpoint
{
	const uint8_t * ptr_in;		// Pointer to "IN" data (US->PC)
	uint8_t size_in;		// Total size of the structure pointed to by ptr_in
	uint8_t advance_in;		// How much data was sent this packet? (How much to advance in ack)
	uint8_t place_in;		// Where in the ptr_in we are currently pointing.
	uint8_t toggle_in; 		// DATA0 or DATA1?
	uint8_t send;			// Sets back to 0 when done sending.

	uint8_t * ptr_out;
	int * transfer_done_ptr;  //Written to # of bytes received when a datagram is done.
	uint8_t max_size_out;
	uint8_t toggle_out;  //Out PC->US
	uint8_t got_size_out;
};

struct usb_internal_state_struct
{
	//This data is modified by the super low-level code.

	uint8_t usb_buffer[USB_BUFFERSIZE];

	uint32_t packet_size; //Of data currently in usb_buffer
	uint32_t last_token;
	uint32_t debug;


	struct usb_endpoint * ce;  //Current endpoint
	struct usb_endpoint eps[ENDPOINTS];

	//Things past here are addressable by C.

	uint32_t my_address;
	uint32_t setup_request; //1 if needing setup packet.

	//Current transmission from us to host.
//	const uint8_t  * usb_bufferret;
//	uint32_t   usb_bufferret_len;

	int last_sent_qty;

	//TODO: make an endpoint struct for handling this...
/*	int   *  eptoggle;
	int   *  marksentptr;
	uint8_t ep1data[4];
	int     sendep1;
	int     ep1toggle;
	uint8_t ep3data[8];
	int     sendep3;
	int     ep3toggle;
*/

	//Awkward example with use of control messages to get data to/from device.
	//uint8_t * usb_bufferaccept;
	//uint32_t  accept_length;

	uint8_t user_control[150];
	int     user_control_length_acc; //From host to us.
	int     user_control_length_ret; //From us to host.
};

extern struct usb_internal_state_struct usb_internal_state __attribute__((aligned(4)));

void ICACHE_FLASH_ATTR init_usb();

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

