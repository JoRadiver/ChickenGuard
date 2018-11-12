#ifndef DEBUG_MANAGER_HEADER
#define DEBUG_MANAGER_HEADER

#include "Arduino.h"
class DebugManager{
public:
	DebugManager(HardwareSerial* _link);
	void activate(){
		active = true;
	}
	void stop(){
		active = false;
	}
	void dprintln(){
		if (active)
			this->link->println();
	}
	template <typename T> void dprint(T a){
		if(active)
			this->link->print(a);
	}
	template <typename T> void dprintln(T a){
		if(active)
			this->link->println(a);
	}
	template <typename X, typename Y> void dprint(X a, Y b){
		if(active)
			this->link->print(a, b);
	}
	template <typename X, typename Y> void dprintln(X a, Y b){
		if(active)
			this->link->println(a, b);
	}
	void dprint_time(unsigned long time);
	void dprint_date(unsigned long time);
	void dprint_time_span(unsigned long time);
	bool active;
private:
	HardwareSerial* link;
};
#endif