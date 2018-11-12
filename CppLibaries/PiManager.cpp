#include <PiManager.h>
#include <Arduino.h>
#include <TimeLib.h>

PiManager::PiManager(HardwareSerial* _pi, Zeiten* zt, ExtendedZustand* _ist, Zustand* _soll, DebugManager* _deb): pi(_pi), zeiten(zt), ist(_ist), soll(_soll), deb(_deb){}


const char N_ERROR = 'X'
const char N_QUICK = 'Q'
const char N_TOOR = 'T'
const char N_ZAUN = 'Z'
const char N_LICHT = 'L'
const char N_TEMP = 'T'
const char NU_AKTUELL = 'a'
const char NU_SONNENAUFGANG = 'r'
const char NU_SONNENUNTERGANG = 's'

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
	pi->println();
	if( TEMP_AKTIV ){
		printTemp();
	}
	if( ZEIT_AKTIV ){
		printZeit();
	}
}

void PiManager::printToor(){
	pi->print(N_TOOR);
	pi->print(ist->toorstatus);
	pi->print(';');
}
void PiManager::printZaun(){
	pi->print(N_ZAUN);
	pi->print(ist->zaunstatus)
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
	pi->print(N_UHRZEIT);	pi->print(NU_AKTUELL);  pi_print(zeiten->loop_zeit); pi->print(';');
	pi->print(N_UHRZEIT); pi->print(NU_SONNENAUFGANG); pi_print(zeiten->Sonnenaufgang); pi->print(';');
	pi->print(N_UHRZEIT); pi->print(NU_SONNENUNTERGANG); pi->print(zeiten->Sonnenuntergang); pi->print(';');
}

void PiManager::quick_report(char reason, String message){
	pi->print(reason);
	pi->print(message);
	pi->println(';');
}



void PiManager::handleInput(){
	if(pi->read() != 's'){
		String cache = '';
		while(pi->available()){
			String += pi->read();
		}
		quick_report(N_ERROR, cache)
		return;
	}
	switch(pi->read()){
		case '0':	//First Set
			switch(pi->read()){
				case '1': //Log
					this->log();
					return;
				case '2': //OpenDoor
					soll->toorstatus = 1;
					zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
					zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
					return;
				case '3': //CloseDoor
					soll->toorstatus = 0;
					zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
					zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
					return;
				case '4': //Fence On
					soll->zaunstatus = 1;
					zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
					zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
					return;
				case '5': //Fence Off
					soll->zaunstatus = 0;
					zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
					zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
					return;
				case '6': //Light On
					soll->lichtstatus = 1;
					zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
					zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
					return;
				case '7': //Light Off
					soll->lichtstatus = 0;
					zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
					zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
					return;
				case '8': //Refresh
					zeiten->GPS_wecker = zeiten->loop_zeit;
					zeiten->Standard_wecker = zeiten->loop_zeit;
					return;
				case '9': //Temparature
					printTemp()
					pi->println()
					return;
				default:
					quick_report(N_ERROR, "M")
					return;
			}
			return;
		case '1':
			switch(pi->read()){
				case '0':
					deb->activate();
					return;
				case '1':
					deb->stop();
					return;
				default:
					quick_report(N_ERROR, "M")
					return;
			}
			return;
		default:
			quick_report(N_ERROR, "M")
			return;
	}
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
