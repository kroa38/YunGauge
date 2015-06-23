Welcome to the YunGauge wiki!

This project is based on a Arduino Yun and a shield i have created.
The aim of this project is to collect data from my electrical counter
and different sensor (temperature, humidity, etc ...) . 
The shield collect data from the electrical counter and filter them. 
the colelcted data are sent to the Avr Arduino Yun. 
Thanks to the bridge function into the Arduino the data are completed
and send linux system. Different Python script put the data into a sqlite3 database. 
The data are sent to the website of Plotly to visualize them.
