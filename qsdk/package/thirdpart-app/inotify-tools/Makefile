include $(TOPDIR)/rules.mk

PKG_NAME:=inotify-tools
PKG_VERSION:=2014-08-11
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/rvoicilas/inotify-tools
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=06007d350faa8731c67e186923ab417486104719
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_FIXUP:=autoreconf

#when use git, not define PKG_BUILD_DIR
#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
####################################################################
define Package/inotify-tools
  CATEGORY:=thirdpart-app
  TITLE:=inotify-tools
  DEPENDS:=
endef

define Build/Configure
#	pushd $(PKG_BUILD_DIR) && ./autogen.sh && popd
	$(call Build/Configure/Default,)
endef

define Package/inotify-tools/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/inotifywait $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/inotifywatch $(1)/usr/sbin/
	
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libinotifytools/src/.libs/libinotifytools.so.0.4.1 $(1)/usr/lib/libinotifytools.so
endef
####################################################################
$(eval $(call BuildPackage,inotify-tools))

