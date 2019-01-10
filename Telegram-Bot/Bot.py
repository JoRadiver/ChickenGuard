#!/usr/bin/env python3

# =========================================
# File Structure:
# -----------------------------------------
# imports
# -----------------------------------------
# constants and Globals
# -----------------------------------------
# keyboards: All telegram custom keyboards
# -----------------------------------------
# Users:
	# Class and functions related to authentication
# -----------------------------------------
# utility functions: 
	# Functions used to convert or calculate 
	# certain things
# -----------------------------------------
# Arduino receive functions: 
	# Functions that parse information sent
	# from the Arduino
# -----------------------------------------
# handle Functions: 
	# Handle Functions Parse the commands 
	# sent by the Telegram User 
	# (executed by the telepot libary)
# -----------------------------------------
# start preparations:
	# makes sure all files exist.
# -----------------------------------------
# Start sequqence:
	# initiates Arduino reboot, waits for 
	# its boot, starts telegram bot, 
# -----------------------------------------
#RUNTIME CODE
#	listens for arduino input
# ==========================================


# TELEGRAM COMMANDS: (master can all admin, admin all user, etc.)
# Master: 
	# approve [approve <chat_id> <status>]: changes status of a user,
	# lspeople: lists all users,
	# name [name <chat_id> <username>]: give a name to a user,
	# Logfile, Tempfile, Userfile,
	# deb-en: enables debug on arudino (carefull!), deb-stop
# Admin:
	# manuell: aktiviert manuelle steuerung	
		# Zurück: beendet manuelle steuerung
		# Aufrollen: motor direkt ansteuern (schalter werden ignoriert)
		# Abrollen:
		# Stop:
		# Warte +60 min: arduino beendet die manuelle steuerung nicht
# User:
	# Schliessen: schliesst das törchen für in Arduino.ino festgelegte zeit
	# Öffnen: öffnet "
	# LichtAn:
	# LichtAus:
	# ZaunAn:
	# ZaunAus:
	# Temparatur:
	# Zeit Aktualisieren:
	
# Guest:
	# Film [Film <sek>]:  macht und sendet einen Film von 20+ sek
	# Bild: macht und sendet ein Bild
	# Status: sendet einen Statusbericht vom Arduino.
# Stranger:

#=================================IMPORTS==================================#
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
import sys
from subprocess import check_call, CalledProcessError
import re
#=========================================================================#


#===============================CONSTANTS===================================#
ser = None
bot = None
serial_lock = RLock()
file_lock = RLock()
camera = PiCamera()
photo = None
manuellmodus = False
IR_led = LED(14)
BURST_PER_HOUR = 60
MAX_USERS = 99
glob_user_list = []
glob_master = None
logfile_path = os.path.join(config.workingPath, 'logfile.txt')
celsiusfile_path = os.path.join(config.workingPath, 'celsiusfile.txt')
userfile_path = os.path.join(config.workingPath, 'users.txt')
#=========================================================================#


#==========================TELEGRAM KEYBOARDS=======================s=====#


stranger_protoype =  [
					  [KeyboardButton(text = "\U0001F413")] # The chicken button ist the only available to a stranger
					 ]
guest_prototype =    [
					  [KeyboardButton(text = "\U0001F413"), KeyboardButton(text = '\U0001F4D6 Status')],
				      [KeyboardButton(text = "\U0001F4F8 Bild"), KeyboardButton(text = "\U0001F3A5 Film 8")],
					 ]
					 
user_prototype = 	 [
					  [KeyboardButton(text = '\U0001F4D6 Status'), KeyboardButton(text="\U0001F4F8 Bild"), KeyboardButton(text="\U0001F3A5 Film 8")],
					  [KeyboardButton(text = '\U00002B06 Öffnen'), KeyboardButton(text="\U00002B07 Schliessen")],
					  [KeyboardButton(text = '/start'), KeyboardButton(text="\U0000267B Zeit Aktualisieren")]
					 ]
					 
admin_prototype = user_prototype + [[KeyboardButton(text = 'Manuell'), KeyboardButton(text = "Neustart")]]
master_protoype = admin_prototype + [[KeyboardButton(text = 'Erweitern')]]
master_extended_prototype = master_protoype[:-1] + [[KeyboardButton(text = 'Logfile'), KeyboardButton(text = "Userfile"), KeyboardButton(text = "lspeople") ],
													[KeyboardButton(text = 'deb-en'), KeyboardButton(text = "deb-stop")],
													[KeyboardButton(text = 'kleiner')]
													]

stranger_keyboard =  ReplyKeyboardMarkup( keyboard = stranger_protoype )
guest_keyboard = ReplyKeyboardMarkup( keyboard = guest_prototype )
user_keyboard = ReplyKeyboardMarkup( keyboard = user_prototype )
admin_keyboard = ReplyKeyboardMarkup( keyboard = admin_prototype )
master_keyboard = ReplyKeyboardMarkup( keyboard = master_protoype )
													
# full_keyboard = ReplyKeyboardMarkup(
								# keyboard=[
									# [KeyboardButton(text = '\U0001F4D6 Status'), KeyboardButton(text="\U0001F4F8 Bild"), KeyboardButton(text="\U0001F3A5 Film 8")],
									# [KeyboardButton(text = '\U00002B06 Öffnen'), KeyboardButton(text="\U00002B07 Schliessen")],
									# [KeyboardButton(text = '/start'), KeyboardButton(text="\U0000267B Refresh")]
								# ]
							# )
					
fehler_keyboard = ReplyKeyboardMarkup(
								keyboard=[
									[KeyboardButton(text = 'Fehlerlog'), KeyboardButton(text="\U0001F4F8 Bild")],
									[KeyboardButton(text = 'Aufrollen'), KeyboardButton(text="Abrollen"), KeyboardButton(text = "Stop")],
									[KeyboardButton(text = 'Warte +60min')],
									[KeyboardButton(text="\U0000267B Zurück")]
								]
							)

login_Keyboard = ReplyKeyboardMarkup(
								keyboard=[
									[KeyboardButton(text = '1'), KeyboardButton(text="2"), KeyboardButton(text="3")],
									[KeyboardButton(text = '4'), KeyboardButton(text="5"), KeyboardButton(text="6")],
									[KeyboardButton(text = '7'), KeyboardButton(text="8"), KeyboardButton(text="9")],
									[KeyboardButton(text = 'Authenticate'), KeyboardButton(text = 'Back')]
							
yesno_keyboard = ReplyKeyboardMarkup(
								keyboard=[
									[KeyboardButton(text = 'Ja'), KeyboardButton(text="Nein")],
								]
							)
#=========================================================================#

#===================================USERS======================================#
class User:
	def __init__(self, chat_id, status = "guest"):
		self.id = chat_id
		self.user_id = None
		self.name = None
		self.status = status #stranger, guest, user, admin
		self.job = "none" #none, login, manual
		self.jobstep = 0 #some jobs require count of steps
		self.lastactive = 0 #when was user last active
		self.burst = BURST_PER_HOUR #how many orders may be processed in a short time
		self.keyboard = get_keyboard()
	
	#sometimes a user issues too many commands. this hinders the bot from beeing blocked.	
	def is_on_cooldown(self):
		if status == "admin":
			return False #Admin has no cooldown
		self.burst -= 1
		if self.burst < -10:
			self.burst = -10
		self.restore_points()
		if burst == 0: 
			bot.sendMessage(self.id, "Zu viele nachrichten. Warte 10 min")
		return burst <= 0
		
	def restore_points(self):
		self.burst += (now()-self.lastactive)/3600 * BURST_PER_HOUR
		if self.burst > BURST_PER_HOUR:
			self.burst = BURST_PER_HOUR
			
	def now(): #pseudo replaced by time function
		return 0
		
	def change_status(self, new_status):
		self.status = new_status
		
	def change_name(self, new_name):
		self.name = new_name
	
	def get_keyboard(self):
		if self.status = "admin":
			self.keyboard = admin_keyboard
		elif self.status = "user":
			self.keyboard = user_keyboard
		elif self.status = "guest":
			self.keyboard = guest_keyboard
		elif self.status = "stranger":
			self.keyboard = stranger_keyboard
		return self.keyboard
		
	def set_job(self, job):
		if self.job == "none":	
			self.job = job
			return True
		else:
			return False
			
	def job_done(self):
		self.job = "none"
		self.jobstep = 0
		
	def file_repr(self):
		if self.name = None:
			return str(self.id) + ";" +  self.status
		else:
			return str(self.id) + ";" + self.status + ";" + self.name
	def __repr__(self):
		if self.name == None:
			return "<User: " + str(self.id)+">"
		else:
			return "<User: " + self.name + "id:  "+str(self.id)+">"

def login_handler(user, command):
	if user.status = "admin":
		user.job_done()
		return True
	elif "Zurück" in command:
		user.job_done()
		return False
	password = ['0','0','0','0','0']
	elif "Login" in command:
		if user.jobstep == len(password)-1:
			user.status == "admin"
			return True
	if command = password[user.jobstep] and user.jobstep != -1:
		user.jobstep += 1
	else:
		user.jobstep = -1
	return False

def find_user_from_chat_id(chat_id, noappend = False):
	for user in glob_user_list:
		if user.id == chat_id:
			return user
	else:
		
		new_user = User(chat_id, "stranger")
		if len(glob_user_list) > MAX_USERS:
			return new_user #too many users. will not be appended
		else:
			notify_master("New User: " + str(chat_id))
			glob_user_list.append(new_user)
			
def update_userfile():
	global glob_user_list
	try:
		with open(userfile_path, 'w') as f:
			for user in glob_user_list:
				if user == glob_master:
					write("master" + user.file_repr()
				write(user.file_repr())
	except Exception as e:
		print(e)
		pass
		
def notify_master(message):
	if glob_master == None:
		return
	try:
		bot.sendMessage(glob_master.chat_id, message)
	except Exception as e:
		bot.sendMessage(glob_master.chat_id, "Failed to send a notification")

#==============================================================================#

#===============================UTILITY FUNCTIONS===================================#
def time_from_unix_int(time):
	return datetime.fromtimestamp(time).strftime('%H:%M')

def date_from_unix_int(time):
	stri = datetime.fromtimestamp(time).strftime('%d. ')
	words = ["Januar","Februar","März","April","Mai","Juni","Juli","August","September","Oktober","November","Dezember"]
	stri += words[int(datetime.fromtimestamp(time).strftime('%m'))-1]
	return stri

def convert_video():
	try:
		os.remove('/home/pi/film.mp4')
	except OSError:
		pass
		
	cmd = ['MP4Box', '-add', '/home/pi/film.h264', '/home/pi/film.mp4']
	check_call(cmd)
	
	try:
		os.remove('/home/pi/film.h264')
	except OSError:
		pass
		
def capture_picture():
	IR_led.on()
	camera.start_preview()
	time.sleep(2)
	camera.vflip = True
	camera.hflip = True
	camera.capture('/home/pi/bild.jpg')
	camera.stop_preview()
	IR_led.off()
	
def capture_movie(seconds):
	IR_led.on()
	camera.start_preview()
	time.sleep(1)
	camera.vflip = True
	camera.hflip = True
	camera.start_recording('/home/pi/film.h264')
	time.sleep(seconds)
	camera.stop_recording()
	camera.stop_preview()
	IR_led.off()
	convert_video()
#=========================================================================#

#=================================ARDUINO RECEIVE FUNCTIONS================================#	
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
						bot.sendMessage(glob_master.id, line, reply_markup = fehler_keyboard)
						glob_master.job = "manual"
				elif letter == 'P':
					line = "*Nachricht:*		"
					line += e
					if config.master_chat_id != None:
						if e == "tstop":
							notify_master("Türchen auf zeit geschlossen")
						else:
							notify_master(line)
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
#=========================================================================#

#encodes strings and sends them to the arduino.
def send_to_arduino(tosend):
	#serial lock to make sure only one thresad is on the communication with the arduino at a time.
	with serial_lock:
		print(" => Sent to Arduino: "+tosend)
		ser.write((tosend).encode()+ b'\n')
					
# def cache_user(user):
	# with file_lock:
		# with open(userfile_path, 'a') as f:
			# f.write(str(user))
	
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

#=================================HANDLE FUNCTIONS========================================#
#The handle Function is called by the telepot thread, 
#whenever a message is received from Telegram
def handle(msg):
	global serial_lock
	chat_id = msg['chat']['id']
	command = msg['text']
	print()
	print('------------------------------------------')
	user = find_user_from_chat_id(chat_id)
	print( 'Command Received: %s' % command)
	if "Zurück" in command:
		user.job_done()
		bot.sendMessage(user.id, "Beendet", reply_markup = user.keyboard)
	if user.job == "login":
		login_handler(user, command)
	elif user.job == "manual":
		manual_mode_handler(user, command)
	elif user == glob_master:
		master_handler(user, command)
	elif user.status == "admin":
		admin_handler(user, command)
	elif user.status == "user":
		user_handler(user, command)
	elif user.status == "guest":
		guest_handler(user, command)
	print("Command Processed.")
	print('------------------------------------------')
	
#The handle Function is called by the telepot thread, 
#whenever a message is received from Telegram

		
		
		
def master_handler(user, command):
	if "approve" in command:
		scom = command.split(" ")
		chat_id = int(re.search(r'\d+', scom[1]).group())
		sel_user = find_user_from_chat_id(chat_id)
		new_status = scom[2]
		sel_user.change_status(new_status)
	elif "lspeople" in command:
		bot.sendMessage(user.id,'\U0000000A'.join([str(u) for u in glob_user_list]))
	elif "name" in command:
		scom = command.split(" ")
		chat_id = int(re.search(r'\d+', scom[1]).group())
		sel_user = find_user_from_chat_id(chat_id)
		new_name = scom[2]
		sel_user.change_name(new_name)
	elif 'Logfile' in command:
		with file_lock:
			with open(logfile_path, 'rb') as file:
				bot.sendDocument(chat_id, file)
	elif 'Tempfile' in command:
		with file_lock:
			with open(celsiusfile_path, 'rb') as file:
				bot.sendDocument(chat_id, file)
	elif 'Userfile' in command:
		pass
	elif 'deb-en' in command:
		send_to_arduino("s10")
	elif 'deb-stop' in command:
		send_to_arduino("s11")
	elif 'Erweitern' in command:
		user.keyboard = master_extended_keyb
		bot.sendMessage(user.id, "Tastatur Erweitert", reply_markup = user.keyboard)
	elif 'kleiner' in command:
		user.keyboard = master_keyboard
		bot.sendMessage(user.id, "Tastatur verkleinert", reply_markup = user.keyboard)
	else:
		admin_handler(user, command)
		
def admin_handler(user, command):
	if "manuell" in command:
		user.job = "manual"
	elif 'Neustart' in command:
		pass
	else:
		user_handler(user, command)
		
def user_handler(user, command):
	if user.is_on_cooldown():
		return
	
	elif 'Schliessen' in command:				
		send_to_arduino("s03")
		bot.sendMessage(chat_id, 'Türchen schliesst.')
	elif 'Öffnen' in command:				
		send_to_arduino("s02")
		bot.sendMessage(chat_id, 'Türchen öffnet') 
	elif 'Zeit Aktualisieren' in command:				
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
	else:
		guest_handler(user ,command)
		
def guest_handler(user, command):
	if user.is_on_cooldown():
		return
	elif 'Bild' in command:
		send_picture(user)
	elif 'Film' in command:
		try:
			record_time = int(re.search(r'\d+', command).group())
		except Exception as e:
			bot.sendMessage(user.id, "Filmlänge in sekunden angeben.")
			return
		send_movie(user, record_time)
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
	elif 'Temparatur' in command:
		with serial_lock:
			while ser.in_waiting > 0:
				log_message(ser.readline().decode('utf-8').strip('\n'),write = True)
			send_to_arduino("s09")
			line = ser.readline().decode('utf-8').strip('\n')
			bot.sendMessage(chat_id, log_message(line), parse_mode = 'Markdown')
	elif 'Login' in command:
		user.job = "login"
		bot.sendMessage()

def manual_mode_handler(user, command):
	elif 'Fehlerlog' in command:
		send_to_arduino("s21")
	elif 'Aufrollen' in command:
		send_to_arduino("s22")
		bot.sendMessage(user.id, "Rolle Auf")
	elif 'Abrollen' in command:
		send_to_arduino("s23")
		bot.sendMessage(user.id, "Rolle Ab")
	elif 'Stop' in command:
		send_to_arduino("s24")
		bot.sendMessage(user.id, "Motor Stop")
	elif 'Warte +60 min' in command:
		send_to_arduino("s25")
		bot.sendMessage(user.id, "Arduino Wartet")

def send_picture(user):
	if user.is_on_cooldown() or user.is_on_cooldown(): #subtract two points for a pic
		return
	try:
		capture_picture()
		with open(os.path.join(config.workingPath,'/home/pi/bild.jpg'), 'rb') as photo: #oeffne das bild nur wenn es gebraucht wird
			bot.sendPhoto(user.id, photo)
	except Exception as e:	#falls es fehlgeschlagen ist, melde das.
		print( '----failed to send or capture picture---' )
		print(e)
		print( '----------------------------------------' )
		bot.sendMessage(user.id, "Bild oeffnen oder senden fehlgeschlagen.")
		
def send_movie(user, seconds):
	if user.is_on_cooldown() or user.is_on_cooldown() or user.is_on_cooldown():
		return
	try:
		if seconds > 20 or seconds == None or seconds < 0:
			bot.sendMessage(chat_id, "Max 20 sek.")
			seconds = 20
		capture_movie(seconds)
		with open('/home/pi/film.mp4', 'rb') as film:            
			bot.sendVideo(user.id, film)
	except Exception as e:	#falls es fehlgeschlagen ist, melde das.
		print( '----failed to send or capture Video---' )
		print(e)
		print( '----------------------------------------' )
		bot.sendMessage(user.id, "Film oeffnen oder senden fehlgeschlagen.")


#=====================================================================================#

#================================START PREPARATIONS====================================#
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
		lines = f.readlines()
		for line in lines:
			line.strip('\n')
			line.strip('\t')
			cols = line.split(';')
			try:
				id = int(re.search(r'\d+', cols[0]).group())
				n_user = User(id)
				n_user.change_status(cols[1])
				if len(cols) == 3:
					n_user.change_name(cols[2])
				if "master" in cols[0]:
					glob_master = n_user
				glob_user_list.append(n_user)
			except Exception as e:
				print(e)
				pass
except IOError:
	f = open(userfile_path, 'w')
	f.close()
#===================================================================================#


#==================================START SEQUENCE=======================================#
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
send_to_arduino("s08")  #tell arduino to refresh satellites

#====================================================================================#

#==================================RUNTIME CODE=======================================#
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
#=======================================================================================#
		
		
