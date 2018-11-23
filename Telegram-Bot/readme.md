
#The Python Bot is stored here
#commad to flash the Arduino:

#avrdude -C avrdude.conf -v -p atmega328p -c arduino -P /dev/ttyUSB0 -b 57600 -D -U flash:w:Arduino.ino.hex:i