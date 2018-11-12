#include <PiManager.h>
#include <Arduino.h>
#include <TimeLib.h>

PiManager::PiManager(HardwareSerial* _pi, Zeiten* zt, ExtendedZustand* _ist, Zustand* _soll, DebugManager* _deb): pi(_pi), zeiten(zt), ist(_ist), soll(_soll), deb(_deb){}

void PiManager::log(){
	pi->println("LOG"); //Scheduled log
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
	pi->println(F("EOM"));
}

void PiManager::printToor(){
	pi->print(F("Türchen: "));
	if( ist->toorstatus == 0)
		pi->print(F("zu"));
	else if (ist->toorstatus == 1)
		pi->print(F("auf"));
	else if(ist->toorstatus == 2)
		pi->print(F("schliesst"));
	else if(ist->toorstatus == 3)
		pi->print(F("öffnet"));
		
}
void PiManager::printZaun(){
	pi->print(F("Zaun: "));
	if( ist->zaunstatus )
		pi->print(F("ein"));
	else
		pi->print(F("aus"));		
}
void PiManager::printLicht(){
	pi->print(F("Licht: "));
	if( ist->lichtstatus )
		pi->print(F("ein"));
	else
		pi->print(F("aus"));
}
void PiManager::printTemp(){
	pi->print(F("Temp: "));
	pi->print(ist->temparatur);
	pi->println(F("°C"));
}
void PiManager::printZeit(){
	pi->print(F("ZIT: "));  sendTime(zeiten->loop_zeit); pi->println();  
	pi->print(F("DAT: ")); sendDate(zeiten->loop_zeit); pi->println();
	pi->print(F("Sonne auf: ")); sendTime(zeiten->Sonnenaufgang); pi->println();
	pi->print(F("Sonne unt: ")); sendTime(zeiten->Sonnenuntergang); pi->println();
}

void PiManager::quick_report(String reason, String message){
	pi->println(reason);
	pi->println(message);
	pi->println(F("EOM"));
}

void PiManager::handleInput(){
	String received = pi->readStringUntil('\n');
	if( received == "LOG" ){
		this->log();
	}
	else if( received == "TEMP" ){
		printTemp();
	}
	else if( received == "DEB_EN" ){
		deb->activate();
	}
	else if( received == "DEB_DIS" ){
		deb->stop();
	}
	else if( received == "O_DOOR" ){
		soll->toorstatus = 1;
		zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
		zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
	}
	else if( received == "C_DOOR" ){
		soll->toorstatus = 0;
		zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
		zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
	}
	else if( received == "FEN_ON" ){
		soll->zaunstatus = 1;
		zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
		zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
	}
	else if( received == "FEN_OF" ){
		soll->zaunstatus = 0;
		zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
		zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
	}
	else if( received == "L_ON" ){
		soll->lichtstatus = 1;
		zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
		zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
	}
	else if( received == "L_OF" ){
		soll->lichtstatus = 0;
		zeiten->Standard_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
		zeiten->GPS_wecker = zeiten->loop_zeit + PI_OVERRIDE_TIME;
	}
	else if( received == "REFR"){
		zeiten->GPS_wecker = zeiten->loop_zeit;
		zeiten->Standard_wecker = zeiten->loop_zeit;
	}/*
	else if( received == "FLSH_ON"){
		soll->blitz = true;
	}
	else if(received == "FLSH_OF"){
		soll->blitz = false;
	}*/
	else{
		quick_report("UKN", received);
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
		pi->print("J ");
	}
	if(month(span) > 1||upper){
		upper = true;
		pi->print(month(span));
		pi->print("M ");
	}
	if(day(span) > 1||upper){
		upper = true;
		pi->print(day(span));
		pi->print("T ");
	}
	if(hour(span) > 0||upper){
		upper = true;
		pi->print(hour(span));
		pi->print("S ");
	}
	if(minute(span) > 0||upper){
		upper = true;
		pi->print(minute(span));
		pi->print("m ");
	}
	if(second(span) > 0||upper){
		upper = true;
		pi->print(second(span));
		pi->print("s ");
	}
}
