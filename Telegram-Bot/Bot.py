import time
import random
from datetime import datetime
import telepot
import os
import serial
import struct
import config
from picamera import PiCamera
from time import sleep
from threading import Lock
ser = None
bot = None
from telepot.namedtuple import InlineKeyboardButton, InlineKeyboardMarkup
from telepot.namedtuple import ReplyKeyboardMarkup, KeyboardButton
#=========================================================================#


#===============================GLOBALS===================================#
serial_lock = Lock()
camera = PiCamera()
photo = None
manuellmodus = False
#=========================================================================#


#==========================TELEGRAM KEYBOARDS=============================#
full_keyboard = ReplyKeyboardMarkup(
								keyboard=[
									[KeyboardButton(text = '\U0001F4D6 Status'), KeyboardButton(text="\U0001F4F8 Bild")],
									[KeyboardButton(text = '\U00002B06 Öffnen'), KeyboardButton(text="\U00002B07 Schliessen")],
									[KeyboardButton(text = '\U00002600 Licht an'), KeyboardButton(text="\U0001F312 Licht aus")],
									[KeyboardButton(text = '/start'), KeyboardButton(text="\U0000267B Refresh")]
								]
							)
							
fehler_keyboard = ReplyKeyboardMarkup(
								keyboard=[
									[KeyboardButton(text = 'Fehlerlog'), KeyboardButton(text="\U0001F4F8 Bild")],
									[KeyboardButton(text = 'Aufrollen'), KeyboardButton(text="Abrollen"), KeyboardButton(text = "Stop")],
									[KeyboardButton(text = 'Warte +60min')]
									[KeyboardButton(text="\U0000267B Zurück")]
								]
							)
							
yesno_keyboard = ReplyKeyboardMarkup(
								keyboard=[
									[KeyboardButton(text = 'Ja'), KeyboardButton(text="Nein")],
								]
							)
#=========================================================================#



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
			if len(e) < 2 :
				continue
			letter = e[0]
			e = e[1:]
			if letter == 'T':
				line = "*Türchen:*	   "
				words = ["Geschlossen","Offen","Schliesst","Öffnet", "Fehler"]
				line += words[int(e)]
			elif letter == 'Z':
				line = "*Zaun:*		   "
				words = ["An","Aus"]
				line += words[int(e)]
			elif letter == 'L':
				line = "*Licht:*	   "
				words = ["An","Aus"]
				line += words[int(e)]
			elif letter == 'C':
				line = "*Temparatur:*  "
				line += float(e) + "°C"
				if write :
					Tf.write(float(e))
			elif letter == 'a':
				line = "*Systemzeit:*  "
				line += time_from_unix_int(int(e))
				line += "	 " + date_from_unix_int(int(e))
			elif letter == 'r':
				line = "*Öffnungszeit:* "
				line += time_from_unix_int(int(e))
			elif letter == 's':
				line = "*Schliesszeit:* "
				line += time_from_unix_int(int(e))
			elif letter == 'X':
				line = "*Fehler:*		"
				line += e
			elif letter == 'H':
				line = "*Fehler:*		"
				line += e
				if config.master_chat_id != None:
					bot.sendMessage(config.master_chat_id, line, reply_markup = fehler_keyboard)
			elif letter == 'P':
				line = "*Nachricht:*		"
				line += e
				if config.master_chat_id != None:
					bot.sendMessage(config.master_chat_id, line, reply_markup = fehler_keyboard)
					line = ""
			elif len(e)>0:
				line = "*Andere:*		"
				line += e
			text += line + "\U0000000A"
		if write:
			lf.write(text.strip('*'))
		return text


def send_to_arduino(tosend):
	with serial_lock:
		ser.write((tosend).encode())
					
def cache_user(user):
	with open('users.txt', 'r+') as f:
		f.write(user)
	
def authenticate(user):
	if user in config.users:
		return True
	return False

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
# 20: Start Manual Mode
# 21: Get Error Log
# 22: Roll Up (in Manual mode)
# 23: Roll Down (in manual mode)
# 24: Stop Motor (in manual mode)
# 25: Warte 60 min länger (in manual mode)
# 26: Exit Manual Mode
#The handle Function is called by the telepot thread, 
#whenever a message is received from Telegram
def handle(msg):
		global serial_lock
		chat_id = msg['chat']['id']
		command = msg['text']
		user_id = chat_id
		if authenticate(user_id):
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
				except Exception as e:	#falls es fehlgeschlagen ist, melde das.
					print( '----failed to send or capture picture---' )
					print(e)
					print( '----------------------------------------' )
					bot.sendMessage(chat_id, "Bild oeffnen oder senden fehlgeschlagen.")
			elif 'Schliessen' in command:				
					send_to_arduino("s03")
					bot.sendMessage(chat_id, 'Türchen schliesst.')
			elif 'Öfnnen' in command:				
					send_to_arduino("s02")
					bot.sendMessage(chat_id, 'Türchen öffnet') 
			elif 'Refresh' in command:				
					send_to_arduino("s08")
					bot.sendMessage(chat_id, 'Suche Satelliten.')
			elif 'Licht an' in command:				
					send_to_arduino("s06")
					bot.sendMessage(chat_id, 'Licht an.')
			elif 'Licht aus' in command:				
					send_to_arduino("s07")
					bot.sendMessage(chat_id, 'Licht aus.')
			elif 'Zaun an' in command:
					send_to_arduino("s04")
					bot.sendMessage(chat_id, 'Zaun an')
			elif 'Zaun aus' in command:
					send_to_arduino("s05")
					bot.sendMessage(chat_id, 'Zaun aus')
			elif 'Temparatur' in command:
				with serial_lock:
					while ser.in_waiting > 0:
						log_message(ser.readline().decode('utf-8').strip('\n'),write = True)
					send_to_arduino("s09")
					line = ser.readline().decode('utf-8').strip('\n')
					bot.sendMessage(chat_id, log_message(line), parse_mode = 'Markdown')
			elif 'Status' in command:
				#Here i should make sure that nothing 
				#is waiting from the Arduino
				#so that the next two Serial lines are the Arduinos 
				#respoonce to the "LOG" command.
				#and that hanlde is the only 
				#function talking to the Serial port now.
				with serial_lock:
					while ser.in_waiting > 0:
						line = ser.readline().decode('utf-8').strip('\n')
						if len(line)>1:
							log_message(line,write = True)
					ser.write(("s01").encode())
					i=0
					while ser.in_waiting == 0 and i<50:
						sleep(1)
						i+=1
					line = ser.readline().decode('utf-8').strip('\n')
					if len(line) >1:
						bot.sendMessage(chat_id, "LOG\U0000000A" + log_message(line), parse_mode = 'Markdown')
					else:
						print("no response from Arduino in 50s")
						bot.sendMessage(chat_id, "*Fehler:* Keine Antwort erhalten in 50 Sekunden")
				#The Arduinos response is now saved as one string 
				#and sent to the User.
			elif 'Manuell' in command and chat_id == config.master_chat_id:
				bot.sendMessage(chat_id, "Manueller modus aktiviert", reply_markup = fehler_keyboard)
				send_to_arduino("s20")
			elif 'Fehlerlog' in command and chat_id == config.master_chat_id:
				send_to_arduino("s21")
			elif 'Aufrollen' in command and chat_id == config.master_chat_id:
				send_to_arduino("s22")
				bot.sendMessage(chat_id, "Rolle Auf")
			elif 'Abrollen' in command and chat_id == config.master_chat_id:
				send_to_arduino("s23")
				bot.sendMessage(chat_id, "Rolle Ab")
			elif 'Stop' in command and chat_id == config.master_chat_id:
				send_to_arduino("s24")
				bot.sendMessage(chat_id, "Motor Stop")
			elif 'Warte +60 min' in command:
				send_to_arduino("s25")
				bot.sendMessage(chat_id, "Arduino Wartet Unendlich")
			elif 'Zurück' in command and chat_id == config.master_chat_id:
				send_to_arduino("s26")
				bot.sendMessage(user_id, "Manueller modus beendet", reply_markup = full_keyboard)
			elif 'start' in command:
				bot.sendMessage(user_id, "Willkommen", reply_markup = full_keyboard)	
			print("Command Processed.")
		else:
			cache_user(user_id)
			print("Unauthorized Acces.")

		

ser = serial.Serial("/dev/ttyUSB0", 9600)
print("Serial Port ready.")

startMSG = ""
print('Arduino Gibberish: ')
while not "Arduino ready" in startMSG:
	while ser.in_waiting < 3:
		pass
	startMSG = ser.readline().decode('utf-8').strip('\n')
	print(startMSG)
print("Arduino ready.")

bot = telepot.Bot(config.telegram_token)
bot.message_loop(handle)
print("Bot ready.")

		
while True:
	#anything to make it not run at full speed (Recommendations welcome)
	#The log updates are only once a hour
	sleep(3)
	#here i need to make sure it does not collide with the other thread.
	with serial_lock:
		while ser.in_waiting > 0:
			try:
				line = ser.readline().decode('utf-8').strip('\n')
				log_message(line,write = True)
			except UnicodeDecodeError:
				print("Bad Arduino Data")
		
		
