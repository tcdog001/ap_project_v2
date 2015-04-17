include $(TOPDIR)/rules.mk

PKG_NAME:=newlisp
PKG_VERSION:=10.6.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=http://www.newlisp.org/downloads/

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
####################################################################
define Package/newlisp
  CATEGORY:=thirdpart-app
  TITLE:=newlisp
  DEPENDS:=
endef

define Build/Compile
	cp -f files/makefile_linux $(PKG_BUILD_DIR)/
	$(call Build/Compile/Default, -f makefile_linux)
endef

define Package/newlisp/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/newlisp $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/util/newlispdoc $(1)/usr/sbin/
	
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/newlisp
	$(INSTALL_DIR) $(1)/usr/lib/newlisp/modules
	$(INSTALL_DIR) $(1)/usr/lib/newlisp/util
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modules/*.lsp $(1)/usr/lib/newlisp/modules/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/util/syntax.cgi $(1)/usr/lib/newlisp/util/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/util/newlisp.vim $(1)/usr/lib/newlisp/util/
endef
####################################################################
$(eval $(call BuildPackage,newlisp))

