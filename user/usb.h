#ifndef _USB_H
#define _USB_H

#include "usb_config.h"

#define USB_LOW_SPEED

#define USB_BUFFERSIZE 12  //Must be big enough to hold PID + DATA + CRC + EOF

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


	struct usb_endpoint * ce;  //Current endpoint (set by IN/OUT PIDs)
	struct usb_endpoint eps[ENDPOINTS];

	//Things past here are addressable by C.

	uint32_t my_address;	//For the current address set up by the setup portion of USB.
	uint32_t setup_request; //1 if needing setup packet.
};

extern struct usb_internal_state_struct usb_internal_state __attribute__((aligned(4)));

//Functions that must be supplied by user.
void usb_handle_custom_control( int bmRequestType, int bRequest, int wLength, struct usb_internal_state_struct * ist );

//Functions within this suite
void ICACHE_FLASH_ATTR usb_init();

//This function is provided in assembly.
extern void gpio_intr();

//poly_function = 0 to include CRC.
//poly_function = 2 to exclude CRC.
//This function is provided in assembly
extern void usb_send_data( uint8_t * data, uint32_t length, uint32_t poly_function );

void usb_pid_handle_setup( uint32_t this_token, struct usb_internal_state_struct * ist );
void usb_pid_handle_sof( uint32_t this_token, struct usb_internal_state_struct * ist );
void usb_pid_handle_in( uint32_t this_token, struct usb_internal_state_struct * ist );
void usb_pid_handle_out( uint32_t this_token, struct usb_internal_state_struct * ist );
void usb_pid_handle_data( uint32_t this_token, struct usb_internal_state_struct * ist, uint32_t which_data );
void usb_pid_handle_ack( uint32_t this_token, struct usb_internal_state_struct * ist );

extern uint32_t usb_reinstate;

#else

.global usb_send_data;
.global usb_internal_state;
.global usb_reinstate;
.global usb_pid_handle_setup
.global usb_pid_handle_sof
.global usb_pid_handle_in
.global usb_pid_handle_out
.global usb_pid_handle_data
.global usb_pid_handle_ack
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

