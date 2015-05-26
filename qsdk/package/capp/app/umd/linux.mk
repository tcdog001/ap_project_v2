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
OBJS=$(DIR_SELF)/umd.o \
	$(DIR_SELF)/user.o \
	$(DIR_SELF)/cli.o \
	$(DIR_SELF)/timer.o \
	$(DIR_SELF)/cfg.o \
	$(DIR_SELF)/flow.o \
	# end
TARGET_NAME=umd
TARGET=$(TARGET_NAME)
TARGET_TYPE=exe
LIB_DEPEND=c appkey
CFLAGS+=-D__USE_INLINE_TIMER

#
#don't change it
#
include $(DIR_ROOT)/mk/common.mk
