#!/bin/ash

ping -w 4 -q -c 2 localhost > ping_status

grep -q "100% packet loss" ping_status

if [ $? -eq 0 ]
then
echo "0"
else
echo "1"
fi;
rm ping_status
