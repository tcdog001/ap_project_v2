include $(TOPDIR)/rules.mk

PKG_NAME:=onig
PKG_VERSION:=5.9.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.geocities.jp/kosako3/oniguruma/archive/

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk
####################################################################
define Package/libonig
  CATEGORY:=thirdpart-lib
  TITLE:=libonig
  DEPENDS:=
endef

define Build/Compile
	$(call Build/Compile/Default,includedir=.)
endef

define Package/libonig/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/.libs/libonig.so.2.0.0 $(1)/usr/lib/libonig.so.2
endef
####################################################################
$(eval $(call BuildPackage,libonig))

