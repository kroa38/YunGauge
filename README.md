Welcome to the YunGauge !

This project is based on a Arduino Yun and a shield i have made.
The aim of this project is to collect data from my electrical counter
and different sensor (temperature, humidity, magnetic etc ...).

The shield collect data from the electrical counter and filter them. 
the collected data are sent to the Avr Arduino Yun. 
Thanks to the bridge function into the Arduino the data are completed
and send to the linux system. 
A Python script put the data into a Sqlite3 database. 
The data are sent to the Plotly to visualize them.
I have developped some cool script that can be use to send data to Google Speadsheet
or to Google Drive, by using the Oauth2.0 protocol.
You can also send email for notifications.

See the Wiki page for more details

Kroa38 
