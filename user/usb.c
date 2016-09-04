#define NEED_CONSTANTS

#include "usb.h"
#include <string.h>
#include <esp82xxutil.h>
#include <eagle_soc.h>
#include <gpio.h>
#include "usb_table_1bit.h"

struct usb_internal_state_struct usb_internal_state __attribute__((aligned(4)));

#define ENDPOINT0_SIZE 8 //Fixed for USB 1.1, Low Speed.

#define INSTANCE_DESCRIPTORS
#include "usb_config.h"

//Received a setup for a specific endpoint.
void usb_pid_handle_setup( uint32_t this_token, struct usb_internal_state_struct * ist )
{
	uint8_t addr = (this_token>>8) & 0x7f;
	uint8_t endp = (this_token>>15) & 0xf;

	if( endp >= ENDPOINTS ) goto end;
	if( addr != 0 && addr != ist->my_address ) goto end;

	struct usb_endpoint * e = ist->ce = &ist->eps[endp];
	e->toggle_out = 0;
	e->toggle_in = 1;
	e->ptr_in = 0;
	e->send = 0;
	ist->setup_request = 1;
end:
	__asm__ __volatile__( "movi a0, usb_reinstate" ); //After this token, we are immediately expecting another grouping.  This short-circuits the 'return'.
}

void usb_pid_handle_sof( uint32_t this_token, struct usb_internal_state_struct * ist )
{
}

void usb_pid_handle_in( uint32_t this_token, struct usb_internal_state_struct * ist )
{
	uint8_t addr = (this_token>>8) & 0x7f;
	uint8_t endp = (this_token>>15) & 0xf;

	//If we get an "in" token, we have to strike any accept buffers.

	if( endp >= ENDPOINTS ) return;
	if( addr != 0 && addr != ist->my_address ) return;

	struct usb_endpoint * e = ist->ce = &ist->eps[endp];

	e->got_size_out = 0;  //Cancel any out transaction


	int tosend = 0;
	uint8_t sendnow[12];
	sendnow[0] = 0x80;

	if( e->send && e->ptr_in ) 
	{
		tosend = e->size_in - e->place_in;

		if( tosend > 8 ) tosend = 8;
	}

	if( e->toggle_in )
	{
		sendnow[1] = 0b01001011; //DATA1
	}
	else
	{
		sendnow[1] = 0b11000011; //DATA0
	}

	if( tosend == 0 || !e->send || !e->ptr_in || e->ptr_in == EMPTY_SEND_BUFFER )  //Tricky: Empty packet.
	{

		//Tricky: Control messages are not allowed to send NAKs.  We /have/ to send an empty packet for them if no more data is available.
		//With endpoints, proper, it's okay to send NAKs.

		if( endp == 0 )
		{
			sendnow[2] = 0;
			sendnow[3] = 0; //CRC = 0
			usb_send_data( sendnow, 4, 2 );  //Force a CRC
			e->ptr_in = 0;
		}
		else
		{
			sendnow[1] = 0x5a; //Empty data (NAK)
			usb_send_data( sendnow, 2, 2 );
		}
	}
	else
	{
		ets_memcpy( sendnow+2, e->ptr_in + e->place_in, tosend );
		usb_send_data( sendnow, tosend+2, 0 );
		e->advance_in = tosend;
	}
	return;
}

void usb_pid_handle_out( uint32_t this_token, struct usb_internal_state_struct * ist )
{
	//We need to handle this here because we could have an interrupt in the middle of a control or bulk transfer.
	//This will correctly swap back the endpoint.
	uint8_t addr = (this_token>>8) & 0x7f;
	uint8_t endp = (this_token>>15) & 0xf;
	if( endp >= ENDPOINTS ) return;
	if( addr != 0 && addr != ist->my_address ) return;
	struct usb_endpoint * e = ist->ce = &ist->eps[endp];

	__asm__ __volatile__( "movi a0, usb_reinstate" );  //After this token, we are immediately expecting another grouping.  This short-circuits the 'return'.
}

void usb_pid_handle_data( uint32_t this_token, struct usb_internal_state_struct * ist, uint32_t which_data )
{
	//Received data from host.

	struct usb_endpoint * e = ist->ce;

	if( e == 0 ) return;

	if( e->toggle_out != which_data )
	{
		goto just_ack;
	}

	e->toggle_out = !e->toggle_out;

	if( ist->setup_request )
	{
		ist->setup_request = 0;
		struct usb_urb * s = (struct usb_urb *)ist->usb_buffer;

		//Send just a data packet.
		e->ptr_in = EMPTY_SEND_BUFFER;
		e->place_in = 0;
		e->size_in = 0;
		e->send = 1;

		if( s->bmRequestType & 0x80 )
		{
			if( s->bRequest == 0x06 ) //Get Request
			{
				int i;
				const struct descriptor_list_struct * dl;
				for( i = 0; i < DESCRIPTOR_LIST_ENTRIES; i++ )
				{
					dl = &descriptor_list[i];
					if( dl->wIndex == s->wIndex && dl->wValue == s->wValue )
						break;
				}

				if( i == DESCRIPTOR_LIST_ENTRIES )
				{
					//??? Somehow fail?  Is 'nak' the right thing? I don't know what to do here.
					goto just_ack;
				}

				e->ptr_in = dl->addr;
				e->size_in = dl->length;
				if( s->wLength < e->size_in ) e->size_in = s->wLength;
			}
			usb_handle_custom_control( s->bmRequestType, s->bRequest, s->wLength, ist );
		}
		else if( s->bmRequestType == 0x00 )
		{
			if( s->bRequest == 0x05 ) //Set address.
			{
				ist->my_address = s->wValue;
			}
			if( s->bRequest == 0x09 ) //Set configuration.
			{
				//s->wValue; has the index.  We don't really care about this.
			}
			usb_handle_custom_control( s->bmRequestType, s->bRequest, s->wLength, ist );
		}
	}
	else if( e->ptr_out )
	{
		//Read into that buffer.
		int acc = ist->packet_size-3;  //packet_size includes CRC and PID, need just data size.
		int place = e->got_size_out;

		if( place + acc > e->max_size_out )
		{
			acc = e->max_size_out - place;
		}

		ets_memcpy( e->ptr_out + e->got_size_out, ist->usb_buffer+1, acc );  //First byte of USB buffer is token.
		e->got_size_out += acc;
		if( e->got_size_out == e->max_size_out && e->transfer_done_ptr ) {
			e->ptr_out = 0;
			*e->transfer_done_ptr = e->got_size_out;
			e->transfer_done_ptr = 0;
		}
	}


just_ack:
	{
		//Got the right data.  Acknowledge.
		uint8_t sendword[2] = { 0x80, 0xD2 };
		usb_send_data( sendword, 2, 2 );
	}
}

void usb_pid_handle_ack( uint32_t this_token, struct usb_internal_state_struct * ist )
{
	struct usb_endpoint * e = ist->ce;
	if( !e ) return;



	e->toggle_in = !e->toggle_in;
	e->place_in += e->advance_in;
	e->advance_in = 0;
	if( e->place_in == e->size_in )
	{
		e->send = 0;
	}
}


void  ICACHE_FLASH_ATTR usb_init()
{
    ETS_GPIO_INTR_DISABLE();                                           //Close the GPIO interrupt

	PIN_FUNC_SELECT(PERIPHSDPLUS,FUNCDPLUS); //D- (needs pullup)
	PIN_FUNC_SELECT(PERIPHSDMINUS,FUNCDMINUS); //D+

    PIN_DIR_INPUT = _BV(DMINUS)|_BV(DPLUS);
	PIN_PULLUP_EN( PERIPHSDMINUS );
	PIN_PULLUP_DIS( PERIPHSDPLUS );

    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(0));
    ETS_GPIO_INTR_ATTACH(gpio_intr,NULL);                                //Attach the gpio interrupt.
    gpio_pin_intr_state_set(GPIO_ID_PIN(DPLUS),GPIO_PIN_INTR_POSEDGE);  //Rising Edge Trigger.

	//Forcibly disconnect from bus.
	volatile uint32_t * gp = (volatile uint32_t*)GPIO_BASE_REG;
	gp[GPIO_OFFSET_CLEAR/4] = _BV(DPLUS) | _BV(DMINUS);
	gp[GPIO_OFFSET_DIR_OUT/4] = _BV(DPLUS) | _BV(DMINUS);
	ets_delay_us( 10000 );
	gp[GPIO_OFFSET_DIR_IN/4] = _BV(DPLUS) | _BV(DMINUS);

    ETS_GPIO_INTR_ENABLE();

}

