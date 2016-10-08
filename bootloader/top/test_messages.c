#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <string.h>

int main()
{
	struct libusb_device_handle *devh = NULL;

	if( libusb_init(NULL) < 0 )
	{
		fprintf( stderr, "Error: Could not initialize libUSB\n" );
		return -1;
	}


	devh = libusb_open_device_with_vid_pid( NULL, 0xabcd, 0x8266 );
	if( !devh )
	{
		fprintf( stderr,  "Error: Cannot find USB device.\n" );
		return -1;
	}

	int i, r1, r2;

	while(1)
	{
		char buffer[2100];

		strcpy( buffer, "U" );
		int len = 2090;

		r1 = libusb_control_transfer( devh,
			0x00,    //reqtype  (0x80 = Device->PC, 0x00 = PC->Device)
			0xA6,    //request
			0x0100,  //wValue
			0x0000,  //wIndex
			buffer,
			len,     //wLength  (more like max length)
			100 );

		r2 = libusb_control_transfer( devh,
			0x80,    //reqtype  (0x80 = in, 0x00 = out)
			0xA7,    //request
			0x0100,  //wValue
			0x0000,  //wIndex
			buffer, //wLength  (more like max length)
			sizeof( buffer ),
			100 );

		buffer[sizeof(buffer)-1] = 0;
		printf( "." );
		fflush( stdout );
		fprintf( stderr, "%d/%d/%s\n", r1, r2, buffer );
		if( r1 < 0 || r2 < 0 ) return -1;
	}

	return 0;
}

