include $(TOPDIR)/rules.mk

PKG_NAME:=myapp
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
PKG_LIB_BUILD_DIR := $(PKG_BUILD_DIR)/lib
PKG_APP_BUILD_DIR := $(PKG_BUILD_DIR)/app

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += -Wall \
		-fexceptions -fno-omit-frame-pointer \
		-I$(STAGING_DIR)/usr/include \
		-I$(BUILD_DIR)/myapp/lib \
		-D__OPENWRT__ \
		-D__TAB_AS_SPACE=4 \
		-Wno-unused \
		-fmerge-all-constants \
		-std=gnu99 \
		#end

TARGET_LDFLAGS+= -L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib

APPKEY_PATH=etc/appkey

define Package/myapp/install/common
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/$(APPKEY_PATH)/
endef
####################################################################
define Package/appkey
  SECTION:=apps
  CATEGORY:=myapp
  TITLE:=Autelan Basic App
  DEPENDS:=+libubacktrace +libappkey
endef

define Package/appkey/install
	$(Package/myapp/install/common)
	
	$(INSTALL_BIN) $(PKG_APP_BUILD_DIR)/appkey/appkey $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_APP_BUILD_DIR)/appkey/appkey.key $(1)/$(APPKEY_PATH)/
endef

define Package/appkey/compile
	$(MAKE) -C $(PKG_APP_BUILD_DIR)/appkey \
		CC="$(TARGET_CC)" \
		CFLAGS=" \
			$(TARGET_CFLAGS) \
			$(TARGET_CPPFLAGS) \
			" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef
####################################################################
define Package/jlogd
  SECTION:=apps
  CATEGORY:=myapp
  TITLE:=Autelan Basic App
  DEPENDS:=+appkey +libjson-c
endef

define Package/jlogd/install
	$(Package/myapp/install/common)
	
	$(INSTALL_BIN) $(PKG_APP_BUILD_DIR)/jlogd/jlogd $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_APP_BUILD_DIR)/jlogd/jlogd.key $(1)/$(APPKEY_PATH)/
endef

define Package/jlogd/compile
	$(MAKE) -C $(PKG_APP_BUILD_DIR)/jlogd \
		CC="$(TARGET_CC)" \
		CFLAGS=" \
			$(TARGET_CFLAGS) \
			$(TARGET_CPPFLAGS) \
			" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef
####################################################################
define Package/jlogger
  SECTION:=apps
  CATEGORY:=myapp
  TITLE:=Autelan Basic App
  DEPENDS:=+jlogd
endef

define Package/jlogger/install
	$(Package/myapp/install/common)
	
	$(INSTALL_BIN) $(PKG_APP_BUILD_DIR)/jlogger/jlogger $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_APP_BUILD_DIR)/jlogger/jlogger.key $(1)/$(APPKEY_PATH)/
endef

define Package/jlogger/compile
	$(MAKE) -C $(PKG_APP_BUILD_DIR)/jlogger \
		CC="$(TARGET_CC)" \
		CFLAGS=" \
			$(TARGET_CFLAGS) \
			$(TARGET_CPPFLAGS) \
			" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef
####################################################################
define Package/stimerd
  SECTION:=apps
  CATEGORY:=myapp
  TITLE:=Autelan Basic App
  DEPENDS:=+jlogger
endef

define Package/stimerd/install
	$(Package/myapp/install/common)

	$(INSTALL_BIN) $(PKG_APP_BUILD_DIR)/stimerd/stimerd $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_APP_BUILD_DIR)/stimerd/stimerd.key $(1)/$(APPKEY_PATH)/
endef

define Package/stimerd/compile
	$(MAKE) -C $(PKG_APP_BUILD_DIR)/stimerd \
		CC="$(TARGET_CC)" \
		CFLAGS=" \
			$(TARGET_CFLAGS) \
			$(TARGET_CPPFLAGS) \
			" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef
####################################################################
define Package/stimerc
  SECTION:=apps
  CATEGORY:=myapp
  TITLE:=Autelan Basic App
  DEPENDS:=+stimerd
endef

define Package/stimerc/install
	$(Package/myapp/install/common)

	$(INSTALL_BIN) $(PKG_APP_BUILD_DIR)/stimerc/stimerc $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_APP_BUILD_DIR)/stimerc/stimerc.key $(1)/$(APPKEY_PATH)/
endef

define Package/stimerc/compile
	$(MAKE) -C $(PKG_APP_BUILD_DIR)/stimerc \
		CC="$(TARGET_CC)" \
		CFLAGS=" \
			$(TARGET_CFLAGS) \
			$(TARGET_CPPFLAGS) \
			" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef
####################################################################
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./* $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
	$(Package/appkey/compile)
	$(Package/jlogd/compile)
	$(Package/jlogger/compile)
	$(Package/stimerd/compile)
	$(Package/stimerc/compile)
endef
####################################################################
$(eval $(call BuildPackage,appkey))
$(eval $(call BuildPackage,jlogd))
$(eval $(call BuildPackage,jlogger))
$(eval $(call BuildPackage,stimerd))
$(eval $(call BuildPackage,stimerc))
