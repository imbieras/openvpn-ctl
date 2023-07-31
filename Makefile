include $(TOPDIR)/rules.mk

PKG_NAME:=openvpn-ctl
PKG_RELEASE:=1
PKG_VERSION:=0.1.0
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/openvpn-ctl
	CATEGORY:=Utilities
	TITLE:=openvpn-ctl
	DEPENDS:= +libubus +libubox +libblobmsg-json
endef

define Package/openvpn-ctl/description
	Simple package for controlling connnected OpenVPN clients 
endef

define Package/openvpn-ctl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openvpn-ctl $(1)/usr/bin
	$(INSTALL_BIN) ./files/openvpn-ctl.init $(1)/etc/init.d/openvpn-ctl
endef

$(eval $(call BuildPackage,openvpn-ctl))
