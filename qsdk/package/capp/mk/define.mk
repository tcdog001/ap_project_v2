ifndef MK_DEFINE
	export MK_DEFINE=1
	
	export MAKE=make
	export CP=cp
	export RM=rm -f
	export LN=ln
	export ARCH_PATH=$(DIR_ROOT)/arch/$(ARCH)
	export LIBS=$(ARCH_PATH)/lib
	export MODS=$(ARCH_PATH)/mod
	export MK_PATH=$(DIR_ROOT)/mk
	export SCRIPT_PATH=$(DIR_ROOT)/script
	
	export APPKEY_PATH=/tmp/$(shell whoami)/appkey
	export OS_TYPE=linux
	
	#export INCS=-I$(DIR_ROOT)/lib -I/usr/src/linux-headers-$(shell uname -r)/include
	export INCS+=-I$(DIR_ROOT)/lib -I$(DIR_ROOT)/lib/json-c
	export CFLAGS+=-D__PC__ -std=gnu99 -DLINUX -DDEBUG -D__TAB_AS_SPACE=4 -Wno-unused -fmerge-all-constants
	#export __LDFLAGS+=-L/lib -L/usr/lib -L/usr/local/lib
	
	export EXTRA_CFLAGS+=
	export CC=gcc -Wall -rdynamic
	export MAKE_DEBUG=0
	export LIB_DEPEND= #
	export ARCH=x86
	export PROJECT_NAME=autelan
	export FILECODE=filecode
endif