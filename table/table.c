#include <stdio.h>
#include <stdint.h>

//NOTES: I have a hunch a table is best for this, though, we might have plenty of free time
//If that's the case, this should just be a test of the bit logic.

//#define DPLUSFIRST
//if so, 1's place = D+ bit
//       2's place = D- bit


//Example from wikipedia
//uint8_t input_stream[] = { 1, 1, 2, 2, 2, 1, 1, 2, 0, 0, 1 };
//uint8_t input_stream[] = { 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 0, 0, 1 };
//uint8_t input_stream[] = { 1, 2, 1, 2, 1, 2, 1, 1, 1, 2, 2, 2, 1, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 0, 0,  };

//"Setup"
//uint8_t input_stream[] = { 1, 2, 1, 2, 1, 2, 1, 2, 2, 2, 1, 1, 1, 2, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 2, 1, 2, 1, 0, 0,  };

//"Data" after "setup"
#if 0
uint8_t input_stream[] = { 1, 2, 1, 2, 1, 2, 1, 2, 2, 2, 2, 1, 2, 1, 2, 2, 2, 1, 2,  1, 2, 1, 2, 1, 1, 2, 2, 2, 1 ,2, 1, 2,
	1, 2, 1, 2, 1, 2, 1, 2, 1, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,  1, 2, 1, 2, 1, 2, 2, 1, 2, 
	1, 2, 1, 2, 1, 2, 1, 1, 2, 2, 2, 2, 1, 1, 1, 2, 1, 1, 2, 2, 1, 2, 2, 0, 0,};
#endif

//"In"
#if 0
uint8_t input_stream[] = { 
	1, 2, 1, 2, 1, 2, 1, 2, 2, 2, 1, 2, 2, 1, 1, 1, 2, 1, 2, 1, 2, 1, 2, 1,2, 1, 2, 1, 2, 1, 2, 1, 2, 2, 1, 2, 1, 0, 0 };
#endif

//Us trying to send a byte
uint8_t input_stream[] = { 1, 2, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2,
	1, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1,
	2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1	,0,0 };

//Table bits:
// LSB: D+
//      D-
//  last_state
//  ones_count[0]
//  ones_count[1]
//  ones_count[2]


//Return value:
//  0th bit: value      (1 if end of packet, 0 if bad)
//  1st bit: emit bit? 
//
// 7th bit: exit


uint8_t state;

#define TABLESIZE 64
uint8_t table[TABLESIZE];

void Init()
{
	int c = 0;
	for( c = 0; c < TABLESIZE; c++ )
	{
#ifdef DPLUSFIRST
		int dp = (c&1)?1:0;
		int dm = (c&2)?1:0;
#else
		int dp = (c&2)?1:0;
		int dm = (c&1)?1:0;
#endif
		int last_state = (c&4)?1:0;
		int ones_count = c>>3;

		int value = 0;
		int emit = 0;
		int err = 0;
		int exval = 0;

		//Found preamble.
		if( dm == 0 && dp == 0 )
		{
			emit = 0;
			value = (ones_count != 7)?1:0; //End-of-packet. (unless we are in the preamble)ss
			exval = 1;
			last_state = 0;
			ones_count = 0;
		}
		else if( dm == 1 && dp == 1 )
		{
			emit = 0;
			value = 0;
			exval = 1;
			last_state = 0;
			ones_count = 0;
		}
		//Finding preamble.
		else if( ones_count == 7 )
		{
			if( last_state == dp )
			{
				//End of preamble.
				if( dp == 0 )
				{
					//Good
					ones_count = 0;
				}
				else
				{
					exval = 1;
					//Bad
				}
			}
			last_state = dp;
		}
		else if( dp != last_state )       //State transition (would emit 0)
		{
			if( ones_count == 6 )   //This is a bit stuffed
			{
				ones_count = 0;
				emit = 0;
				value = 0;
			}
			else
			{
				ones_count = 0;
				emit = 1;
				value = 0;
			}
			last_state = dp;
		}
		else if( dp == last_state )
		{
			if( ones_count == 6 ) //This is an error case.
			{
				ones_count = 0;
				exval = 1;
			}
			else
			{
				emit = 1;
				value = 1;
				ones_count++;
			}
			last_state = dp;
		}
		else
		{
			fprintf( stderr, "Fault.  This is an unexpected case.\n" );
		}
		
		table[c] = value | (emit<<1) | (last_state<<2) | (ones_count<<3) | (exval<<7);
	}
}

int RunState( int i )
{
	return table[i];
}

int main()
{
	int i;
	Init();

	uint8_t c = 0x38;// | 0x4; //7 Count, was 1.

	uint8_t outbuff[32];
	int outplace = 0;
	int outbitplace = 0;
	uint8_t outbyte = 0;

uint32_t crc16=0xffff;//0xffff;
//#define POLY 0x8005
#define POLY 0xA001
#define CHECKGOOD 0xB001
//#define POLY 0xC002

#define CHECK5GOOD 0x06
#define CRC5POLY 0x14
uint16_t crc5=0x1f;


	for( i = 0; i < sizeof( input_stream ); i++ )
	{
#ifdef DPLUSFIRST
		c |= input_stream[i];
#else
		int bit1 = (input_stream[i]&2)?1:0;
		int bit2 = (input_stream[i]&1)?2:0;
		c |= bit1|bit2;
#endif
		printf( "C IN: (%d) %02x / ", input_stream[i], c );
		c = RunState( c );
		printf( "%02x - ", c );
		if( c & 2 ) {
			if( c&1 )
				outbyte |= 1<<outbitplace;
			outbitplace++;
			printf( "E: %d", c & 1); 
			if( outbitplace == 8 ) 
			{
				outbuff[outplace++] = outbyte;
				outbyte = 0; 
				outbitplace = 0;
			}
		}
		printf( "\n" );
		if( c & 0x80 )
		{
			printf( "Exit.\n" );
		}
		c &= 0xfc;
	}

	printf( "\n" );
	for( i = 0; i < outplace; i++ )
	{
		printf( "%02x ", outbuff[i] );
	}
	printf( "\n" );

	outbuff[1] = 0x00;

#if 1
	//Note that this operates differently than table2.
	for( i = 1; i < outplace; i++ )
	{
		int byte = outbuff[i];
		int nextmask = POLY<<1;

		crc16^=byte;

		int k;
		for( k = 0; k < 8; k++ )
		{
			if( crc16 & 0x01 )
			{
				crc16^=nextmask;
			}
			crc16>>=1;
		}
		printf( "%08x\n", crc16 );
	}
#else
	//Another way to calculate CRC

	//01001000 10000000 11001100 11110100
	outbuff[1] = 0b00010010;
	outbuff[2] = 0b10000000;
	outbuff[3] = 0b11001100;
	outbuff[4] = 0b11110100;
	outplace = 5;

	for( i = 1; i < outplace; i++ )
	{
		int byte = outbuff[i];

		//crc16^=byte;

		int k;
		for( k = 0; k < 8; k++ )
		{
			if( (crc16 ^ (byte>>k)) & 0x01 )
			{
				crc16>>=1;
				crc16^=POLY;
			}
			else
			{
				crc16>>=1;
			}
		}
		printf( "%08x\n", crc16 );
	}
#endif

	FILE * f = fopen( "../user/usb_table_1bit.h", "w" );
	fprintf( f, "#define NEED_CONSTANTS\n\n" );
	fprintf( f, "#include \"usb.h\"\n\n" );

	fprintf( f, "#define TABLE_OFFSET 60\n\n" );
	fprintf( f, "#define GPIO_BASE_OFFSET 0\n" );
	fprintf( f, "#define LOOP_COUNT_OFFSET 12\n" );
	fprintf( f, "#define ANDING_MASK_OFFSET 24\n" );
	fprintf( f, "#define RUNNING_TIMEOUT_OFFSET  28\n" );
	fprintf( f, "#define USB_INTERNAL_STATE_OFFSET  32\n" );
	fprintf( f, "#define CRC16_INIT_OFFSET 40\n" );
	fprintf( f, "#define CRC16_POLY_OFFSET 44\n" );
	fprintf( f, "#define CRC16_CHECK_OFFSET 48\n" );
	fprintf( f, "#define CRC5_INITIAL 0x1f\n" );
	fprintf( f, "#define CRC5_POLY    0x14\n" );
	fprintf( f, "#define CRC5_CHECK   0x06\n" );

	fprintf( f, "#ifndef _INASM_\n\n" );
	fprintf( f, "uint32_t usb_ramtable[%d] __attribute__((aligned(16))) = {\n", (60 + TABLESIZE)/4 );
	fprintf( f, "\tGPIO_BASE_REG, // Base reg for all GPIO (Offset 0)\n" );
	fprintf( f, "\t0, // Reserved                          (Offset 1)\n" );
	fprintf( f, "\t0, // Reserved                          (Offset 2)\n" );
	fprintf( f, "\t100, // Loop Count                      (Offset 3)\n" );
	fprintf( f, "\t0, // Reserved                          (Offset 4)\n" );
	fprintf( f, "\t0, // Reserved                          (Offset 5)\n" );
	fprintf( f, "\t0x3c, // Anding mask for table          (Offset 6)\n" );
	fprintf( f, "\t136, //Timeout (in bits)                (Offset 7)\n" );
	fprintf( f, "\t(uint32_t)&usb_internal_state,  //State (Offset 8)\n" );
	fprintf( f, "\t0x00, //Reserved                        (Offset 9)\n" );
	fprintf( f, "\t0xffff, //Initial value for CRC         (Offset 10)\n");
	fprintf( f, "\t0xA001, //CRC Poly                      (Offset 11)\n");
	fprintf( f, "\t0xB001, //CRC Check                     (Offset 12)\n");
	fprintf( f, "\t0x00, //Reserved                        (Offset 13)\n");
	fprintf( f, "\t0x00, //Reserved                        (Offset 14)\n");

	for( i = 0; i < TABLESIZE/4; i++ )
	{
		uint32_t outval = table[i*4] | (table[i*4+1]<<8)  | (table[i*4+2]<<16)  | (table[i*4+3]<<24);
		if( i % 4 == 0 ) fprintf( f, "\n\t" );
		fprintf( f, "0x%08x, ", outval );
	}
	fprintf( f, "};\n" );
	fprintf( f, "#endif\n" );
	fclose( f );

}

