#include "c_types.h"
#include "esp8266_auxrom.h"
#include "esp8266_rom.h"
#include "eagle_soc.h"
#include "ets_sys.h"
#include "gpio.h"
#include <usb.h>

#define CUSTOM_BUFFERSIZE 2100

uint8_t usb_custom_acc[CUSTOM_BUFFERSIZE];
uint8_t usb_custom_ret[CUSTOM_BUFFERSIZE];


//This might look a little odd, but, keep in mind that the xtensa architecture can
//access class-like variables much faster than it can access globals.
struct USBControlStruct
{
	uint8_t *	acc;
	short		acc_index;
	short		acc_value;
	uint8_t		acc_request;
	int			length_acc;

	uint8_t *	ret;
	int			length_ret;
	int			ret_done;
} cctrl;


//This function is called-back from within the USB stack whenever a control message is begun.
//This code must execute VERY quickly.  So, no copying things or doddling around.  All of that must
//be done in the main thread.
void usb_handle_custom_control( int bmRequestType, int bRequest, int wLength, struct usb_internal_state_struct * ist )
{
	struct usb_urb * s = (struct usb_urb *)ist->usb_buffer;
	struct usb_endpoint * e = ist->ce;
	struct USBControlStruct * cc = &cctrl;

	if( bmRequestType == 0x80 )
	{
		if( bRequest == 0xa0) //US TO HOST "in"
		{
			if( cc->length_ret )
			{
				e->ptr_in = cc->ret;
				e->size_in = cc->length_ret;
				e->transfer_in_done_ptr == &cc->ret_done;
				if( wLength < e->size_in ) e->size_in = wLength;
				cc->length_ret = 0;
			}
		}
	}

	if( bmRequestType == 0x00 )
	{
		if( bRequest >= 0xa0 && bRequest < 0xc0 && cc->length_acc == 0 ) //HOST TO US "out" Only permit if we've already cleared out the message.
		{
			cc->acc_request = bRequest;
			cc->acc_value = s->wValue;
			cc->acc_index = s->wIndex;
			e->ptr_out = cc->acc;
			e->max_size_out = CUSTOM_BUFFERSIZE;
			if( e->max_size_out > wLength ) e->max_size_out = wLength;
			e->got_size_out = 0;
			e->transfer_done_ptr = &cc->length_acc;
		}
		else
		{
			e->ptr_out = 0;
			e->max_size_out = 0;
		}

	}

}



int main()
{
	int i = 0;
	struct USBControlStruct * cc = &cctrl;
	romlib_init();

	cc->acc = usb_custom_acc;
	cc->ret = usb_custom_ret;
	cc->ret_done = 1;

	//PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);
	//PIN_DIR_OUTPUT = _BV(2); //Enable GPIO2 light off.

	printf( "Initializing USB\n" );
	usb_init();

	while(1)
	{
		//Data comes in on user_control, and updates user_control_length_acc
		//You can return data by putting it in user_control and setting user_control_length_ret
		//To determine if you are connected, check usb_internal_state.there_is_a_host


		if( cc->length_acc && cc->ret_done ) //XXX Todo: Make sure our response was received.
		{
			switch( cc->acc_request )
			{
			case 0xa0: //Echo
				ets_memcpy( cc->ret, cc->acc, cc->length_acc );
				cc->ret = usb_custom_ret;
				cc->length_ret = cc->length_acc;
				cc->length_acc = 0; //Clear out the incoming data once done processing.
				printf( "/%d MSG:%s %d %02x:%04x:%04x\n", cc->length_acc, cc->acc, usb_internal_state.there_is_a_host,cc->acc_request,cc->acc_value,cc->acc_index );
				break;
			case 0xa1: //Read RAM.
				cc->ret = (uint8_t*)((cc->acc_value<<16) | cc->acc_index);
				cc->length_ret = cc->length_acc;
				cc->length_acc = 0;
				break;
			case 0xa2: //Read FLASH
			{
				int toread = (cc->acc_value>>8)*16;
				int addy = (((cc->acc_value & 0x0f)<<16) | cc->acc_index)*4;
				cc->ret = usb_custom_ret;
				SPIRead( addy, (uint32_t*)cc->ret, toread );
				cc->length_ret = toread;
				cc->length_acc = 0;
				break;
			}
			case 0xa3: //Write FLASH 
			{
				int addy = (((cc->acc_value & 0x0f)<<16) | cc->acc_index)*4;
				cc->ret = usb_custom_ret;
				SPIWrite( addy, (uint32_t*)cc->acc, cc->length_acc & 0xfffc );
				cc->length_ret = 1;
				cc->length_acc = 0;
				break;
			}
			case 0xa4: //Erase FLASH Block
			{
				int addy = (((cc->acc_value & 0x0f)<<16) | cc->acc_index)*4;
				cc->ret = usb_custom_ret;
				SPIEraseBlock( cc->acc_value );
				cc->length_ret = 1;
				cc->length_acc = 0;
				break;
			}



			}


		}

		//Don't know why.  Without this, it breaks.
		ets_delay_us( 1 );
		i++;

		//If there is no host connected, and it's been 1/2 second, bail.
		if( i == 400000 && !usb_internal_state.there_is_a_host )
		{
			printf( "No host (Should do normal boot)\n" );
		}
	}
}

