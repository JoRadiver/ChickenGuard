import time
import random
from datetime import datetime
import telepot
import os
import serial
import struct
from picamera import PiCamera
from time import sleep
from threading import Lock
ser = None
bot = None
from telepot.namedtuple import InlineKeyboardButton, InlineKeyboardMarkup
from telepot.namedtuple import ReplyKeyboardMarkup, KeyboardButton

serial_lock = Lock()


camera = PiCamera()

photo = None

def time_from_unix_int(time):
	return datetime.fromtimestamp(time).strftime('%H:%M')

def date_from_unix_int(time):
	str = datetime.fromtimestamp(time).strftime('%d. ')
	words = ["Januar","Februar","März","April","Mai","Juni","Juli","August","September","Oktober","November","Dezember"]
	str += words[int(datetime.fromtimestamp(time).strftime('%m'))-1]
	return str
	
def log_message(str, write = False):
	with open("logfile.txt",'r+') as lf, open("celsiusfile.txt", 'r+') as Tf :
		print("Received from ardiuno: "+str)
		list = str.split(';')		
		text = ""
		for e in list:
			line = ""
			letter = e[0]
			e = e[1:]
			if letter == 'T':
				line = "Türchen: "
				words = ["Geschlossen","Offen","Schliesst","Öffnet", "Fehler"]
				line += words[int(e)]
			elif letter == 'Z':
				line = "Zaun: "
				words = ["Ein","Aus"]
				line += words[int(e)]
			elif letter == 'L':
				line = "Licht: "
				words = ["Ein","Aus"]
				line += words[int(e)]
			elif letter == 'C':
				line = "Temparatur: "
				line += float(e) + "°C"
				if write :
					Tf.write(float(e))
			elif letter == 'a':
				line = "Systemzeit: "
				line += time_from_unix_int(int(e))
				line += "    " + date_from_unix_int(int(e))
			elif letter == 'r':
				line = "Öffnungszeit: "
				line += time_from_unix_int(int(e))
			elif letter == 's':
				line = "Schliesszeit: "
				line += time_from_unix_int(int(e))
			elif letter == 'X':
				line = "Fehler: "
				line += e
			elif len(e)>0:
				line = "Andere: "
				line += e
			text += line + "\U0000000A"
		if write:
			lf.write(text)
		return text

def authenticate(chat_id):
	return True
	
	
guest_Keyboard = ReplyKeyboardMarkup(
								keyboard=[
									[KeyboardButton(text = '\U0001F4F8 Bild 	'),KeyboardButton(text='\U0001F4D6 Log')],
									[KeyboardButton(text = "\U0001F4A1 Authenticate")]
								]
							)
	
login_Keyboard = ReplyKeyboardMarkup(
								keyboard=[
									[KeyboardButton(text = '1'), KeyboardButton(text="2"), KeyboardButton(text="3")],
									[KeyboardButton(text = '4'), KeyboardButton(text="5"), KeyboardButton(text="6")],
									[KeyboardButton(text = '7'), KeyboardButton(text="8"), KeyboardButton(text="9")],
									[KeyboardButton(text = 'Authenticate'), KeyboardButton(text = 'Back')]
								]
							)
full_keyboard = ReplyKeyboardMarkup(
								keyboard=[
									[KeyboardButton(text = '\U0001F4D6 Log'), KeyboardButton(text="\U0001F4F8 Bild")],
									[KeyboardButton(text = '\U00002B06 Open Door'), KeyboardButton(text="\U00002B07 Close Door")],
									[KeyboardButton(text = '\U00002600 Licht an'), KeyboardButton(text="\U0001F312 Licht aus")],
									[KeyboardButton(text = '/start'), KeyboardButton(text="\U0000267B Refresh")]
								]
							)
	
	
	
#Command Codes For Chicken Door:
# 01: Log
# 02: OpenDoor
# 03: CloseDoor
# 04: FenceOn
# 05: FenceOff
# 06: LightOn
# 07: LightOff
# 08: Refresh
# 09: Temparature
# 10: DebugEnable
# 11: DebugDisable

#The handle Function is called by the telepot thread, 
#whenever a message is received from Telegram
def handle(msg):
		global serial_lock
		chat_id = msg['chat']['id']
		command = msg['text']
		if authenticate(chat_id):
			print( 'Command Received: %s' % command)
			if command == '/start':
				bot.sendMessage(chat_id, 'welcome')
			elif 'Bild' in command:
				try:
					camera.start_preview()
					sleep(2)
					camera.capture('/home/pi/bild.jpg')
					camera.stop_preview()
					with open('/home/pi/bild.jpg', 'rb') as photo: #oeffne das bild nur wenn es gebraucht wird
						bot.sendPhoto(chat_id, photo)
				except Exception as e:  #falls es fehlgeschlagen ist, melde das.
					print( '----failed to send or capture picture---' )
					print(e)
					print( '----------------------------------------' )
					bot.sendMessage(chat_id, "Bild oeffnen oder senden fehlgeschlagen.")

			elif 'Schliessen' in command:
				with serial_lock:
					ser.write(("s03").encode()+b'\n')
					bot.sendMessage(chat_id, 'Türchen schliesst.')
			elif 'Öfnnen' in command:
				with serial_lock:
					ser.write(("s02").encode()+b'\n')
					bot.sendMessage(chat_id, 'Türchen öffnet') 
			elif 'Refresh' in command:
				with serial_lock:
					ser.write(("s08").encode()+b'\n')
					bot.sendMessage(chat_id, 'Suche Satelliten.')
			elif 'Licht an' in command:
				with serial_lock:
					ser.write(("s06").encode()+b'\n')
					bot.sendMessage(chat_id, 'Licht an.')
			elif 'Licht aus' in command:
				with serial_lock:
					ser.write(("s07").encode()+b'\n')
					bot.sendMessage(chat_id, 'Licht aus.')
			elif 'Zaun an' in command:
				with serial_lock:
					ser.write(("s04").encode()+b'\n')
					bot.sendMessage(chat_id, 'Zaun an')
			elif 'Zaun aus' in command:
				with serial_lock:
					ser.write(("s05").encode()+b'\n')
					bot.sendMessage(chat_id, 'Zaun aus')
			elif 'Temparatur' in command:
				with serial_lock:
					while ser.in_waiting > 0:
						log_message(str(ser.readline(),'ascii'),write = True)
					ser.write(("s09").encode()+b'\n')
					bot.sendMessage(chat_id, log_message(str(ser.readline(),'ascii')))
			elif 'Log' in command:
				#Here i should make sure that nothing 
				#is waiting from the Arduino
				#so that the next two Serial lines are the Arduinos 
				#respoonce to the "LOG" command.
				#and that hanlde is the only 
				#function talking to the Serial port now.
				with serial_lock:
					while ser.in_waiting > 0:
						line = str(ser.readline(),'ascii')
						if len(line)>1:
							log_message(line,write = True)
					ser.write(("s01").encode()+b'\n')
					i=0
					while ser.in_waiting == 0 and i<50:
						sleep(1)
						i+=1
					line = str(ser.readline(),'ascii')
					if len(line) >1:
						bot.sendMessage(chat_id, "LOG\U0000000A" + log_message(line))
					else:
						print("no response from Arduino in 50s")
						bot.sendMessage(chat_id, "Keine Antwort erhalten in 50 Sekunden")
				#The Arduinos response is now saved as one string 
				#and sent to the User.
			elif 'start' in command:
				bot.sendMessage(user_id, "Welcome", reply_markup = full_keyboard)
				
			print("Command Processed.")
		else:
			print("Unauthorized Acces.")

		
		
print(("s08").encode()+b'\n')	
bot = telepot.Bot('660105356:AAE6t8sryjvqdctISLUvGqLJe7ondnRxeE8')
bot.message_loop(handle)


ser = serial.Serial("/dev/ttyUSB0", 9600, timeout = 2)

print( 'I am listening ...')
		
while True:
	#anything to make it not run at full speed (Recommendations welcome)
	#The log updates are only once a hour
	sleep(3)
	#here i need to make sure it does not collide with the other thread.
	with serial_lock:
		while ser.in_waiting > 0:
			try:
				line = struct.unpack(ser.readline())
				log_message(line,write = True)
			except UnicodeDecodeError:
				print("Bad Arduino Data")
		
		