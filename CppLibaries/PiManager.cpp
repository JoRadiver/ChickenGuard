#include <PiManager.h>
#include <Arduino.h>
#include <TimeLib.h>

PiManager::PiManager(HardwareSerial* _pi, Zeiten* zt, ExtendedZustand* _ist, Zustand* _soll, DebugManager* _deb): pi(_pi), zeiten(zt), ist(_ist), soll(_soll), deb(_deb){}


enum CommandType{
	log = 1;
	open_door = 2,
	close_door = 2,
	fence_on = 4,
	fence_off = 5,
	light_on = 6,
	light_off = 7,
	refresh_all = 8,
	temparature = 9,
	activate_debugging = 10,
	deactivate_debugging = 11,
	jump_to_manual_mode = 20,
	respond_if_ready = 99
}

enum MessageType{
	
}



const char N_ERROR = 'X';
const char N_QUICK = 'Q';
const char N_TOOR = 'T';
const char N_ZAUN = 'Z';
const char N_LICHT = 'L';
const char N_TEMP = 'C';
const char NU_AKTUELL = 'a';
const char NU_SONNENAUFGANG = 'r';
const char NU_SONNENUNTERGANG = 's';

void PiManager::log(){
	if( TOOR_AKTIV ){
		printToor();
	}
	if( ZAUN_AKTIV ){
		printZaun();
	}
	if( LICHT_AKTIV ){
		printLicht();
	}
	if( TEMP_AKTIV ){
		printTemp();
	}
	if( ZEIT_AKTIV ){
		printZeit();
	}
	pi->println();
}

void PiManager::printToor(){
	pi->print(N_TOOR);
	pi->print(ist->toorstatus);
	pi->print(';');
}
void PiManager::printZaun(){
	pi->print(N_ZAUN);
	pi->print(ist->zaunstatus);
	pi->print(';');	
}
void PiManager::printLicht(){
	pi->print(N_LICHT);
	pi->print(ist->lichtstatus);
	pi->print(';');
}
void PiManager::printTemp(){
	pi->print(N_TEMP);
	pi->print(ist->temparatur);
	pi->print(';');
}
void PiManager::printZeit(){
	pi->print(NU_AKTUELL);  pi->print(zeiten->loop_zeit); pi->print(';');
	pi->print(NU_SONNENAUFGANG); pi->print(zeiten->Sonnenaufgang); pi->print(';');
	pi->print(NU_SONNENUNTERGANG); pi->print(zeiten->Sonnenuntergang); pi->print(';');
}

void PiManager::quick_report(char reason, String message){
	pi->print(reason);
	pi->print(message);
	pi->println(';');
}

//Reads 4 bytes.
//Looks in serial for 's' char
//returns 0 if its a normal command
//returns  a nmuber when its a special command like manual mode
int PiManager::handleInput(){
	//We will loop until we find a starting 's' character
	while (pi->peek() != 's'){
		if (pi->available() < 4){
			return 0; //To few bytes available, lets return to the program until we have new bytes
		}
		pi->read(); //Skim of any bytes which had no starting character
	}
	pi->read(); //consume the starting character
	CommandType received_command = pi->read()*10 + pi->read();
	switch(received_command){
		case log:
			this->log();
			break;
		case open_door:
			soll->toorstatus = 1;
			zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			break;
		case close_door:
			soll->toorstatus = 0;
			zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			break;
		case fence_on:
			soll->zaunstatus = 1;
			zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			break;
		case fence_off:
			soll->zaunstatus = 0;
			zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			break;
		case light_on:
			soll->lichtstatus = 1;
			zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			break;
		case light_off:
			soll->lichtstatus = 0;
			zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			break;
		case refresh_all:
			zeiten->GPS_wecker = zeiten->loop_zeit;
			zeiten->Standard_wecker = zeiten->loop_zeit;
			break;
		case temparature:
			printTemp();
			pi->println();
			break;
		case activate_debugging:
			deb->activate();
			break;
		case deactivate_debugging:
			deb->stop();
			break;
		case jump_to_manual_mode:
			return -1; //return 1 to indicate manual mode
			break;
		case respond_if_ready:
			pi->print('S'); //Start Letter
			pi->print('R'); //Ready
			pi->print(';'); 
			break;
		default:
			quick_report(N_ERROR, "M");
	}
	while(pi->available() && pi->peek() != 's'){
		pi->read();//skimm of any excess that is sent before the next s char is sent. 's' char must remain on the serial.
	}
	return 0;
}

void PiManager::sendTime(unsigned long time){
	pi->print(hour(time), DEC);
	pi->print(':');
	pi->print(minute(time), DEC);
	pi->print(':');
	pi->print(second(time), DEC);
}

//Takes an unsigned integer representing a Unix Time
//POST::Serial prints the number as human readeable time and date
void PiManager::sendDate(unsigned long time){
	pi->print(day(time), DEC);
	pi->print('.');
	pi->print(month(time));
	pi->print('.');
	pi->print(year(time));
}

//Takes an unsigned integer representing a unix time span
//POST::Serial prints the number as human readeable time span
void PiManager::sendTimeSpan(unsigned long span){
	bool upper = false;
	if(year(span) > 1970){
		upper = true;
		pi->print(year(span));
		pi->print('J');
	}
	if(month(span) > 1||upper){
		upper = true;
		pi->print(month(span));
		pi->print('M');
	}
	if(day(span) > 1||upper){
		upper = true;
		pi->print(day(span));
		pi->print('T');
	}
	if(hour(span) > 0||upper){
		upper = true;
		pi->print(hour(span));
		pi->print('S');
	}
	if(minute(span) > 0||upper){
		upper = true;
		pi->print(minute(span));
		pi->print('m');
	}
	if(second(span) > 0||upper){
		upper = true;
		pi->print(second(span));
		pi->print('s');
	}
}
