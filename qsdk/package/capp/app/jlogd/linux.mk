#
#don't change it
#
DIR_SELF=$(shell pwd)

#
#changed DIR_ROOT
#
DIR_ROOT=$(DIR_SELF)/../..

#
#don't change it
#
include $(DIR_ROOT)/mk/define.mk

#
#changed me, append obj
#
OBJS=$(DIR_SELF)/jlogd.o
TARGET_NAME=jlogd
TARGET=$(TARGET_NAME)
TARGET_TYPE=exe
LIB_DEPEND=c appkey
CFLAGS+=

#
#don't change it
#
include $(DIR_ROOT)/mk/common.mk
