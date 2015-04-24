#!/bin/sh
/sbin/uci set  network.lan=wlan0
/sbin/uci set  network.lan.proto=dhcp
/sbin/uci delete  network.lan.ipaddr
/sbin/uci delete  network.lan.netmask
/sbin/uci commit network
/sbin/uci set wireless.@wifi-iface[0].mode=sta
/sbin/uci set wireless.@wifi-iface[0].ssid=guismo
/sbin/uci set wireless.@wifi-iface[0].encryption=psk2
/sbin/uci set wireless.@wifi-iface[0].key=radiofipcsuper
/sbin/uci commit wireless
/sbin/wifi
/etc/init.d/network  restart
