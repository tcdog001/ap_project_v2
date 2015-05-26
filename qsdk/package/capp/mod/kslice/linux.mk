#
#don't change it
#
DIR_SELF=$(shell pwd)

#
#changed me, append obj
#
OBJS=kslice.o
TARGET_NAME=kslice
TARGET=$(TARGET_NAME).ko
TARGET_TYPE=mod
MOD_DEPEND=

include $(DIR_ROOT)/mk/common.mk