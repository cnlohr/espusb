TARGET_OUT = image.elf
FW_1 = image.elf-0x10000.bin
FW_2 = image.elf-0x00000.bin

all : $(TARGET_OUT)

ESPTOOL:=~/esp8266/esptool/esptool.py
ESPTOOLOPTS:=-b 1000000
GCC_FOLDER:=~/esp8266/esp-open-sdk/xtensa-lx106-elf
PREFIX:=$(GCC_FOLDER)/bin/xtensa-lx106-elf-
OBJDUMP:=$(PREFIX)objdump
OBJCOPY:=$(PREFIX)objcopy
GCC:=$(PREFIX)gcc
CFLAGS:=-mlongcalls -Os -Iinclude -nostdlib -DMAIN_MHZ=80 -I.  -I../esp82xx/include -I.. -I../user -DNOSDK 
LDFLAGS:=-T linkerscript.ld -T addresses.ld

FOLDERPREFIX:=$(GCC_FOLDER)/bin
GCC_FOLDER:=~/esp8266/esp-open-sdk/xtensa-lx106-elf
ESPTOOL_PY:=~/esp8266/esptool/esptool.py
PORT:=/dev/ttyUSB0

SRCS:=romlib.c main.c startup.s ../user/usb.c ../user/usb_asm_1bit.S

$(TARGET_OUT) : $(SRCS)
	$(GCC) $(CFLAGS) $^  $(LDFLAGS) -o $@
	nm -S -n $(TARGET_OUT) > image.map
	$(PREFIX)objdump -S $@ > image.lst
	PATH=$(FOLDERPREFIX):$$PATH;$(ESPTOOL_PY) elf2image $(TARGET_OUT) #-ff 80m -fm dio 

burn : $(FW_FILE_1) $(FW_FILE_2)
	($(ESPTOOL_PY) --port $(PORT) write_flash 0x00000 image.elf-0x00000.bin)||(true)


clean :
	rm -rf $(TARGET_OUT) image.map image.lst $(FW_1) $(FW_2)

