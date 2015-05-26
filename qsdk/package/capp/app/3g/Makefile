################################################################################
#LIB = -Wl,--start-group -lpthread -lm   -Wl,--end-group
################################################################################
ifeq ($(CFG_HI_EXPORT_FLAG),)
#SDK_DIR       := $(shell pwd)/../../../../../../../../..
SDK_DIR       := $(shell pwd)/../../../histb
LIB_DIR       := $(SDK_DIR)/pub/lib
SHARED_LIB_DIR:= $(LIB_DIR)/share
STATIC_LIB_DIR:= $(LIB_DIR)/static
INCLUDE_DIR   := $(SDK_DIR)/pub/include
ROOTFS_DIR    := $(SDK_DIR)/pub/rootfs
include $(SDK_DIR)/cfg.mak
include $(SDK_DIR)/base.mak
endif

#add  by autelan
TARGET=at_ttyUSB0 at_ttyUSB1 at_ttyUSB3
CROSS_COMPILE=arm-hisiv200-linux-
CC:=$(CROSS_COMPILE)gcc
RM:=rm -rf

ifdef LTEFI
	AUTE_CFLAGS+=-DLTEFI
endif
ifdef LTEFI_V2
        AUTE_CFLAGS+=-DLTEFI_V2
endif
#add  end by autelan

image=
ifeq ($(CFG_HI_3G_SUPPORT),y)
image=ppp_dial
endif

################################################################################

all:$(image) install

ppp_dial: ppp_dial.c
	@$(CC) -static -o $@ $^ -I$(INCLUDE_DIR) -L$(STATIC_LIB_DIR) -lhi_3g -lhi_common $(LIB) -lpthread
	@$(STRIP) $@

#add by autelan
	$(CC) $(AUTE_CFLAGS) -o at_ttyUSB0 at_ttyUSB0.c -lpthread
	$(CC) $(AUTE_CFLAGS) -o at_ttyUSB1 at_ttyUSB1.c -lpthread
	$(CC) $(AUTE_CFLAGS) -o at_ttyUSB3 at_ttyUSB3.c -lpthread
#add end by autelan

clean:
	-@rm -f $(image)
#add by autelan
	$(RM) $(TARGET) *.o *.ko *.mod.c .*.cmd *.order *.symvers .tmp_versions $(TARGET)
#add end by autelan

install:
	cp ppp_dial $(ROOTBOX_DIR)/usr/sbin
#add by autelan
	cp at_ttyUSB0 $(ROOTBOX_DIR)/usr/sbin
	cp at_ttyUSB1 $(ROOTBOX_DIR)/usr/sbin
	cp at_ttyUSB3 $(ROOTBOX_DIR)/usr/sbin
#add end by autelan

uninstall:

