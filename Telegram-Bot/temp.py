def master_handler(user, command):
	if "approve" in command:
		chat_id = 0
		ap_user = find_user_from_chat_id(chat_id)
		new_status = "guest"
		ap_user.change_status(new_status)
	elif "lspeople" in command:
		bot.sendMessage(user.chat_id,'\U0000000A'.join([str(u.chat_id) for u in glob_user_list]))
	else:
		admin_handler(user, command)
		
def admin_handler(user, command):
	if "manual" in command:
		user.job = "manual"
		manual_mode_handler(user, command)
	else:
		user_handler(user, command)
		
def user_handler(user, command):
	if user.is_on_cooldown():
		return
	else:
		guest_handler(user ,command)
		
def guest_handler(user, command):
	if user.is_on_cooldown():
		return
	pass

def login_handler(user, command):
	if user.status = "admin":
		user.job_done()
		return True
	password = ['0','0','0','0','0']
	if command = password[user.jobstep]:
		user.jobstep += 1
		if user.jobstep == len(password)-1:
			return True
	else:
		user.jobstep *= -1
		user.jobstep -= 1
		if user.jobstep == -(len(password)-1): #only notify user of false code when full password is given.
			bot.sendMessage(user.chat_id, "Login Failed: Bad password") 
			user.job_done()
	return False
	
		
def manual_mode_handler(user, command):
	pass
def capture_picture():
	pass
def capture_movie(seconds):
	pass
def send_picture(user):
	if user.is_on_cooldown() or user.is_on_cooldown(): #subtract two points for a pic
		return
	pass
def send_movie(user, seconds):
	if user.is_on_cooldown() or user.is_on_cooldown() or user.is_on_cooldown():
		return
	pass

def notify_master(message):
	if glob_master == None:
		return
	try:
		bot.sendMessage(glob_master.chat_id, message)
	except Exception as e:
		bot.sendMessage(glob_master.chat_id, "Failed to send a notification")
		
glob_user_list = []
glob_master = None

def find_user_from_chat_id(chat_id, noappend = False):
	for user in glob_user_list:
		if user.chat_id == chat_id:
			return user
	else:
		
		new_user = User(chat_id, "stranger")
		if len(glob_user_list) > 99:
			return new_user #too many users. will not be appended
		else:
			notify_master("New User: " + str(chat_id))
			glob_user_list.append(new_user)
		

		
	
BURST_PER_HOUR = 60
class User:
	def __init__(self, chat_id, status = "guest"):
		self.chat_id = chat_id
		self.user_id = None
		self.status = status #stranger, guest, user, admin
		self.job = "none" #none, login, manual
		self.jobstep = 0 #some jobs require count of steps
		self.lastactive = 0 #when was user last active
		self.burst = BURST_PER_HOUR #how many orders may be processed in a short time
		
	def is_on_cooldown(self):
		if status == "admin":
			return False #Admin has no cooldown
		self.burst -= 1
		if self.burst < -10:
			self.burst = -10
		self.restore_points()
		if burst == 0: 
			bot.sendMessage(self.chat_id, "Zu viele nachrichten. Warte 10 min")
		return burst <= 0
		
	def restore_points(self):
		self.burst += (now()-self.lastactive)/3600 * BURST_PER_HOUR
		if self.burst > BURST_PER_HOUR:
			self.burst = BURST_PER_HOUR
			
	def now(): #pseudo replaced by time function
		return 0
		
	def change_status(self, new_status):
		self.status = new_status
	def set_job(self, job):
		if self.job == "none":	
			self.job = job
			return True
		else:
			return False
			
	def job_done(self):
		self.job = "none"
		self.jobstep = 0