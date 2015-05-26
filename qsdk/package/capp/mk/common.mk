#
#don't change this file!!!
#
SRCS=$(OBJS:.o=.c)
DEPS=$(OBJS:.o=.d)
PRES=$(OBJS:.o=.i)

ifeq (0,$(MAKE_DEBUG))
  define dump_common_vars
  endef
else
  define dump_common_vars
	@echo "===================================================================="
	@echo "OBJS=$(OBJS)"
	@echo "SRCS=$(SRCS)"
	@echo "DEPS=$(DEPS)"
	@echo "CFLAGS=${CFLAGS}"
	@echo "LDFLAGS=${LDFLAGS}"
	@echo "LIBS=$(LIBS)"
	@echo "INCS=$(INCS)"
	@echo "TARGET=$(TARGET)"
	@echo "===================================================================="
  endef
endif

# =========  function clean ============
define do_clean_target
	-$(RM) $(TARGET)
endef

define do_clean_obj
	-$(RM) $(OBJS)
	-$(RM) $(TARGET_NAME).map
	-$(RM) *.maps
	-$(RM) *.trace
	-$(RM) *.locate
	-$(RM) *.sym
	-$(RM) *.symvers
	-$(RM) *.cmd
	-$(RM) *.order
endef

define do_clean_dep
	-$(RM) $(DEPS)
endef

define do_clean_pre
	-$(RM) $(PRES)
endef

ifeq (lib,$(TARGET_TYPE))
  define do_clean_install
	-$(RM) $(LIBS)/$(SONAME)*
  endef
else
  define do_clean_install
  endef
endif

ifdef DO_CLEAN
  define do_clean
    $(DO_CLEAN)
  endef
else
  define do_clean
	$(do_clean_install)
	$(do_clean_target)
	$(do_clean_dep)
	$(do_clean_pre)
	$(do_clean_obj)
  endef
endif
# =========  function clean ============



# =========  function install ============

define do_install_lib
	$(CP) -f $(TARGET) $(LIBS)
	( \
		cd $(LIBS); \
		$(LN) -sf $(TARGET) $(SONAME_MASTER); \
		$(LN) -sf $(SONAME_MASTER) $(SONAME); \
	)
endef

define do_install_exe
endef

define do_install_mod
	$(CP) -f $(TARGET) $(MODS)
endef

ifdef DO_INSTALL
  define do_install
    $(DO_INSTALL)
  endef
else
  ifeq (lib,$(TARGET_TYPE))
    define do_install
	$(do_install_lib)
    endef
  endif
  
  ifeq (exe,$(TARGET_TYPE))
    define do_install
	$(do_install_exe)
    endef
  endif
  
  ifeq (mod,$(TARGET_TYPE))
    define do_install
	$(do_install_mod)
    endef
  endif
endif
# =========  function install ============


# =========  function check ============
ifdef DO_CHECK
  define do_check
    $(DO_CHECK)
  endef
else
  ifeq (lib,$(TARGET_TYPE))
    define do_check
#	now, do nothing
    endef
  endif
  
  ifeq (exe,$(TARGET_TYPE))
    define do_check
#	now, do nothing
    endef
  endif
endif
# =========  function check ============

lib_depend=$(foreach i,$(LIB_DEPEND),-l$(i))
lib_depend+=-L$(LIBS)


# =========  LDFLAGS ============
EXE_LDFLAGS=-lrt -Wl,-Map=$(TARGET_NAME).map -Wl,-rpath,$(LIBS) -Wl,--export-dynamic
TEST_LDFLAGS=$(EXE_LDFLAGS)
LIB_LDFLAGS=-shared -Wl,-Map=lib$(TARGET_NAME).map -Wl,-soname,$(SONAME)

AOBJS=$(OBJS) $(TOBJS)

__CFLAGS+=	-DAPPKEY_PATH=\"$(APPKEY_PATH)\"
EXE_CFLAGS=	-DAPP_TYPE_EXE
TEST_CFLAGS=	-DAPP_TYPE_EXE
LIB_CFLAGS=	-fPIC -ljson-c \
		-DAPP_TYPE_LIB

ifeq (exe,$(TARGET_TYPE))
	FILENO_NAME=$(TARGET_NAME)
	__CFLAGS+=$(EXE_CFLAGS)
	__LDFLAGS+=$(EXE_LDFLAGS)
	include $(DIR_ROOT)/mk/target.mk
else ifeq (test,$(TARGET_TYPE))
	FILENO_NAME=test$(TARGET_NAME)
	__CFLAGS+=$(TEST_CFLAGS)
	__LDFLAGS+=$(TEST_LDFLAGS)
	include $(DIR_ROOT)/mk/target.mk
else ifeq (lib,$(TARGET_TYPE))
	FILENO_NAME=lib$(TARGET_NAME)
	__CFLAGS+=$(LIB_CFLAGS)
	__LDFLAGS+=$(LIB_LDFLAGS)
	TFLAGS=-l$(TARGET_NAME)
	include $(DIR_ROOT)/mk/target.mk
else ifeq (mod,$(TARGET_TYPE))
  include $(DIR_ROOT)/mk/mod.mk
endif

# =========  LDFLAGS ============