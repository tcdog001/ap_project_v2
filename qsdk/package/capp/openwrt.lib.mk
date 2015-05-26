include $(TOPDIR)/rules.mk

PKG_NAME:=mylib
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
PKG_LIB_BUILD_DIR := $(PKG_BUILD_DIR)/lib
PKG_APP_BUILD_DIR := $(PKG_BUILD_DIR)/app

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += -Wall \
		-fexceptions -fno-omit-frame-pointer \
		-I$(STAGING_DIR)/usr/include \
		-I$(BUILD_DIR)/mylib/lib \
		-D__OPENWRT__ \
		-D__TAB_AS_SPACE=4 \
		-Wno-unused \
		-fmerge-all-constants \
		-std=gnu99 \
		-fPIC \
		-shared \
		#end

TARGET_LDFLAGS+= -L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib

APPKEY_PATH=etc/appkey

define Package/mylib/install/common
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/$(APPKEY_PATH)/
endef
####################################################################
define Package/libappkey
  SECTION:=libs
  CATEGORY:=mylib
  TITLE:=Autelan Basic library
  DEPENDS:=+libubacktrace
endef

define Package/libappkey/install
	$(Package/mylib/install/common)
	
	$(INSTALL_DATA) $(PKG_LIB_BUILD_DIR)/appkey/libappkey.so $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_LIB_BUILD_DIR)/appkey/libappkey.key $(1)/$(APPKEY_PATH)/
endef

define Package/libappkey/compile
	$(MAKE) -C $(PKG_LIB_BUILD_DIR)/appkey \
		CC="$(TARGET_CC)" \
		CFLAGS=" \
			$(TARGET_CFLAGS) \
			$(TARGET_CPPFLAGS) \
			" \
		LDFLAGS="$(TARGET_LDFLAGS)"
	$(CP) $(PKG_LIB_BUILD_DIR)/appkey/libappkey.so $(STAGING_DIR)/lib
endef
####################################################################
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./* $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
	$(Package/libappkey/compile)
endef
####################################################################
$(eval $(call BuildPackage,libappkey))
