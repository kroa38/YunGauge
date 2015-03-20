#!/bin/ash
# Ping internet to know if it is available
# return 1 if internet is On else return 0

ping -w 4 -q -c 2 www.google.com > ping_status

grep -q "100% packet loss" ping_status

if [ $? -eq 0 ]
then
echo "0"
else
echo "1"
fi;
rm ping_status
