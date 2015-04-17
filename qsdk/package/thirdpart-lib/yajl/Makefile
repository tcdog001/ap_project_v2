include $(TOPDIR)/rules.mk

PKG_NAME:=yajl
PKG_VERSION:=2.1.1
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/lloyd/yajl.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=12ee82ae5138ac86252c41f3ae8f9fd9880e4284
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_FIXUP:=autoreconf

#when use git, not define PKG_BUILD_DIR
#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)



include $(INCLUDE_DIR)/package.mk
####################################################################
define Package/libyajl
  CATEGORY:=thirdpart-lib
  TITLE:=yajl
  DEPENDS:=+libm
endef


define Build/Compile
	cp -f files/* $(PKG_BUILD_DIR)/
	$(call Build/Compile/Default,)
endef

define Package/libyajl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/yajl-$(PKG_VERSION)/bin/* $(1)/usr/bin/
	
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/yajl-$(PKG_VERSION)/lib/libyajl.so.$(PKG_VERSION) $(1)/usr/lib/libyajl.so.2
endef
####################################################################
$(eval $(call BuildPackage,libyajl))

