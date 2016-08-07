//Unless what else is individually marked, all code in this file is
//Copyright 2015 <>< Charles Lohr Under the MIT/x11 License, NewBSD License or
// ColorChord License.  You Choose.

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

struct libusb_device_handle *devh = NULL;

#define sector_SIZE 4096
int sockfd;
char recvline[10000];

int use_usb = 0;
int sendsize_max = 1024; //For USB it will be smaller
struct sockaddr_in servaddr,cliaddr;

int SendData( uint8_t * buffer, int len )
{
	if( use_usb )
	{
		int r1 = libusb_control_transfer( devh,
			0x00,    //reqtype  (0x80 = Device->PC, 0x00 = PC->Device)
			0xA6,    //request
			0x0100,  //wValue
			0x0000,  //wIndex
			buffer, 
			len,     //wLength  (more like max length)
			1000 );
	}
	else
	{
		return sendto( sockfd, buffer, len, MSG_NOSIGNAL, (struct sockaddr *)&servaddr,sizeof(servaddr));
	}
}


int PushMatch( const char * match )
{
	if( use_usb )
	{
		int tries = 0;
		for( tries = 0; tries < 10; tries++ )
		{
			usleep( 1000 );

			int r2 = libusb_control_transfer( devh,
				0x80,    //reqtype  (0x80 = in, 0x00 = out)
				0xA7,    //request
				0x0100,  //wValue
				0x0000,  //wIndex
				recvline, //wLength  (more like max length)
				128,
				1000 );

			if( r2 < 0 ) continue;

			recvline[r2] = 0;

			if( strncmp( recvline, match, strlen( match ) ) == 0 ) //XXX? Should this be memcmp?
			{
				return 0;
			}

			usleep( 20000 );
		}
		return 1;
	}
	else
	{
		struct timeval tva, tvb;
		gettimeofday( &tva, 0 );
		gettimeofday( &tvb, 0 );
		while( tvb.tv_sec - tva.tv_sec < 3 ) //3 second timeout.
		{
			struct pollfd ufds;
			ufds.fd = sockfd;
			ufds.events = POLLIN;
			int rv = poll(&ufds, 1, 500);
			if( rv > 0 )
			{
	//			tbuf = recvline;
				int n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
				if( strncmp( recvline, match, strlen( match ) ) == 0 )
				{
					return 0;
				}
			}
			gettimeofday( &tvb, 0 );
		}
		return 1;
	}
}


int main(int argc, char**argv)
{
	int n;
	char sendline[1000];
//	char recvline[1000];

	if (argc != 4)
	{
		printf("usage: pushtodev [IP address] [address offset] [file]\n");
		exit(-1);
	}

	int offset = atoi( argv[2] );
	const char * file = argv[3];

	if( offset <= 0 )
	{
		fprintf( stderr, "Error: Cannot write to address 0 or before.\n" );
		exit(-2);
	}

	FILE * f = fopen( file, "rb" );
	if( !f || feof( f ) )
	{
		fprintf( stderr, "Error: cannot open file.\n" );
		exit(-3);
	}


	if( strcmp( argv[1], "USB" ) == 0 )
	{
		int r;
		use_usb = 1;
		printf( "Connecting by USB\n" );
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

		libusb_detach_kernel_driver( devh, 0 );

		if( (r=libusb_claim_interface(devh, 0)) < 0 )
		{
			fprintf(stderr, "usb_claim_interface error %d\n", r);
			return -1;
		}
		printf( "Connected.\n" );
		//USB is attached
		sendsize_max = 128;
	}
	else
	{
		sockfd=socket(AF_INET,SOCK_DGRAM,0);

		bzero(&servaddr,sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr=inet_addr(argv[1]);
		servaddr.sin_port=htons(7878);
	}

	int devo = 0;
	int lastsector_block = -1;
	int resend_times = 0;
	int r;
	while( !feof( f ) )
	{
		int tries;
		int thissuccess;
		char buffer[1024];
		char bufferout[1600];
		int reads = fread( buffer, 1, sendsize_max, f );
		int sendplace = offset + devo;
		int sendsize = reads;

#ifdef SECTOR
		int sector = sendplace / sector_SIZE;
		if( sector != lastsector_block )
		{
			char se[64];
			int sel = sprintf( se, "FE%d\r\n", sector );
	
			thissuccess = 0;
			for( tries = 0; tries < 10; tries++ )
			{
				char match[75];
				printf( "Erase: %d\n", sector );
				SendData( se, sel );
				sprintf( match, "FE%d", sector );

				if( PushMatch(match) == 0 ) { thissuccess = 1; break; }
				printf( "Retry.\n" );
			}
			if( !thissuccess )
			{
				fprintf( stderr, "Error: Timeout in communications.\n" );
				exit( -6 );
			}

			lastsector_block = sector;
		}
#else //block

		int block = sendplace / 65536;
		if( block != lastsector_block )
		{
			char se[64];
			int sel = sprintf( se, "FB%d\r\n", block );
	
			thissuccess = 0;
			for( tries = 0; tries < 10; tries++ )
			{
				char match[75];
				printf( "Erase: %d\n", block );
				SendData( se, sel );
				sprintf( match, "FB%d", block );

				if( PushMatch(match) == 0 ) { thissuccess = 1; break; }
				printf( "Retry.\n" );
			}
			if( !thissuccess )
			{
				fprintf( stderr, "Error: Timeout in communications.\n" );
				exit( -6 );
			}

			lastsector_block = block;
		}
#endif
		resend_times = 0;
resend:
		r = sprintf( bufferout, "FW%d\t%d\t", sendplace, sendsize );
		printf( "bufferout: %d %d %s\n", sendplace, sendsize, bufferout );
		memcpy( bufferout + r, buffer, sendsize );


		thissuccess = 0;
		for( tries = 0; tries < 10; tries++ )
		{
			char match[75];
			SendData( bufferout, reads + r );
			sprintf( match, "FW%d", sendplace );

			if( PushMatch(match) == 0 ) { thissuccess = 1; break; }
			printf( "Retry.\n" );
		}
		if( !thissuccess )
		{
			fprintf( stderr, "Error: Timeout in communications.\n" );
			exit( -6 );
		}

/*
		printf( "Verifying..." );
		fflush( stdout );

		int r = sprintf( bufferout, "FR%d:%d", sendplace, sendsize );
		bufferout[r] = 0;

		thissuccess = 0;
		for( tries = 0; tries < 10; tries++ )
		{
			char match[1600];
			sendto( sockfd, bufferout, r, MSG_NOSIGNAL, (struct sockaddr *)&servaddr,sizeof(servaddr));
			devo += reads;
			sprintf( match, "FR%08d", sendplace );

			if( PushMatch(match) == 0 ) {

				//Check data...
//printf( "RR:%s\n", recvline );
				char * colon1 = strchr( recvline, ':' );
				char * colon2 = (colon1)?strchr( colon1+1, ':' ):0;
//printf( "::%p %p \"%s\"\n", colon1, colon2,recvline );
				if( colon2 )
				{
					if( memcmp( colon2+1, buffer, sendsize ) == 0 )
						thissuccess = 1;
				}

				if( !thissuccess )
				{
					if( resend_times > 2 )
					{
						break;
					}
					resend_times++;
					goto resend;
				}
				break;
			}
			printf( "Retry.\n" );
		}
		if( !thissuccess )
		{
			fprintf( stderr, "Error: Fault verifying.\n" );
			exit( -6 );
		}
*/
			devo += reads;

	}

	return 0;
}
