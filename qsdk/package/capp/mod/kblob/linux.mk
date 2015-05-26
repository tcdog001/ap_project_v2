#
#don't change it
#
DIR_SELF=$(shell pwd)

#
#changed me, append obj
#
OBJS=kblob.o
TARGET_NAME=kblob
TARGET=$(TARGET_NAME).ko
TARGET_TYPE=mod
MOD_DEPEND=kslice

include $(DIR_ROOT)/mk/common.mk