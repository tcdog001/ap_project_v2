include $(TOPDIR)/rules.mk

PKG_NAME:=jq
PKG_VERSION:=2014-08-14
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/stedolan/jq
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
#PKG_SOURCE_VERSION:=287e2c96ed795ebd57875a855d2ed5894311ecbb
PKG_SOURCE_VERSION:=0f7759f83106098312224e9cabc5464b8e19a3d0
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_FIXUP:=autoreconf

#when use git, not define PKG_BUILD_DIR
#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)



include $(INCLUDE_DIR)/package.mk
####################################################################
define Package/jq
  CATEGORY:=thirdpart-app
  TITLE:=jq
  DEPENDS:=+libm +libonig
endef

ONIG_PATH=$(BUILD_DIR)/onig-5.9.5

TARGET_CFLAGS += -std=gnu99 -I$(ONIG_PATH)
#TARGET_LDFLAGS += -L$(ONIG_PATH)/.libs -Wl,-rpath $(ONIG_PATH)/.libs
TARGET_LDFLAGS += -L$(ONIG_PATH)/.libs

define Build/Configure
	cp -f files/configure.ac $(PKG_BUILD_DIR)/
	$(call Build/Configure/Default,--disable-docs --disable-static --enable-silent-rules)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)"
endef

define Package/jq/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/jq $(1)/usr/bin/
	
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/.libs/libjq.so.1.0.4 $(1)/usr/lib/libjq.so.1
endef
####################################################################
$(eval $(call BuildPackage,jq))

