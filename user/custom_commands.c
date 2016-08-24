//Copyright 2015 <>< Charles Lohr, see LICENSE file.

#include <commonservices.h>
#include <esp82xxutil.h>
#include <usb.h>

extern uint8_t last_leds[512*3];
extern int last_led_count;
uint8_t my_ep1[4];
uint8_t my_ep2[8];

int keybt;
int keymod;
int keypress;

int ICACHE_FLASH_ATTR CustomCommand(char * buffer, int retsize, char *pusrdata, unsigned short len)
{
	char * buffend = buffer;
	switch( pusrdata[1] )
	{
	case 'C': case 'c': //Custom command test
	{
		buffend += ets_sprintf( buffend, "CC" );
		return buffend-buffer;
	}
	case 'K': case 'k': //Keyboard
	{
		paramcount = 0;
		keymod = ParamCaptureAndAdvanceInt( );
		keybt = ParamCaptureAndAdvanceInt( );
		if( paramcount != 2 ) break;
		keypress = 1;
		buffend += ets_sprintf( buffend, "CK" );
		return buffend-buffer;
	}
	case 'M': case 'm': //Mouse
	{
		//CM[Mouse Button]\t[x]\t[y];
		int b = ParamCaptureAndAdvanceInt( );
		int dx = ParamCaptureAndAdvanceInt( );
		int dy = ParamCaptureAndAdvanceInt( );
		if( paramcount != 3 ) break;

		struct usb_internal_state_struct * uis = &usb_internal_state;
		struct usb_endpoint * e1 = &uis->eps[1];
		struct usb_endpoint * e2 = &uis->eps[2];

		//First see if we've alredy sent this, if so, zero the ep data.
		if( e1->send == 0 )
		{
			my_ep1[1] = 0;
			my_ep1[2] = 0;
		}

		my_ep1[0] = b;
		my_ep1[1] += dx;
		my_ep1[2] += dy;

		e1->ptr_in = my_ep1;
		e1->place_in = 0;
		e1->size_in = sizeof( my_ep1 );
		e1->send = 1;

		buffend += ets_sprintf( buffend, "CM" );
		return buffend-buffer;
	}



	}
	return -1;
}
