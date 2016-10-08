#include "c_types.h"
#include "esp8266_auxrom.h"
#include "esp8266_rom.h"
#include "eagle_soc.h"
#include "ets_sys.h"
#include "gpio.h"
#include <usb.h>

//Awkward example with use of control messages to get data to/from device.
uint8_t user_control[1100]; //Enough for FW######## ### [128 bytes of data] [null]
int     user_control_length_acc = 0; //From host to us.
int     user_control_length_ret = 0; //From us to host.
int hit = 0;

void usb_handle_custom_control( int bmRequestType, int bRequest, int wLength, struct usb_internal_state_struct * ist )
{
	struct usb_endpoint * e = ist->ce;

	if( bmRequestType == 0x80 )
	{
		if( bRequest == 0xa7) //US TO HOST "in"
		{
			if( user_control_length_ret )
			{
				e->ptr_in = user_control;
				e->size_in = user_control_length_ret;
				if( wLength < e->size_in ) e->size_in = wLength;
				hit = e->size_in;
				user_control_length_ret = 0;
			}
		}
	}

	if( bmRequestType == 0x00 )
	{
		if( bRequest == 0xa6 && user_control_length_acc == 0 ) //HOST TO US "out"
		{
			e->ptr_out = user_control;
			e->max_size_out = sizeof( user_control );
			if( e->max_size_out > wLength ) e->max_size_out = wLength;
			e->got_size_out = 0;
			e->transfer_done_ptr = &user_control_length_acc;
		}

	}

}



int main()
{
	int i = 0;
	user_control_length_acc = 0; //From host to us.
	user_control_length_ret = 0; //From us to host.

	romlib_init();

	//PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);
	//PIN_DIR_OUTPUT = _BV(2); //Enable GPIO2 light off.

	printf( "Initializing USB\n" );
	usb_init();

	while(1)
	{
		//Data comes in on user_control, and updates user_control_length_acc
		//You can return data by putting it in user_control and setting user_control_length_ret
		//To determine if you are connected, check usb_internal_state.there_is_a_host

		user_control[1] = 0;
		printf( "/%d(%d) MSG:%s There_Is_Host: %d\n", user_control_length_acc, hit, user_control, usb_internal_state.there_is_a_host );
		user_control_length_acc = 0; //Clear out the incoming data since we just got it.

		//Write some stuff into the outgoing data.
		user_control[0] = 'K';
		user_control_length_ret = 1024;

		PIN_OUT_SET = _BV(2); //Turn GPIO2 light off.
		ets_delay_us( 10000 );
		PIN_OUT_CLEAR = _BV(2); //Turn GPIO2 light off.
		ets_delay_us( 10000 );

		i++;
	}
}

