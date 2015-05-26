#
#don't change it
#
DIR_SELF=$(shell pwd)

#
#changed me, append obj
#
OBJS=kmnl.o
TARGET_NAME=kmnl
TARGET=$(TARGET_NAME).ko
TARGET_TYPE=mod
MOD_DEPEND=

include $(DIR_ROOT)/mk/common.mk