# espusb bootloader

THIS IS A WORK IN PROGRESS.  It is NOT YET A BOOTLOADER, just a really small USB thinger.

But, it's totes a SDK-less implementation that works with USB!

Ok, it's a little bigger than I was initially hoping, at 2.5kB, but, it seems to work pretty solid. It's much better than when you have the wifis and everything else going.

Right now, it just accepts control messages of length of ~2090 bytes (2048 plus a little bit) back and forth. Check out the "main" function for more info about how it works. You also get to keep the bRequest, wIndex, and other details so you don't need to weigh down the code with a protocol ontop of the control requests. I've tried sending/receiving as quickly as I could and I seem to get around 30kB/sec down at the same time as 30kB/sec up, or 60kB/sec in one direction.

Detection that there is a computer seems to happen in .1 to .3 seconds, so that's good.

I am still trying to figure out a good way to merge the interfaces... Take a look at main and see if you want something a little different?

