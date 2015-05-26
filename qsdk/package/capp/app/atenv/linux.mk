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
OBJS=$(DIR_SELF)/atenv.o
TARGET_NAME=atenv
TARGET=$(TARGET_NAME)
TARGET_TYPE=exe
LIB_DEPEND=c appkey
CFLAGS+=-DATENV_DEV=\"./devenv\"

#
#don't change it
#
include $(DIR_ROOT)/mk/common.mk
