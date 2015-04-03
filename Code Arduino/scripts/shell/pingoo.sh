#!/bin/sh

wget -q --tries=20 --timeout=10 --no-check-certificate https://www.google.com -O /tmp/google.idx &> /dev/null
if [ ! -s /tmp/google.idx ]
then
    echo "0"
else
    echo "1"
fi
