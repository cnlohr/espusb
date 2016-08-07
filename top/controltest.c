#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>

struct libusb_device_handle *devh = NULL;

int doexit = 0;

void sigexit( int sig )
{
	doexit = 1;
}

//#define DOTTER
#define DATASIZE 32

int main()
{
	int frame = 0, i, r;

	if( libusb_init(NULL) < 0 )
	{
		fprintf( stderr, "Error: Could not initialize libUSB\n" );
		return -1;
	}

	devh = libusb_open_device_with_vid_pid( NULL, 0xabcd, 0x8266 );

	if( !devh )
	{
		fprintf( stderr,  "Error: Cannot find device.\n" );
		return -1;
	}

	libusb_detach_kernel_driver( devh, 0 );

	if( (r=libusb_claim_interface(devh, 0)) < 0 )
	{
		fprintf(stderr, "usb_claim_interface error %d\n", r);
		return -1;
	}

	printf( "Interface ok!\n" );

	signal( SIGINT, sigexit );
	signal( SIGTERM, sigexit );
	signal( SIGQUIT, sigexit );

	while( !doexit )
	{
		unsigned char rbuffer[DATASIZE];

//		if( libusb_handle_events(NULL) < 0 )
//		{
//			doexit = 1;
//			}
		//

//int LIBUSB_CALL libusb_control_transfer(libusb_device_handle *dev_handle,
//	uint8_t request_type, uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
//	unsigned char *data, uint16_t wLength, unsigned int timeout);

#ifdef NUMERINGTEST		
		memset( rbuffer, 0, DATASIZE );
		for( i = 0; i < DATASIZE; i++ )
		{
			rbuffer[i] = i + frame;
		}
		int outsize = DATASIZE;
#else
		sprintf( rbuffer, "EHe%04x", frame );
		int outsize = strlen( rbuffer );
#endif


		int r1 = libusb_control_transfer( devh,
			0x00,    //reqtype  (0x80 = Device->PC, 0x00 = PC->Device)
			0xA6,    //request
			0x0100,  //wValue
			0x0000,  //wIndex
			rbuffer, 
			outsize,     //wLength  (more like max length)
			1000 );


		usleep( 1000 );

		int r2 = libusb_control_transfer( devh,
			0x80,    //reqtype  (0x80 = in, 0x00 = out)
			0xA7,    //request
			0x0100,  //wValue
			0x0000,  //wIndex
			rbuffer, //wLength  (more like max length)
			DATASIZE,
			1000 );

		if( r2 > 0 ) { 		frame++; } 

		if( r1 < -1 && r2 < -1 )
		{
			break;
		}
#ifdef DOTTER
		if( r2 >= 0 )
		{
			printf( "." );
			fflush( stdout );
		}
		else
		{
			fprintf( stderr, "Error: %d\n", r );
		}

#else

		printf( "%d %d ", r1, r2 );
		for( i = 0; i < r2; i++ )
				printf( " %02x ", rbuffer[i] );
		printf( "\n" );

#endif


		usleep(1000);
	}

	libusb_release_interface( devh, 0 );
	libusb_attach_kernel_driver( devh, 0 );
	libusb_close( devh );
	libusb_exit( NULL );
	printf( "Exit ok.\n" );
	return -1;

}

