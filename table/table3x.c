/////////////////
///XXX TODO TODO TODO
///
/// Should use a 3x bit table, not a 2x!  This will make things tighter and cleaner.
///
/// CONSIDER: Doing fault/term detection outside of this.
///    Inputs:
///       bit0 bit1 bit2 [State, count (3 bits) + Last]
///    Outputs:
///       bit0 bit1 bit2    # of bits


#include <stdio.h>
#include <stdint.h>

//
// This is my rapid prototyping file for playing with different aspects of the USB stuff.
//
//
//


//NOTES: I have a hunch a table is best for this, though, we might have plenty of free time
//If that's the case, this should just be a test of the bit logic.



//1's place = D+ bit
//2's place = D- bit

//Master?
#if 1
uint8_t input_stream[] = { 2, 1, 2, 1, 2, 1, 2, 2, 2, 1, 1, 1, 2, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 2, 1, 2, 1, 0, 0,  };
#endif

//Second byte (a DATA0 Byte)!
#if 0
uint8_t input_stream[] = { 2, 1, 2, 1, 2, 1, 2, 2, 2, 2, 1, 2, 1, 2, 2, 2, 1, 2,  1, 2, 1, 2, 1, 1, 2, 2, 2, 1 ,2, 1, 2,
	1, 2, 1, 2, 1, 2, 1, 2, 1, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,  1, 2, 1, 2, 1, 2, 2, 1, 2, 
	1, 2, 1, 2, 1, 2, 1, 1, 2, 2, 2, 2, 1, 1, 1, 2, 1, 1, 2, 2, 1, 2, 2, 0, 0,};
#endif

uint8_t input_stream2x[50];

//Table bits:
// LSB2 D+
//      D-
// LSB1 D+ (Least recent)
//      D-
//  last_state
//  ones_count[0]
//  ones_count[1]
//  ones_count[2]

//Return value:
//  bit to emit 0
//  bit to emit 1
//  emit 0
//  emit 1
//
//If codes:
//  emit 1 / emit 0 / bit  1 / bit  0
//     1       1         1       0      =   Emit a 0 (And exit)
//     1       1         1       1      =   emit a 1 (And exit)
//     1       1         0       0      =   Do not emit, end of data.
//                                          ones_count = 0 = OK! End of Packet
//                                          ones_count = 7 > Error
//
//     1       0         x       x      =   Ok!
//     0       1         x       x      =   Ok!
//     0       0         x       x      =   Ok! (No emission)

uint8_t state;

#define TABLESIZE 256
uint8_t table[TABLESIZE];

void Init()
{
	int c = 0;
	for( c = 0; c < TABLESIZE; c++ )
	{
		int dp1 = (c&1)?1:0;
		int dm1 = (c&2)?1:0;
		int dp0 = (c&4)?1:0;
		int dm0 = (c&8)?1:0;
		int last_state = (c&16)?1:0;
		int ones_count = c>>5;

		int value0 = 0;
		int value1 = 0;
		int emit = 0;

		int exval = 0;  //If 1,  will set emit = 3, ones_count = 0 (End of packet)
						//If 2,  will set emit = 3, ones_count = 7 (Error)

		{
			if( dm0 == 0 && dp0 == 0 )  //End of packet, or error.
			{
				exval = (ones_count == 7)?2:1; //If in preamble, this is an error.
			}
			else if( dm0 == 1 && dp0 == 1 ) //Error State
			{
				exval = 2;
			}
			//Finding preamble?
			else if( ones_count == 7 )
			{
				if( last_state == dp0 )
				{
					//End of preamble.
					if( dp0 == 0 )
					{   //Good
						ones_count = 0;
					}
					else
					{   //Bad
						exval = 2;
					}
				}
				last_state = dp0;
			}
			else if( dp0 != last_state )       //State transition (would emit 0)
			{
				if( ones_count != 6 )   //This is a bit stuffed
				{
					emit = 1;
					value0 = 0;
				}
				ones_count = 0;
			}
			else if( dp0 == last_state )
			{
				if( ones_count == 6 ) //This is an error case.
				{
					exval = 2;
				}
				else
				{
					emit = 1;
					value0 = 1;
					ones_count++;
				}
			}
			else
			{
				fprintf( stderr, "Fault.  This is an unexpected case.\n" );
			}

			last_state = dp0;
		}

		if( !exval )
		{
			if( dm1 == 0 && dp1 == 0 )  //End of packet, or error.
			{
				exval = (ones_count == 7)?2:1; //If in preamble, this is an error.
			}
			else if( dm1 == 1 && dp1 == 1 ) //Error State
			{
				exval = 2;
			}
			//Finding preamble?
			else if( ones_count == 7 )
			{
				if( last_state == dp1 )
				{
					//End of preamble.
					if( dp1 == 0 )
					{   //Good
						ones_count = 0;
					}
					else
					{   //Bad
						exval = 2;
					}
				}
				last_state = dp1;
			}
			else if( dp1 != last_state )       //State transition (would emit 0)
			{
				if( ones_count != 6 )   //This is a bit stuffed
				{
					if( emit ) { emit = 2; value1 = 0; }
					else { emit = 1; value0 = 0; }
				}
				ones_count = 0;
			}
			else if( dp1 == last_state )
			{
				if( ones_count == 6 ) //This is an error case.
				{
					exval = 2;
				}
				else
				{
					if( emit ) { emit = 2; value1 = 1; }
					else { emit = 1; value0 = 1; }
					ones_count++;
				}
			}
			else
			{
				fprintf( stderr, "Fault.  This is an unexpected case.\n" );
			}

			last_state = dp1;
		}


		//Tricky - we have no way to emit twice and error on the same frame... Thankfully that doesn't happen in USB.
		if( exval == 1 )
		{
			if( emit ) value1 = 1;
			emit = 3;
			ones_count = 0;
		}
		else if( exval == 2 )
		{
			if( emit ) value1 = 1;
			emit = 3;
			ones_count = 7;
		}


		table[c] = value0 | (value1<<1) | (emit<<2) | (last_state<<4) | (ones_count<<5);
	}
}

int RunState( int i )
{
	return table[i];
}

uint8_t bufferout[1024];
int byteplace;

uint8_t bytea;
int bitplace;

uint16_t crc16=0xffff;//0xffff;
//#define POLY 0x8005
#define POLY 0xA001
#define CHECKGOOD 0xB001
//#define POLY 0xC002

#define CHECK5GOOD 0x06
#define CRC5POLY 0x14
uint16_t crc5=0x1f;

void TackOut( int bit )
{
	if( byteplace > 0 )
	{
		if( bit ^ (crc16&1) )
		{
			crc16 = crc16>>1;
			crc16 ^= POLY; 
		}
		else
		{
			crc16 = crc16>>1;
		}
	}

	if( byteplace > 0 )
	{
		if( bit ^ (crc5&1) )
		{
			crc5 = crc5>>1;
			crc5 ^= CRC5POLY; 
		}
		else
		{
			crc5 = crc5>>1;
		}
	}

	bytea |= bit<<bitplace;
	printf( "E: %d (%02x) ", bit, bytea );
	bitplace++;
	if( bitplace == 8 )
	{
		printf( "(%04x %04x)", crc16, crc5 );
		bufferout[byteplace++] = bytea;
		bytea = 0;
		bitplace = 0;
	}
}

int main()
{
	int i;
	Init();

uint8_t input_stream2x[50];
	for( i = 0; i < sizeof( input_stream ) / 2; i++ )
	{
		input_stream2x[i] = (input_stream[i*2+0] << 2) | input_stream[i*2+1];
	}

	uint8_t c = 0xf0; //7 Count, was 1.

	for( i = 0; i < sizeof( input_stream )/2; i++ )
	{
		c |= input_stream2x[i];

		printf( "C IN: (%1x %1x) %02x / ", input_stream2x[i]>>2, input_stream2x[i]&0x03, c );
		c = RunState( c );
		printf( "%02x - ", c );
		if( (c & 0x0c) == 0x0c )
		{
			printf( "Error or exit" );
			if( c & 0x02 ) TackOut( c&1 );
		}
		else
		{
			if( c & 0x04 ) TackOut(c & 1);
			if( c & 0x08 ) { TackOut( c&1); TackOut( (c&2)?1:0 ); }
		}
		printf( "\n" );
		if( (c & 0x0c) == 0x0c )
		{
			printf( "Exit (Code %d)\n", c>>5 );
		}
		c &= 0xf0;
	}


	FILE * f = fopen( "../user/usb_table_2bit.h", "w" );
	fprintf( f, "#define NEED_CONSTANTS\n\n" );
	fprintf( f, "#include \"usb.h\"\n\n" );

	fprintf( f, "#define TABLE_OFFSET 32\n\n" );

	fprintf( f, "#define PIN_OUT_SET_OFFSET 4\n" );
	fprintf( f, "#define GPIO_STATUS_W1TC_OFFSET 16\n" );
	fprintf( f, "#define GPIO_STATUS_OFFSET 20\n\n" );

	fprintf( f, "#ifndef _INASM_\n\n" );
	fprintf( f, "uint32_t usb_ramtable[%d] __attribute__((aligned(16))) = {\n", (32 + TABLESIZE)/4 );
	fprintf( f, "\tPIN_IN, // Reading Pins        (Offset 0)\n" );
	fprintf( f, "\tPIN_OUT_SET, // Debug Output   (Offset 1)\n" );
	fprintf( f, "\tPIN_OUT_CLEAR, // Debug Output (Offset 2)\n" );
	fprintf( f, "\t0x000000f0, // Anding mask     (Offset 3)\n" );
	fprintf( f, "\tGPIO_STATUS_W1TC, // For int   (Offset 4)\n" );
	fprintf( f, "\tGPIO_STATUS, // For int        (Offset 5)\n" );
	fprintf( f, "\t0, 0, ");
	for( i = 0; i < TABLESIZE/4; i++ )
	{
		uint32_t outval = table[i*4] | (table[i*4+1]<<8)  | (table[i*4+2]<<16)  | (table[i*4+3]<<24);
		if( i % 4 == 0 ) fprintf( f, "\n\t" );
		fprintf( f, "0x%08x, ", outval );
	}
	fprintf( f, "};\n" );
	fprintf( f, "#endif\n" );
	fclose( f );

	printf( "%d %d %d\n", byteplace, bytea, bitplace );

	for( i = 0; i < byteplace; i++ )
	{
		printf( "%02x ", bufferout[i] );
	}
	printf( "\n" );
}

