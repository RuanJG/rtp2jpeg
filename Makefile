#
# Copyright (C) 2010-2013 hua.shao@mediatek.com
#
# Ralink Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtp2jpeg
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/rtp2jpeg
	SECTION:=firefly
	CATEGORY:=rtp2jpeg
	TITLE:= rtp2jpeg
	DEPENDS:=+libjpeg +libpthread +kmod-video-core +kmod-video-uvc +kmod-usb-core +kmod-usb2
endef

define Package/rtp2jpeg/description
	An program to config rtp2jpeg.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Package/rtp2jpeg/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/camera_encode $(1)/usr/bin
endef
$(eval $(call BuildPackage,rtp2jpeg))
