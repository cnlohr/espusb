include user.cfg
-include esp82xx/common.mf
-include esp82xx/main.mf

CFLAGS += 
SRCS += 	user/usb_asm_1bit.S \
		user/usb.c

usbburn : $(FW_FILE1) $(FW_FILE2)
	web/execute_reflash USB $(FW_FILE1) $(FW_FILE2)

#Useful git commands
ifndef TARGET
# Fetch submodule if the user forgot to clone with `--recursive`
GETSUBMODS = all burn burnweb netweb netburn clean cleanall purge
.PHONY : $(GETSUBMODS)
$(GETSUBMODS) :
	$(warning Submodule esp82xx was not fetched. Trying it now.)
	git submodule update --init --recursive
	$(info Re-unning make...)
	make $@ $(MFLAGS) $(MAKEOVERRIDES)
endif
