#
#sub_system target:all/dep/pre/test/check/clean/install, append new!!!
#
sub_system_all=$(foreach i,$(SUB_SYSTEM),$(i)_all)
sub_system_pre=$(foreach i,$(SUB_SYSTEM),$(i)_pre)
sub_system_test=$(foreach i,$(SUB_SYSTEM),$(i)_test)
sub_system_check=$(foreach i,$(SUB_SYSTEM),$(i)_check)
sub_system_clean=$(foreach i,$(SUB_SYSTEM),$(i)_clean)
sub_system_install=$(foreach i,$(SUB_SYSTEM),$(i)_install)

#
#dump all/dep/pre/test/check/clean/install, append new!!!
#
ifeq (0,$(MAKE_DEBUG))
  define dump_template_vars
  endef
else
  define dump_template_vars
	@echo "===================================================================="
	@echo "DIR_ROOT=$(DIR_ROOT)"
	@echo "SUB_SYSTEM=$(SUB_SYSTEM)"
	@echo "sub_system_all=$(sub_system_all)"
	@echo "sub_system_pre=$(sub_system_pre)"
	@echo "sub_system_test=$(sub_system_test)"
	@echo "sub_system_check=$(sub_system_check)"
	@echo "sub_system_clean=$(sub_system_clean)"
	@echo "sub_system_install=$(sub_system_install)"
	@echo "===================================================================="
  endef
endif

#
#don't change it
#
define SUB_TARGET_TEMPLATE
.PHONY:$(1)_$(2)
$(1)_$(2):
	$(dump_template_vars)
	$$(MAKE) $(2) -C $(1)
endef

#
#all/dep/pre/test/check/clean/install, append new!!!
#
.PHONY:all
all:$(sub_system_all)

.PHONY:pre
pre:$(sub_system_pre)

.PHONY:test
test:$(sub_system_test)

.PHONY:check
check:$(sub_system_check)

.PHONY:clean
clean:$(sub_system_clean)

.PHONY:install
install:$(sub_system_install)

$(foreach i,$(SUB_SYSTEM),$(eval $(call SUB_TARGET_TEMPLATE,$(i),all)))
$(foreach i,$(SUB_SYSTEM),$(eval $(call SUB_TARGET_TEMPLATE,$(i),pre)))
$(foreach i,$(SUB_SYSTEM),$(eval $(call SUB_TARGET_TEMPLATE,$(i),test)))
$(foreach i,$(SUB_SYSTEM),$(eval $(call SUB_TARGET_TEMPLATE,$(i),check)))
$(foreach i,$(SUB_SYSTEM),$(eval $(call SUB_TARGET_TEMPLATE,$(i),clean)))
$(foreach i,$(SUB_SYSTEM),$(eval $(call SUB_TARGET_TEMPLATE,$(i),install)))
