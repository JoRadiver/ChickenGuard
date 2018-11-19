# ChickenGuard
Arduino and Rasperry Pi Smart IoT Chicken Coop Door
This project is made for very specific Hardware and myght be hard to port or understand, 
as the code is not commented nicely and in a mix of German and English.

#You need:

An Arduino
A Motor (DC or Stepper)
A Motor conroller board
One Or two end swithces
A GPS Module
A Power source for the Arduino and the Motor

#For the Telegram Part:
A Rasperry Pi
A power source for the Rpi
A Rpi Camera, preferably IR (called NoIR), with csi port
An USB mini cable to connect the Arduino to the Pi


# How to install:

#Arduino Part
Add the Cpp Libaries to the Arduino Libary folder:
->You can do that by zipping the folder and then use "add .zip libary" from the Arduino IDE
Then open the Arduino.ino file with the Arduino IDE. This should directly open all .ino files from the folder. 
If not make sure you have the desktop version and not the one from the Windows store

#Telegram Part
Add the Telegram-Bot folder to your rpi
install python3 on your Rasperry. 
install all the Libaries needed on your rpi
(maybe i will add a script to do that in the future)
Get a Telegram Bot Token from the Botfather
Create a text file
	telegram_token = 'BOT TOKEN'
	users = []  
	master_chat_id = None # A chat ID to controll the manual mode and be the admin of the system
Save it as config.py
create empty users_chache.txt , logfile.txt and celsiusfile.txt on your pi
All those files need to be inside the Telegram-Bot folder

Connect a Picamera over the csi port. Connect a programmed arduino to the usb port.
Maybe you will need to change the ser = Serial('dev/tty0',9600 timeout = 2 ) line to match your usb port in the Bot.py file. 
You can find out how to do this by googling.
Now Launch the Bot by executing   python3 Bot.py   on your pi
Write something to the bot. It should ignore you because you are not in the users list.
But it will write your user id into the users_chache.txt file
Open the users_chache.txt file.
Copy or Note your user id (A number of 6 or so digits)
open the config.py file in a text editor
change the line    users = []    to     users = [your user id]
for any other perosn using the same bot you have to add his user id like    users = [your user id, his user id]
This is so that nobody else can acces your bot. You can add as many as you like.

#Done!
Your Bot is now ready. You can start by sending \start to the bot to receive your custom keyboard to control the py.


