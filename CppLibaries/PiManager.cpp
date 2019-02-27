#include <PiManager.h>
#include <Arduino.h>
#include <TimeLib.h>

PiManager::PiManager(HardwareSerial* _pi, Zeiten* zt, ExtendedZustand* _ist, Zustand* _soll, DebugManager* _deb): pi(_pi), zeiten(zt), ist(_ist), soll(_soll), deb(_deb){}



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
	pi->print(MessageSpecifier::door_state);
	pi->print(ist->toorstatus);
	pi->print(';');
}
void PiManager::printZaun(){
	pi->print(MessageSpecifier::fence_state);
	pi->print(ist->zaunstatus);
	pi->print(';');	
}
void PiManager::printLicht(){
	pi->print(MessageSpecifier::light_state);
	pi->print(ist->lichtstatus);
	pi->print(';');
}
void PiManager::printTemp(){
	pi->print(MessageSpecifier::temparature);
	pi->print(ist->temparatur);
	pi->print(';');
}
void PiManager::printZeit(){
	pi->print(MessageSpecifier::system_time);  pi->print(zeiten->loop_zeit); pi->print(';');
	pi->print(MessageSpecifier::next_sunrise_time); pi->print(zeiten->Sonnenaufgang); pi->print(';');
	pi->print(MessageSpecifier::next_sunset_time); pi->print(zeiten->Sonnenuntergang); pi->print(';');
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
		if (pi->available() < 4){ //shortest command is 4 bytes
			return 0; //To few bytes available, lets return to the program until we have new bytes
		}
		pi->read(); //Skim of any bytes which had no starting character
	}
	pi->read(); //consume the starting character
	byte received_command = pi->read()*10 + pi->read();
	switch(received_command){
		case CommandName::log:
			this->log();
			break;
		case CommandName::open_door:
			soll->toorstatus = 1;
			zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			break;
		case CommandName::close_door:
			soll->toorstatus = 0;
			zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			break;
		case CommandName::fence_on:
			soll->zaunstatus = on;
			zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			break;
		case CommandName::fence_off:
			soll->zaunstatus = off;
			zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			break;
		case CommandName::light_on:
			soll->lichtstatus = on;
			zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			break;
		case CommandName::light_off:
			soll->lichtstatus = off;
			zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
			break;
		case CommandName::refresh_all:
			zeiten->GPS_wecker = zeiten->loop_zeit;
			zeiten->Standard_wecker = zeiten->loop_zeit;
			break;
		case CommandName::temparature:
			printTemp();
			pi->println();
			break;
		case CommandName::activate_debugging:
			deb->activate();
			break;
		case CommandName::deactivate_debugging:
			deb->stop();
			break;
		case CommandName::jump_to_manual_mode:
			return -1; //return 1 to indicate manual mode
			break;
		case CommandName::respond_if_ready:
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
