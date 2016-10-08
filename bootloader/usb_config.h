#ifndef _USB_CONFIG_H
#define _USB_CONFIG_H


//Defines the number of endpoints for this device. (Always add one for EP0)
#define ENDPOINTS 1

//Defines a pin that is useful for debugging USB on a logic analyzer
//#define DEBUGPIN 2

//DPLUS and DMINUS are not actually used except for setting things up.
#define DPLUS 5
#define DMINUS 4

//This is what's used in the assembly
#define DMINUSBASE DMINUS  //Must be D- first, then D+ second.

#define PERIPHSDPLUS    PERIPHS_IO_MUX_GPIO5_U
#define PERIPHSDMINUS   PERIPHS_IO_MUX_GPIO4_U
#define FUNCDPLUS  FUNC_GPIO5
#define FUNCDMINUS FUNC_GPIO4

#endif


#ifdef INSTANCE_DESCRIPTORS
//Taken from http://www.usbmadesimple.co.uk/ums_ms_desc_dev.htm
static const uint8_t device_descriptor[] = {
	18, //Length
	1,  //Type (Device)
	0x10, 0x01, //Spec
	0xff, //Device Class
	0xff, //Device Subclass
	0xff, //Device Protocol  (000 = use config descriptor)
	0x08, //Max packet size for EP0 (This has to be 8 because of the USB Low-Speed Standard)
	0xcd, 0xab, //ID Vendor   //TODO: register this in http://pid.codes/howto/ or somewhere.
	0x66, 0x82, //ID Product
	0x02, 0x00, //ID Rev
	1, //Manufacturer string
	2, //Product string
	0, //Serial string
	1, //Max number of configurations
};

static const uint8_t config_descriptor[] = { 
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9, 					// bLength;
	2,					// bDescriptorType;
	0x09, 0x00,			// wTotalLength  	

	0x00,					// bNumInterfaces (Normally 1)
	0x01,					// bConfigurationValue
	0x00,					// iConfiguration
	0xC0,					// bmAttributes (was 0xa0) (add self-powered)
	0x64,					// bMaxPower (200mA)

};


//Ever wonder how you have more than 6 keys down at the same time on a USB keyboard?  It's easy. Enumerate two keyboards!


#define STR_MANUFACTURER L"CNLohr"
#define STR_PRODUCT      L"BOOTLOADER"
#define STR_SERIAL       L"000"

struct usb_string_descriptor_struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wString[];
};
const static struct usb_string_descriptor_struct string0 = {
	4,
	3,
	{0x0409}
};
const static struct usb_string_descriptor_struct string1 = {
	sizeof(STR_MANUFACTURER),
	3,
	STR_MANUFACTURER
};
const static struct usb_string_descriptor_struct string2 = {
	sizeof(STR_PRODUCT),
	3,
	STR_PRODUCT
};
const static struct usb_string_descriptor_struct string3 = {
	sizeof(STR_SERIAL),
	3,
	STR_SERIAL
};


// This table defines which descriptor data is sent for each specific
// request from the host (in wValue and wIndex).
const static struct descriptor_list_struct {
	uint16_t	wValue;
	uint16_t	wIndex;
	const uint8_t	*addr;
	uint8_t		length;
} descriptor_list[] = {
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0200, 0x0000, config_descriptor, sizeof(config_descriptor)},
	{0x0300, 0x0000, (const uint8_t *)&string0, 4},
	{0x0301, 0x0409, (const uint8_t *)&string1, sizeof(STR_MANUFACTURER)},
	{0x0302, 0x0409, (const uint8_t *)&string2, sizeof(STR_PRODUCT)},	
	{0x0303, 0x0409, (const uint8_t *)&string3, sizeof(STR_SERIAL)}
};
#define DESCRIPTOR_LIST_ENTRIES ((sizeof(descriptor_list))/(sizeof(struct descriptor_list_struct)) )

#endif


