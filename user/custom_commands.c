//Copyright 2015 <>< Charles Lohr, see LICENSE file.

#include <commonservices.h>
#include <mystuff.h>
#include <usb.h>

extern uint8_t last_leds[512*3];
extern int last_led_count;
uint8_t my_ep1[4];
uint8_t my_ep2[8];


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

	case 'M': case 'm': //Custom command test
	{
		//CM[Mouse Button]\t[x]\t[y];
		char * btn = len>2 ? &pusrdata[2] : 0;
		char * mx = btn ? (char *)ets_strstr( (char*)(btn+1), "\t" ) : 0;
		char * my = mx ? (char *)ets_strstr( (char*)(mx+1), "\t" ) : 0;
		if( mx ) { *mx = 0; mx++; }
		if( my ) { *my = 0; my++; } //Properly null terminate, and advance beyond the \t.

		int b = my_atoi( btn );
		int dx = my_atoi( mx );
		int dy = my_atoi( my );

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


		printf( "%d: %d :%d\n", b, dx, dy );
		buffend += ets_sprintf( buffend, "CM" );
		return buffend-buffer;
	}



	}
	return -1;
}
