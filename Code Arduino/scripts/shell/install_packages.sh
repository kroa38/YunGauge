#!/bin/sh
opkg update
opkg install bzip2
opkg install unzip
opkg install tar
opkg install wget
opkg install fdisk
opkg install e2fsprogs
opkg install vsftpd
opkg install pyopenssl
opkg install python-openssl
opkg install python-crypto
opkg install python-bzip2
opkg install python-sqlite3
opkg install python-ncurses
opkg install distribute
easy_install pip
pip install httplib2
pip install google-api-python-client
pip install gspread
pip install plotly
