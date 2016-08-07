#ESP8266 USB Software Driver

WARNING: THIS PROJECT IS A WORK IN PROGRESS.  DO NOT EXPECT IT TO WORK.

TODO: Continue.


I wanted to have USB on the ESP8266, and a while ago I saw on the ESP32 flier, it read: "Rich Peripherals" ... "Sorry, no USB!" I thought to myself, that is ridiculous.  Of course there's USB.

So, it was born.

This is a software USB stack running on the ESP8266/ESP8285.  It requires only one external resistor.

Currently, the chip expects D- on GPIO 12 and D+ on GPIO 13 - but any GPIO pair may be used.  It is important to note that both D- and D+ MUST be adjacent.

Additionally, this only operates with low-speed USB (1.5 MBit/s), ideal for making peripherals, not for fake networkand devices usb-serial bridges.  That said - you can still write "control" messages that communicate with the ESP8266.  Control messages are a great way to encapsulate your data since they handle all the framing and describing what function you wish to pass data for.

I do hope to develop a USB Full speed implementation (12 MBit/s) - however - it will take more RAM than the USB low-speed implementation (~3kB) and some mechanism to do ultra-low latency GPIO change interrupts.  Preferably with the NMI.  Espressif has been very tight lipped about this, and unless they provide the information it is outside of my capability to develop it.

More info hopefully coming soon.

