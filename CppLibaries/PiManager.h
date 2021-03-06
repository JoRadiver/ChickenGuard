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


struct CommandName{
	enum{
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
	};
};

struct MessageSpecifier{
	enum{ //the message type specifies to the server what the info represents
		error = 'X',
		quick_message = 'Q',
		door_state = 'T',
		fence_state = 'Z',
		light_state = 'L',
		temparature = 'C',
		system_time = 'a',
		next_sunrise_time = 'r',
		next_sunset_time = 's'
	};
};

class PiManager{
public:
	PiManager(HardwareSerial* _pi, Zeiten* zt, ExtendedZustand* _ist, Zustand* _soll, DebugManager* _deb);
	void log();
	void quick_report(char reason, String message); //Reason is always a short Capital letters message. message can be any string.
	int handleInput();
private:
	void printToor();
	void printZaun();
	void printLicht();
	void printTemp();
	void printZeit();
	HardwareSerial *const pi;
	Zeiten *const zeiten;
	ExtendedZustand *const ist;
	Zustand *const soll;
	DebugManager *const deb;
	
};





#endif