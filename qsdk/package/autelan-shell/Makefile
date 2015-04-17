include $(TOPDIR)/rules.mk

PKG_NAME:=autelan-shell
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

####################################################################
define Package/autelan-jsock
  SECTION:=apps
  CATEGORY:=autelan-shell
  TITLE:=Autelan Basic Shell Script
  DEPENDS:=+xinetd +jshn +jq +libonig
#  DEPENDS:=+jshn +jq +libonig
endef


define Package/autelan-jsock/install
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/xinetd.d/
	$(INSTALL_DIR) $(1)/etc/utils/
	$(INSTALL_DIR) $(1)/etc/jsock/
	$(INSTALL_DIR) $(1)/etc/jsock/functions/
	$(INSTALL_DIR) $(1)/etc/jsock/functions/msg/
	$(INSTALL_DIR) $(1)/etc/jsock/functions/sig/

	sed -i 's,env += JSOCK_PEER=1.0.0.5,env += JSOCK_PEER=1.0.0.6,' ./src/jsock/etc/xinetd.d/jsock_tcp
	sed -i 's,env += JSOCK_PEER=1.0.0.5,env += JSOCK_PEER=1.0.0.6,' ./src/jsock/etc/xinetd.d/jsock_udp
	$(CP) ./src/jsock/etc/* $(1)/etc/
endef
####################################################################
define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef
####################################################################
$(eval $(call BuildPackage,autelan-jsock))

