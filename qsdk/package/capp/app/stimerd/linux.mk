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
OBJS=$(DIR_SELF)/stimerd.o
TARGET_NAME=stimerd
TARGET=$(TARGET_NAME)
TARGET_TYPE=exe
LIB_DEPEND=c appkey
CFLAGS+=-D__USE_INLINE_TIMER -DSTIMER_TICKS=1000 -DSTIMER_TIMEOUT=3000

#
#don't change it
#
include $(DIR_ROOT)/mk/common.mk
