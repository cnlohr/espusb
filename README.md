#ESP8266 USB Software Driver

WARNING: THIS PROJECT IS A WORK IN PROGRESS.  DO NOT EXPECT IT TO WORK WELL.

Forum for discussion of development: http://www.esp8266.com/espusb

I wanted to have USB on the ESP8266, and a while ago I saw on the ESP32 flier, it read: "Rich Peripherals" ... "Sorry, no USB!" I thought to myself, that is ridiculous.  Of course there's USB.

So, it was born.  This is a software USB stack running on the ESP8266/ESP8285.  It requires only one external resistor, between D- and 3.3V.

Currently, the chip expects D- on GPIO 4 and D+ on GPIO 5 - but any GPIO pair may be used.  It is important to note that both D- and D+ MUST be adjacent.

Additionally, this only operates with low-speed USB (1.5 MBit/s), ideal for making peripherals, not for fake networkand devices usb-serial bridges.  That said - you can still write "control" messages that communicate with the ESP8266.  Control messages are a great way to encapsulate your data since they handle all the framing and describing what function you wish to pass data for.

More info hopefully coming soon.

## Memory Usage

This is typical memory usage for a two-endpoint (in addition to EP0) device.

In summary:

Total SRAM: 232 bytes + Descriptors (~317 bytes)

Total Flash/IRAM (Only iram, tables and usb_init can be stuck in slow flash): 1422 bytes



More details:

SRAM:

0000007c D usb_ramtable
0000006c B usb_internal_state

IRAM:

You must write: usb_handle_custom_control - the demos here hook it up to the command engine which allows self flashing on 1MB+ parts.

C functions:
0000002a T usb_pid_handle_ack
0000014f T usb_pid_handle_data
00000022 T usb_pid_handle_out
000000e9 T usb_pid_handle_in
00000002 T usb_pid_handle_sof
00000039 T usb_pid_handle_setup

From/To (in ASM):
40100f20 t usb_asm_start
401011ef t usb_asm_end
Total length of ASM: 2cf

