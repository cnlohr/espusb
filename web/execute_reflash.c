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
#include "md5.h"
#include <libusb-1.0/libusb.h>

struct libusb_device_handle *devh = NULL;



#define BLOCK_SIZE 65536
#define SECTOR_SIZE 4096
#define PADDING 1024

int sendsize_max = PADDING;
int use_usb = 0;


int sockfd;
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
		//printf( "Sent: %d/%d   %c%c", r1, len, buffer[0], buffer[1] );
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
		char recvline[10000];
		int tries = 0;
		for( tries = 0; tries < 10; tries++ )
		{
			usleep( 500 );

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
		while( tvb.tv_sec - tva.tv_sec < 3 )
		{
			struct pollfd ufds;
			ufds.fd = sockfd;
			ufds.events = POLLIN;
			int rv = poll(&ufds, 1, 100);
			if( rv > 0 )
			{
				char recvline[10000];
				int n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
	//			printf( "%s === %s\n", recvline, match );
				if( strncmp( recvline, match, strlen( match ) ) == 0 )
				{
					printf( "Ok - " ); fflush( stdout );
					return 0;
				}
			}
			gettimeofday( &tvb, 0 );
		}
		return 1;
	}
}

uint32_t Push( uint32_t offset, const char * file )
{
	char sendline[1000];
	char recvline[1000];

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



	int devo = 0;
	int lastblock = -1;
	int keep_padding = 0;
	int retdevo = 0;
	while( keep_padding || !feof( f ) )
	{
		int tries;
		int thissuccess;
		char buffer[PADDING];
		char bufferout[2000];

		int reads = fread( buffer, 1, sendsize_max, f );
		int sendplace = offset + devo;
		int sendsize = sendsize_max;
		int block = sendplace / BLOCK_SIZE;

		memset( buffer + reads, 0, sendsize-reads );

		if( block != lastblock )
		{
			char se[64];
			int sel = sprintf( se, "FB%d\r\n", block );
	
			thissuccess = 0;
			for( tries = 0; tries < 10; tries++ )
			{
				char match[75];
				//printf( "Erase: %d\n", block );
				printf( "E" ); fflush( stdout );
				SendData(  se, sel );
				sprintf( match, "FB%d", block );

				if( PushMatch(match) == 0 ) { thissuccess = 1; break; }
				printf( "Retry.\n" );
			}
			if( !thissuccess )
			{
				fprintf( stderr, "Error: Timeout in communications.\n" );
				exit( -6 );
			}

			lastblock = block;
		}

		
		int r = sprintf( bufferout, "FW%d\t%d\t", sendplace, sendsize );
		memcpy( bufferout + r, buffer, sendsize ); 

		//printf( "bufferout: %d %d\n", sendplace, sendsize );
		printf( "." ); fflush( stdout );

		thissuccess = 0;
		for( tries = 0; tries < 10; tries++ )
		{
			char match[75];
			SendData( bufferout, sendsize + r );
			sprintf( match, "FW%d", sendplace );

			if( PushMatch(match) == 0 ) { thissuccess = 1; break; }
			printf( "Retry.\n" );
		}
		if( !thissuccess )
		{
			fprintf( stderr, "Error: Timeout in communications.\n" );
			exit( -6 );
		}

		if( reads != 0 )
		{
			devo += sendsize;
			retdevo = devo;
		}

		//Tricky: If we are using a smaller sendsize than the pad size, keep padding with 0's.
		if( PADDING!=sendsize && (devo & (PADDING-1)) && feof( f ) )
		{
			keep_padding = 1;
			if( reads == 0 )
				devo += sendsize;
		}
		else
		{
			keep_padding = 0;
		}
	}

	return retdevo;
}

void ComputeMD5WithKey( char * md5retText, const char * filename, const char * key )
{
	uint8_t retmd5[16];
	MD5_CTX ctx;
	FILE * f = fopen( filename, "rb" );
	if( !f )
	{
		fprintf( stderr, "Error opening %s\n", filename );
		exit( -9 );
	}

	fseek( f, 0, SEEK_END );
	int l = ftell( f );
printf("MD5 Size: %d\n", l );
	int padl = ((l-1) / sendsize_max)*sendsize_max+sendsize_max;
printf("MD5 Pad: %d\n", padl );
	fseek( f, 0, SEEK_SET );
	uint8_t data[padl];
	fread( data, l, 1, f );
	fclose( f );

	memset( data+l, 0, padl-l );
	MD5_Init( &ctx );
	if( !strlen(key) )
		MD5_Update( &ctx, key, strlen( key ) );
	MD5_Update( &ctx, data, padl );
	MD5_Final( retmd5, &ctx );

	for( l = 0; l < 16; l++ )
	{
		sprintf( md5retText + l*2, "%02x", retmd5[l] );
	}

	return;
}

uint32_t roundup( uint32_t r )
{
	return ((r-1)&(~0xFFF))+0x1000;
}


int main(int argc, char**argv)
{
	int n;

	char sendline[1000];
	char recvline[1000];

	char md5_f1[48];
	char md5_f2[48];

	if (argc < 4 )
	{
		printf("usage: pushtodev [IP address] [file_lower] [file_upper] [key (optional)]\n");
		exit(-1);
	}

	const char * file1 = argv[2];
	const char * file2 = argv[3];



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

	uint32_t fs1 = Push( 0x080000, file1 );
	uint32_t fs2 = Push( 0x0c0000, file2 );

	if( !fs1 || !fs2 )
	{
		fprintf( stderr, "Error: File size not acceptable.\n" );
		return 0;
	}

	const char * dat = "";
	if( argc == 5 )
	{
		dat = argv[4];
	}

	ComputeMD5WithKey( md5_f1, file1, dat );
	ComputeMD5WithKey( md5_f2, file2, dat );

	printf( "%s %s\n", md5_f1, md5_f2 );

	//FM[from_address]\t[to_address]\t[size]\t[MD5(key+data)]\t[from_address]\t[to_address]\t[size]\t[MD5(key+data)]

	char cmd[1024];

	sprintf( cmd, "FM%d\t%d\t%d\t%s\t%d\t%d\t%d\t%s\n", 
		0x080000,
		0x000000,
		fs1, //roundup( fs1 ),
		md5_f1,
		0x0C0000,
		0x040000,
		fs2, //roundup( fs2 ),
		md5_f2 );

	printf( "Issuing: %s\n", cmd );
	SendData( cmd, strlen(cmd) );
	usleep(10000);
	SendData( cmd, strlen(cmd) );

	struct pollfd ufds;
	ufds.fd = sockfd;
	ufds.events = POLLIN;
	int rv = poll(&ufds, 1, 100);
	if( rv > 0 )
	{
		char recvline[10000];
		int n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);

		printf( "Response: %s\n",recvline ); 
		return 0;
	}
	else
	{
		printf( "Timeout. Good? Maybe?\n" );
		return 0;
	}

	return 0;
}


