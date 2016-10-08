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
		uint8_t buffer[2100];

		memset( buffer, 0, sizeof( buffer ) );
		strcpy( buffer, "U" );
		buffer[3] = 0xff;
		int len = 2090;

		int show_output = 1;
#if 1
		r1 = libusb_control_transfer( devh,
			0x00,    //reqtype  (0x80 = Device->PC, 0x00 = PC->Device)
			0xA0,    //request (Echo)
			0x1234,  //wValue
			0x5678,  //wIndex
			buffer,
			len,     //wLength  (more like max length)
			100 );
		show_output = 0;
#elif 0
		r1 = libusb_control_transfer( devh,
			0x00,    //reqtype  (0x80 = Device->PC, 0x00 = PC->Device)
			0xA1,    //Read RAM
			0x3FFE,  //MSB
			0x0000,  //LSB
			buffer,
			len,     //wLength  (more like max length)
			100 );
#elif 0
		r1 = libusb_control_transfer( devh,
			0x00,    //reqtype  (0x80 = Device->PC, 0x00 = PC->Device)
			0xA2,    //request (Read Flash)
			0x0402,  //wValue (MSB = # of 16-byte words to read, LSB = MSB of address x4)
			0x0000,  //lsb of address x4
			buffer,
			1,     //wLength  (more like max length)
			100 );
#elif 0
		r1 = libusb_control_transfer( devh,
			0x00,    //reqtype  (0x80 = Device->PC, 0x00 = PC->Device)
			0xA3,    //request (Write Flash)
			0x0002,  //wValue (MSB = # of 16-byte words to read, LSB = MSB of address x4)
			0x0000,  //lsb of address x4
			buffer,
			len,     //wLength  (more like max length)
			100 );
#else 0
		r1 = libusb_control_transfer( devh,
			0x00,    //reqtype  (0x80 = Device->PC, 0x00 = PC->Device)
			0xA4,    //request (Erase Flash Block)
			0x0008,  //Block ID (wValue)
			0x0000,  //
			buffer,
			1,     //wLength  (more like max length)
			100 );
#endif
		memset( buffer, 0, sizeof( buffer ) );

		r2 = libusb_control_transfer( devh,
			0x80,    //reqtype  (0x80 = in, 0x00 = out)
			0xA0,    //request
			0x0100,  //wValue
			0x0000,  //wIndex
			buffer, //wLength  (more like max length)
			sizeof( buffer ),
			100 );

		buffer[sizeof(buffer)-1] = 0;
		printf( "." );
		fflush( stdout );
		fprintf( stderr, "%d/%d/", r1, r2 );
		if( show_output )
		{
			for( i = 0; i < r2; i++ )
			{
				fprintf( stderr, "%02x ", buffer[i] );
			}
		}
		fprintf( stderr, "\n" );
		if( r1 < 0 || r2 < 0 ) return -1;
	}

	return 0;
}

