#ifndef PI_MANAGER_HEADER
#define PI_MANAGER_HEADER
#include <Arduino.h>
#include <Daten.h>
#include <debugmanager.h>

#define TOOR_AKTIV 1
#define ZAUN_AKTIV 0
#define LICHT_AKTIV 0
#define TEMP_AKTIV 0
#define ZEIT_AKTIV 1

#ifndef PI_OVERRIDE_TIME
#define PI_OVERRIDE_TIME 1800    //30 minutes ovveride (30*60)
#endif




class PiManager{
public:
	PiManager(HardwareSerial* _pi, Zeiten* zt, ExtendedZustand* _ist, Zustand* _soll, DebugManager* _deb);
	void log();
	void quick_report(String reason);  //Reason is always a short Capital letters message.
	void quick_report(String reason, String message); //Reason is always a short Capital letters message. message can be any string.
	void handleInput();
private:
	void printToor();
	void printZaun();
	void printLicht();
	void printTemp();
	void printZeit();
	void sendTimeSpan(unsigned long span);
	void sendDate(unsigned long time);
	void sendTime(unsigned long time);
	HardwareSerial *const pi;
	Zeiten *const zeiten;
	ExtendedZustand *const ist;
	Zustand *const soll;
	DebugManager *const deb;
	
};





#endif