#ifndef DATEN_HEADER
#define DATEN_HEADER
#include <Arduino.h>

struct Zeiten{
	//Timers
	unsigned long loop_zeit = 0;
	unsigned long GPS_wecker = 0;
	unsigned long display_wecker = 0;
	unsigned long PIreport_wecker = 0;
	unsigned long Standard_wecker = 0;
	unsigned long Step_cooldown = 0;  //This one is in Milliseconds, carefull
	unsigned long Toor_stop_wecker = 0; //This one tells the dc motor to stop if no sensor is touched.
	volatile bool interrupt = false;

	//Sonnenzeiten
	unsigned long Sonnenaufgang;
	unsigned long Sonnenuntergang;
	byte Tageszeit;
	
	
	void adjustTimes(long diff){
		loop_zeit += diff;
		GPS_wecker += diff;
		display_wecker += diff;
		PIreport_wecker += diff;
		Standard_wecker += diff;
		Sonnenaufgang += diff;
		Sonnenuntergang += diff;
		Toor_stop_wecker += diff;
	}
};

//Speichert den soll zustand
struct Zustand{
	byte toorstatus; //0 geschlossen, 1 offen, 2, schliessend, 3 öffnend
	bool zaunstatus;
	bool lichtstatus;
	//bool blitz;
};
//Trennung weil speicherplatz knapp ist...
//Speichert den ist zustand
struct ExtendedZustand: public Zustand{
	ExtendedZustand(): unterer_endschalter(0), oberer_endschalter(0){}
	bool unterer_endschalter;
	bool oberer_endschalter;
	float temparatur;
	bool fehler;
};



#endif

