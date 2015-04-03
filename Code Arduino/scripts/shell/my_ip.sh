#!/bin/sh

eth_ip=$(ifconfig eth1 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}')

wlan_ip=$(ifconfig wlan0 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}')

if [ $eth_ip ]
then
	result=$eth_ip
        python /mnt/sda1/temboo_python_sdk_2.5.0/./email.py Doguino $eth_ip 

elif [ $wlan_ip ]
then
	result=$wlan_ip
        python /mnt/sda1/temboo_python_sdk_2.5.0/./email.py Doguino $wlan_ip

else
        python /mnt/sda1/temboo_python_sdk_2.5.0/./email.py Doguino No_Ip_Adress 
fi;

