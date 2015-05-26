KERNELDIR?=/lib/modules/$(shell uname -r)/build
obj-m:=$(OBJS)
MODULE_NAME=$(TARGET_NAME)
EXTRA_CFLAGS += -I$(DIR_ROOT)/lib \
		-I$(KDIR)/include \
		-D__TAB_AS_SPACE=4 \
		-D__KERNEL__ \
		-DMODULE \
		-D__KERNEL_SYSCALLS__ \
		-DEXPORT_SYMTAB -O2 -Wall  -DMODVERSIONS
all:
	$(foreach i,$(MOD_DEPEND),$(shell $(CP) $(DIR_ROOT)/mod/$(i)/Module.symvers .))
	$(MAKE) -C $(KERNELDIR) M=$(DIR_SELF)
	$(CP) -f $(TARGET) $(MODS)


.PHONY: check
check:
	$(do_check)



.PHONY: clean
clean:
	$(do_clean)



.PHONY: install
install:
	$(do_install)
