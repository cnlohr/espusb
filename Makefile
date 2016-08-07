TARGET_OUT:=image.elf
FW_FILES:=image.elf-0x00000.bin image.elf-0x40000.bin
all : $(TARGET_OUT) $(FW_FILES)


SRCS:=driver/uart.c \
	common/mystuff.c \
	common/flash_rewriter.c \
	common/http.c \
	common/commonservices.c \
	common/http_custom.c \
	common/mdns.c \
	common/mfs.c \
	user/custom_commands.c \
	user/usb_asm_1bit.S \
	user/usb.c \
	user/user_main.c 

GCC_FOLDER:=~/esp8266/esp-open-sdk/xtensa-lx106-elf
ESPTOOL_PY:=~/esp8266/esptool/esptool.py
SDK:=/home/cnlohr/esp8266/esp_iot_sdk_v1.5.1
PORT:=/dev/ttyUSB0
#PORT:=/dev/ttyACM0

XTLIB:=$(SDK)/lib
XTGCCLIB:=$(GCC_FOLDER)/lib/gcc/xtensa-lx106-elf/4.8.2/libgcc.a
FOLDERPREFIX:=$(GCC_FOLDER)/bin
PREFIX:=$(FOLDERPREFIX)/xtensa-lx106-elf-
CC:=$(PREFIX)gcc

CFLAGS:=-mlongcalls -I$(SDK)/include -Imyclib  -I. -Iinclude -Iuser -Os -I$(SDK)/include/ -Icommon -DICACHE_FLASH

#	   \
#

LDFLAGS_CORE:=\
	-nostdlib \
	-Wl,--relax -Wl,--gc-sections \
	-L$(XTLIB) \
	-L$(XTGCCLIB) \
	$(SDK)/lib/liblwip.a \
	$(SDK)/lib/libssl.a \
	$(SDK)/lib/libupgrade.a \
	$(SDK)/lib/libnet80211.a \
	$(SDK)/lib/liblwip.a \
	$(SDK)/lib/libwpa.a \
	$(SDK)/lib/libnet80211.a \
	$(SDK)/lib/libphy.a \
	$(SDK)/lib/libcrypto.a \
	$(SDK)/lib/libmain.a \
	$(SDK)/lib/libpp.a \
	$(XTGCCLIB) \
	-T $(SDK)/ld/eagle.app.v6.ld

LINKFLAGS:= \
	$(LDFLAGS_CORE) \
	-B$(XTLIB)

#image.elf : $(OBJS)
#	$(PREFIX)ld $^ $(LDFLAGS) -o $@

$(TARGET_OUT) : $(SRCS)
	$(PREFIX)gcc $(CFLAGS) $^  -flto $(LINKFLAGS) -o $@
	nm -S -n $(TARGET_OUT) > image.map
	$(PREFIX)objdump -S $@ > image.lst
	$(PREFIX)size $@



$(FW_FILES): $(TARGET_OUT)
	@echo "FW $@"
	PATH=$(FOLDERPREFIX):$$PATH;$(ESPTOOL_PY) elf2image $(TARGET_OUT)

burn : $(FW_FILES)
	expr 0`cat count.txt` + 1 > count.txt
	$(ESPTOOL_PY) -b 460800 --port $(PORT) write_flash -fm dout 0x00000 image.elf-0x00000.bin 0x40000 image.elf-0x40000.bin

#If you have space, MFS should live at 0x100000, if you don't it can also live at
#0x10000.  But, then it is limited to 180kB.  You might need to do this if you have a 512kB 
#ESP variant.

burnweb : web/page.mpfs
	($(ESPTOOL_PY) --port $(PORT) write_flash 0x10000 web/page.mpfs)||(true)


IP?=192.168.4.1

netburn : image.elf $(FW_FILES)
	web/execute_reflash $(IP) image.elf-0x00000.bin image.elf-0x40000.bin

usbburn : image.elf $(FW_FILES)
	web/execute_reflash USB image.elf-0x00000.bin image.elf-0x40000.bin


clean :
	rm -rf user/*.o driver/*.o $(TARGET_OUT) $(FW_FILE_1) $(FW_FILE_2)


