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
OBJS=$(DIR_SELF)/fd.o
TARGET_NAME=fd
TARGET=$(TARGET_NAME)
TARGET_TYPE=test
LIB_DEPEND=c appkey
CFLAGS+=-D__TEST__ -D__USE_INLINE_TIMER -D__USE_INLINE_COROUTINE -D__USE_INLINE_FD

#
#don't change it
#
include $(DIR_ROOT)/mk/common.mk
