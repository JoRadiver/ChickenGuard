#ifndef DEBUG_MANAGER_HEADER
#define DEBUG_MANAGER_HEADER

#include "Arduino.h"
class DebugManager{
public:
	DebugManager(HardwareSerial* _link);
	void activate();
	void stop();
	void dprintln();
	template <typename T> void dprint(T a);
	template <typename T> void dprintln(T a);
	template <typename X, typename Y> void dprint(X a, Y b);
	template <typename X, typename Y> void dprintln(X a, Y b);
	bool active;
private:
	HardwareSerial* link;
};
#endif