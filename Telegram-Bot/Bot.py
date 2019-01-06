#!/usr/bin/env python3

import time
from gpiozero import LED
import random
from datetime import datetime
import telepot
import os
import serial
import struct
import config
from picamera import PiCamera
from threading import RLock
from telepot.namedtuple import ReplyKeyboardMarkup, KeyboardButton
#=========================================================================#


#===============================GLOBALS===================================#
ser = None
bot = None
serial_lock = RLock()
file_lock = RLock()
camera = PiCamera()
photo = None
manuellmodus = False
IR_led = LED(14)
logfile_path = os.path.join(config.workingPath, 'logfile.txt')
celsiusfile_path = os.path.join(config.workingPath, 'celsiusfile.txt')
userfile_path = os.path.join(config.workingPath, 'users.txt')
#=========================================================================#


#==========================TELEGRAM KEYBOARDS=============================#
full_keyboard = ReplyKeyboardMarkup(
								keyboard=[
									[KeyboardButton(text = '\U0001F4D6 Status'), KeyboardButton(text="\U0001F4F8 Bild")],
									[KeyboardButton(text = '\U00002B06 Öffnen'), KeyboardButton(text="\U00002B07 Schliessen")],
									[KeyboardButton(text = '/start'), KeyboardButton(text="\U0000267B Refresh")]
								]
							)
					
fehler_keyboard = ReplyKeyboardMarkup(
								keyboard=[
									[KeyboardButton(text = 'Fehlerlog'), KeyboardButton(text="\U0001F4F8 Bild")],
									[KeyboardButton(text = 'Aufrollen'), KeyboardButton(text="Abrollen"), KeyboardButton(text = "Stop")],
									[KeyboardButton(text = 'Warte +60min')],
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
	stri = datetime.fromtimestamp(time).strftime('%d. ')
	words = ["Januar","Februar","März","April","Mai","Juni","Juli","August","September","Oktober","November","Dezember"]
	stri += words[int(datetime.fromtimestamp(time).strftime('%m'))-1]
	return stri
	
def log_message(stri, write = False):
	with file_lock:
		with open(logfile_path,'a') as lf, open(celsiusfile_path, 'a') as Tf :
			print()
			print(" - Received from ardiuno: "+stri)
			list = stri.split(';')		
			text = ""
			for e in list:
				line = ""
				if len(e) < 2 :
					continue
				letter = e[0]
				e = e[1:]
				if letter == 'T':  #Türchen
					line = "*Türchen:*	   "
					words = ["Geschlossen","Offen","Schliesst","Öffnet", "Fehler"]
					line += words[int(e)]
				elif letter == 'Z': #Zaun
					line = "*Zaun:*		   "
					words = ["An","Aus"]
					line += words[int(e)]
				elif letter == 'L':  #Licht
					line = "*Licht:*	   "
					words = ["An","Aus"]
					line += words[int(e)]
				elif letter == 'C':  #Temparatur
					line = "*Temparatur:*  "
					line += float(e) + "°C"
					if write :
						Tf.write('\n')
						Tf.write(float(e))
						Tf.write('\n')
				elif letter == 'a':  #
					line = "*Systemzeit:*  "
					line += time_from_unix_int(int(e))
					line += "	 " + date_from_unix_int(int(e))
				elif letter == 'r':
					line = "*Öffnungszeit:* "
					line += time_from_unix_int(int(e))
				elif letter == 's':
					line = "*Schliesszeit:* "
					line += time_from_unix_int(int(e))
				elif letter == 'S':
					line = "*Arduino hat gestartet*"
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
						bot.sendMessage(config.master_chat_id, line)
						line = ""
				elif letter == 'D': #Differenz
					line = "*Zeit Aktualisiert:* "
					shifttime = int(e)
					if shifttime < 60:
						line += "Zeit hat sich um wenige sekunden verändert."
					elif shifttime >= 60:
						line += "Zeit hat sich um "
						line += str(int(e)//60)
						line += " Minuten verändert."
					elif shifttime > 3600:
						line += "Zeit wurde eingestellt."
				elif letter == ' ': #Those messages are not logged.
					pass
				elif len(e)>0:
					line = "*Andere:*		"
					line += e
				text += line + "\U0000000A"
			if write:
				lf.write('\n')
				lf.write('-----------------------------------\n')
				lf.write(text.strip('*'))
				lf.write('-----------------------------------\n')
			return text


def send_to_arduino(tosend):
	with serial_lock:
		print(" => Sent to Arduino: "+tosend)
		ser.write((tosend).encode()+ b'\n')
					
def cache_user(user):
	with file_lock:
		with open(userfile_path, 'a') as f:
			f.write(str(user))
	
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
		print()
		print('------------------------------------------')
		if authenticate(user_id):
			print( 'Command Received: %s' % command)
			if 'start' in command:
				bot.sendMessage(user_id, "Willkommen", reply_markup = full_keyboard)
			elif 'Bild' in command:
				try:
					IR_led.on()
					camera.start_preview()
					time.sleep(2)
					camera.vflip = True
					camera.hflip = False
					camera.capture('/home/pi/bild.jpg')
					camera.stop_preview()
					IR_led.off()
					with open(os.path.join(config.workingPath,'/home/pi/bild.jpg'), 'rb') as photo: #oeffne das bild nur wenn es gebraucht wird
						bot.sendPhoto(chat_id, photo)
				except Exception as e:	#falls es fehlgeschlagen ist, melde das.
					print( '----failed to send or capture picture---' )
					print(e)
					print( '----------------------------------------' )
					bot.sendMessage(chat_id, "Bild oeffnen oder senden fehlgeschlagen.")
			elif 'Schliessen' in command:				
					send_to_arduino("s03")
					bot.sendMessage(chat_id, 'Türchen schliesst.')
			elif 'Öffnen' in command:				
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
				print(" trying serial lock:")
				with serial_lock:
					print(" got serial lock")
					while ser.in_waiting > 0:
						line = ser.readline().decode('utf-8').strip('\n')
						if len(line)>1:
							log_message(line,write = True)
					send_to_arduino("s01")
					i=0
					print(" arduino data skimmed off and command sent")
					while ser.in_waiting < 1 and i<50:
						time.sleep(1)
						i+=1
						print(" .")
					line = ser.readline().decode('utf-8').strip('\n')
					if len(line) >1:
						bot.sendMessage(chat_id, "LOG\U0000000A" + log_message(line), parse_mode = 'Markdown')
					else:
						print(" no response from Arduino in 50s")
						bot.sendMessage(chat_id, "*Fehler:* Keine Antwort erhalten in 50 Sekunden", parse_mode = 'Markdown')
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
			elif 'Warte +60 min' in command  and chat_id == config.master_chat_id:
				send_to_arduino("s25")
				bot.sendMessage(chat_id, "Arduino Wartet Unendlich")
			elif 'Logfile' in command:
				with file_lock:
					with open(logfile_path, 'rb') as file:
						bot.sendDocument(chat_id, file)
			elif 'Tempfile' in command:
				with file_lock:
					with open(celsiusfile_path, 'rb') as file:
						bot.sendDocument(chat_id, file)
			elif 'Zurück' in command and chat_id == config.master_chat_id:
				send_to_arduino("s26")
				bot.sendMessage(user_id, "Manueller modus beendet", reply_markup = full_keyboard)
			elif 'deb-en' in command and chat_id == config.master_chat_id:
				send_to_arduino("s10")
			elif 'deb-stop' in command and chat_id == config.master_chat_id:
				send_to_arduino("s11")
			print("Command Processed.")
		else:
			cache_user(user_id)
			print("Unauthorized Acces from:"+str(user_id))
		print('------------------------------------------')
		


#Create files if they don't yet exist.
try :
	with open(logfile_path, 'r') as f:
		pass
except IOError:
	f = open(logfile_path, 'w')
	f.close()
try :
	with open(celsiusfile_path, 'r') as f:
		pass
except IOError:
	f = open(celsiusfile_path, 'w')
	f.close()
try :
	with open(userfile_path, 'r') as f:
		pass
except IOError:
	f = open(userfile_path, 'w')
	f.close()

	
print()
print()
print('################# BOT STARTING #################')
ser = serial.Serial("/dev/ttyUSB0", 9600, timeout = 2)
print("Serial Port ready.")

startMSG = ""
print('Waiting for Arduino Boot: ')
send_to_arduino('s99')
i = 0
while not "SR;" in startMSG:
	i+=1
	if ser.in_waiting > 3:
		startMSG = ser.readline().decode('utf-8').strip('\n')
		print('  Arduino reports: ' + startMSG)
	if i == 200 or i == 300 or i == 400 or i == 600:
		send_to_arduino('s99')
	if i == 1800 or 'FJ ist' in startMSG:
		i = 1200
		send_to_arduino('s26')
		send_to_arduino('s99')
	time.sleep(.1)
print("Arduino ready.")
bot = telepot.Bot(config.telegram_token)
bot.message_loop(handle)
print("Bot ready.")
print('###############################################')
time.sleep(30)
send_to_arduino("s08")
while True:
	#anything to make it not run at full speed (Recommendations welcome)
	#The log updates are only once a hour
	time.sleep(3)
	#here i need to make sure it does not collide with the other thread.
	with serial_lock:
		while ser.in_waiting > 0:
			try:
				line = ser.readline().decode('utf-8').strip('\n')
				log_message(line,write = True)
			except UnicodeDecodeError:
				print("Bad Arduino Data")
		
		
