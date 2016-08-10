#define NEED_CONSTANTS

#include "usb.h"
#include <string.h>
#include <mystuff.h>
#include <eagle_soc.h>
#include <gpio.h>

#ifdef USB_LOW_SPEED
#include "usb_table_1bit.h"
#else
#error NOT IMPLEMENTED
#include "usb_table_3bit.h"
#endif

struct usb_internal_state_struct usb_internal_state __attribute__((aligned(4)));

//TODO: register this in http://pid.codes/howto/ or somewhere.

#define ENDPOINT0_SIZE 8

//Taken from http://www.usbmadesimple.co.uk/ums_ms_desc_dev.htm
static const uint8_t device_descriptor[] = {
	18, //Length
	1,  //Type (Device)
	0x10, 0x01, //Spec
	0x0, //Device Class
	0x0, //Device Subclass
	0x0, //Device Protocol  (000 = use config descriptor)
	0x08, //Max packet size for EP0 (This has to be 8 because of the USB Low-Speed Standard)
	0xcd, 0xab, //ID Vendor
	0x66, 0x82, //ID Product
	0x01, 0x00, //ID Rev
	1, //Manufacturer string
	2, //Product string
	0, //Serial string
	1, //Max number of configurations
};

static const uint8_t config_descriptor[] = {  //Mostly stolen from a USB mouse I found.
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9, 					// bLength;
	2,					// bDescriptorType;
	0x3b, 0x00,			// wTotalLength  	

	//34, 0x00, //for just the one descriptor
	
	0x01,					// bNumInterfaces (Normally 1)
	0x01,					// bConfigurationValue
	0x00,					// iConfiguration
	0x80,					// bmAttributes (was 0xa0)
	0x64,					// bMaxPower (200mA)


	//Mouse
	9,					// bLength
	4,					// bDescriptorType
	0,			// bInterfaceNumber (unused, would normally be used for HID)
	0,					// bAlternateSetting
	1,					// bNumEndpoints
	0x03,					// bInterfaceClass (0x03 = HID)
	0x01,					// bInterfaceSubClass
	0x02,					// bInterfaceProtocol (Mouse)
	0,					// iInterface

	9,					// bLength
	0x21,					// bDescriptorType (HID)
	0x10,0x01,		//bcd 1.1
	0x00, //country code
	0x01, //Num descriptors
	0x22, //DescriptorType[0] (HID)
	0x48, 0x00, //Descriptor length XXX This looks wrong!!!

	7, //endpoint descriptor (For endpoint 1)
	0x05, //Endpoint Descriptor (Must be 5)
	0x81, //Endpoint Address
	0x03, //Attributes
	0x04,	0x00, //Size
	0x07, //Interval (Was 0x0a)


	//Keyboard  (It is unusual that this would be here)
	9,					// bLength
	4,					// bDescriptorType
	1,			// bInterfaceNumber  = 1 instead of 0 -- well make it second.
	0,					// bAlternateSetting
	1,					// bNumEndpoints
	0x03,					// bInterfaceClass (0x03 = HID)
	0x01,					// bInterfaceSubClass
	0x01,					// bInterfaceProtocol (??)
	0,					// iInterface

	9,					// bLength
	0x21,					// bDescriptorType (HID)
	0x10,0x01,		//bcd 1.1
	0x00, //country code
	0x01, //Num descriptors
	0x22, //DescriptorType[0] (HID)
	63, 0x00, //Descriptor length XXX This looks wrong!!!

	7, //endpoint descriptor (For endpoint 1)
	0x05, //Endpoint Descriptor (Must be 5)
	0x82, //Endpoint Address
	0x03, //Attributes
	0x08,	0x00, //Size (8 bytes)
	0x13, //Interval (Was 0x0a)
};




static const uint8_t mouse_hid_desc[] = {  //Mostly stolen from a Microsoft USB mouse I found.
	0x05, 0x01, 0x09, 0x02, 0xa1, 0x01, 0x09, 0x01, 0xa1, 0x00, 0x05, 0x09, 0x19, 0x01, 0x29, 0x03,
	0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x03, 0x81, 0x02, 0x75, 0x05, 0x95, 0x01, 0x81, 0x01,
	0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x09, 0x38, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08, 0x95, 0x03,
	0x81, 0x06, 0xc0, 0x05, 0xff, 0x09, 0x02, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x01, 0xb1,
	0x22, 0x75, 0x07, 0x95, 0x01, 0xb1, 0x01, 0xc0, 
};

//From http://codeandlife.com/2012/06/18/usb-hid-keyboard-with-v-usb/
static const uint8_t keyboard_hid_desc[63] = {   /* USB report descriptor */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)(224)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs) ; Modifier byte
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs) ; Reserved byte
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs) ; LED report
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs) ; LED report padding
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))(0)
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)(101)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION
};


#define STR_MANUFACTURER L"CNLohr"
#define STR_PRODUCT      L"ESPUSB"
#define STR_SERIAL       L"000"

struct usb_string_descriptor_struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wString[];
};
const static struct usb_string_descriptor_struct string0 = {
	4,
	3,
	{0x0409}
};
const static struct usb_string_descriptor_struct string1 = {
	sizeof(STR_MANUFACTURER),
	3,
	STR_MANUFACTURER
};
const static struct usb_string_descriptor_struct string2 = {
	sizeof(STR_PRODUCT),
	3,
	STR_PRODUCT
};
const static struct usb_string_descriptor_struct string3 = {
	sizeof(STR_SERIAL),
	3,
	STR_SERIAL
};


// This table defines which descriptor data is sent for each specific
// request from the host (in wValue and wIndex).
const static struct descriptor_list_struct {
	uint16_t	wValue;
	uint16_t	wIndex;
	const uint8_t	*addr;
	uint8_t		length;
} descriptor_list[] = {
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0200, 0x0000, config_descriptor, sizeof(config_descriptor)},
	{0x2200, 0x0000, mouse_hid_desc, sizeof(mouse_hid_desc)},
	{0x2200, 0x0001, keyboard_hid_desc, sizeof(keyboard_hid_desc)},
	{0x0300, 0x0000, (const uint8_t *)&string0, 4},
	{0x0301, 0x0409, (const uint8_t *)&string1, sizeof(STR_MANUFACTURER)},
	{0x0302, 0x0409, (const uint8_t *)&string2, sizeof(STR_PRODUCT)},	
	{0x0303, 0x0409, (const uint8_t *)&string3, sizeof(STR_SERIAL)}
};
#define DESCRIPTOR_LIST_ENTRIES ((sizeof(descriptor_list))/(sizeof(struct descriptor_list_struct)) )


//Received a setup for a specific endpoint.
void handle_setup( uint32_t this_token, struct usb_internal_state_struct * ist )
{
	uint8_t addr = (this_token>>8) & 0x7f;
	uint8_t endp = (this_token>>15) & 0xf;

	if( endp >= ENDPOINTS ) goto end;
	if( addr != 0 && addr != ist->my_address ) goto end;  //XXX TODO: Is my_address per endpoint?s

	struct usb_endpoint * e = ist->ce = &ist->eps[endp];
	e->toggle_out = 0;
	e->toggle_in = 1;
	e->ptr_in = 0;
	e->send = 0;
	ist->setup_request = 1;
end:
	__asm__ __volatile__( "movi a0, usb_reinstate" ); //After this token, we are immediately expecting another grouping.  This short-circuits the 'return'.
}

void handle_sof( uint32_t this_token, struct usb_internal_state_struct * ist )
{
}

void PerpetuatePacket( struct usb_internal_state_struct * ist )
{
	struct usb_endpoint * e = ist->ce;
	if( !e ) return;
	if( e->ptr_in )
	{
		int tosend = e->size_in - e->place_in;

		if( tosend > 8 ) tosend = 8;

		uint8_t sendnow[12];
		sendnow[0] = 0x80;

		if( e->toggle_in )
		{
			sendnow[1] = 0b01001011; //DATA1
		}
		else
		{
			sendnow[1] = 0b11000011; //DATA0
		}

		if( e->ptr_in == EMPTY_SEND_BUFFER )  //Tricky: Empty packet.
		{
			usb_send_data( sendnow, 2, 3 );  //Force a CRC
			e->ptr_in = 0;
		}
		else
		{
			if( tosend )
				ets_memcpy( sendnow+2, e->ptr_in + e->place_in, tosend );
			usb_send_data( sendnow, tosend+2, 0 );
			e->advance_in = tosend;
		}
	}
}

void handle_in( uint32_t this_token, struct usb_internal_state_struct * ist )
{
	uint8_t addr = (this_token>>8) & 0x7f;
	uint8_t endp = (this_token>>15) & 0xf;

	//If we get an "in" token, we have to strike any accept buffers.

	if( endp >= ENDPOINTS ) return;
	if( addr != 0 && addr != ist->my_address ) return; //XXX TODO: is my_address per-endpoint?

	struct usb_endpoint * e = ist->ce = &ist->eps[endp];

	e->got_size_out = 0;  //Cancel any out transaction


	//XXX TODO: The following is wrong, for some reason e->size_in == e->place_in even if data is in there, I think!!!
	// /* && ( e->size_in - e->place_in > 0 )*/ ) // || ( endp == 0 && ist->setup_request ) )  //XXX I think this is wrong.  e->send?
	if( e->send && e->ptr_in ) 
	{
		PerpetuatePacket( ist );
		return;
	}
	else
	{
		volatile int i;
		for( i = 0; i < 10; i++ );
		uint8_t sendword[2] = { 0x80, 0x5a };  //Empty data. "NAK"
		usb_send_data( sendword, 2, 2 );
		return;
	}

}

void handle_out( uint32_t this_token, struct usb_internal_state_struct * ist )
{
	//I ... uuhh... I don't think we need to do anything here, unless we're accepting interrupt_in data?

	__asm__ __volatile__( "movi a0, usb_reinstate" );  //After this token, we are immediately expecting another grouping.  This short-circuits the 'return'.
}

void handle_data( uint32_t this_token, struct usb_internal_state_struct * ist, uint32_t which_data )
{
	//Received data from host.

	struct usb_endpoint * e = ist->ce;

	if( e == 0 ) return;

	if( e->toggle_out != which_data )
	{
		goto just_ack;
	}

	e->toggle_out = !e->toggle_out;

	if( ist->setup_request )
	{
		ist->setup_request = 0;
		struct usb_urb * s = (struct usb_urb *)ist->usb_buffer;

		//Send just a data packet.
		e->ptr_in = EMPTY_SEND_BUFFER;
		e->place_in = 0;
		e->size_in = 0;
		e->send = 1;

		if( s->bmRequestType & 0x80 )
		{
			if( s->bRequest == 0x06 ) //Get Request
			{
				int i;
				const struct descriptor_list_struct * dl;
				for( i = 0; i < DESCRIPTOR_LIST_ENTRIES; i++ )
				{
					dl = &descriptor_list[i];
					if( dl->wIndex == s->wIndex && dl->wValue == s->wValue )
						break;
				}

				if( i == DESCRIPTOR_LIST_ENTRIES )
				{
					//??? Somehow fail?  Is 'nak' the right thing? I don't know what to do here.
					goto just_ack;
				}

				e->ptr_in = dl->addr;
				e->size_in = dl->length;
				if( s->wLength < e->size_in ) e->size_in = s->wLength;
			}

			/////////////////////////////EXAMPLE CUSTOM CONTROL MESSAGE/////////////////////////////
			if(s->bRequest == 0xa7 ) //US TO HOST "in"
			{
				if( ist->user_control_length_ret )
				{					
					e->ptr_in = ist->user_control;
					e->size_in = ist->user_control_length_ret;
					if( s->wLength < e->size_in ) e->size_in = s->wLength;
					ist->user_control_length_ret = 0;
				}
			}
		}
		else if( s->bmRequestType == 0x00 )
		{
			if( s->bRequest == 0x05 ) //Set address.
			{
				ist->my_address = s->wValue;
			}
			if( s->bRequest == 0x09 ) //Set configuration.
			{
				//s->wValue; has the index.  We don't really care about this.
			}

			/////////////////////////////EXAMPLE CUSTOM CONTROL MESSAGE/////////////////////////////
			if( s->bRequest == 0xa6 && ist->user_control_length_acc == 0 ) //HOST TO US "out"
			{
				e->ptr_out = ist->user_control;
				e->max_size_out = sizeof( ist->user_control );
				if( e->max_size_out > s->wLength ) e->max_size_out = s->wLength;
				e->got_size_out = 0;
			}
		}
	}
	else if( e->ptr_out )
	{
		//Read into that buffer.
		int acc = ist->packet_size-3;  //packet_size includes CRC and PID, need just data size.
		int place = e->got_size_out;
		if( place + acc > e->max_size_out )
		{
			acc = e->max_size_out - place;
		}

		ets_memcpy( e->ptr_out + e->got_size_out, ist->usb_buffer+1, acc );  //First byte of USB buffer is token.
		e->got_size_out += acc;
	}


just_ack:
	{
		//Got the right data.  Acknowledge.
		uint8_t sendword[2] = { 0x80, 0xD2 };
		usb_send_data( sendword, 2, 2 );
	}
}

void handle_ack( uint32_t this_token, struct usb_internal_state_struct * ist )
{
	struct usb_endpoint * e = ist->ce;
	if( !e ) return;


	e->toggle_in = !e->toggle_in;
	e->place_in += e->advance_in;
	if( e->place_in == e->size_in )
	{
		e->send = 0;
	}
	
	ist->last_sent_qty = 0;
}


void  ICACHE_FLASH_ATTR init_usb()
{
    ETS_GPIO_INTR_DISABLE();                                           //Close the GPIO interrupt

	PIN_FUNC_SELECT(PERIPHSDPLUS,FUNCDPLUS); //D- (needs pullup)
	PIN_FUNC_SELECT(PERIPHSDMINUS,FUNCDMINUS); //D+

    PIN_DIR_INPUT = _BV(DMINUS)|_BV(DPLUS);
	PIN_PULLUP_DIS( PERIPHSDMINUS );
	PIN_PULLUP_EN( PERIPHSDPLUS );

    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(0));
    ETS_GPIO_INTR_ATTACH(gpio_intr,NULL);                                //Attach the gpio interrupt.
    gpio_pin_intr_state_set(GPIO_ID_PIN(DMINUS),GPIO_PIN_INTR_POSEDGE);  //Rising Edge Trigger.

	//Forcibly disconnect from bus.
	volatile uint32_t * gp = (volatile uint32_t*)GPIO_BASE_REG;
	gp[GPIO_OFFSET_CLEAR/4] = _BV(DPLUS) | _BV(DMINUS);
	gp[GPIO_OFFSET_DIR_OUT/4] = _BV(DPLUS) | _BV(DMINUS);
	ets_delay_us( 10000 );
	gp[GPIO_OFFSET_DIR_IN/4] = _BV(DPLUS) | _BV(DMINUS);

    ETS_GPIO_INTR_ENABLE();

}
