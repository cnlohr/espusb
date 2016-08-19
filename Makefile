include user.cfg
include esp82xx/common.mf
include esp82xx/main.mf

SRCS += 	user/usb_asm_1bit.S \
		user/usb.c

usbburn : $(FW_FILE1) $(FW_FILE2)
	web/execute_reflash USB $(FW_FILE1) $(FW_FILE2)
