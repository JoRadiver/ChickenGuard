#include <debugmanager.h>
#include <Arduino.h>
#include <Timelib.h>
DebugManager::DebugManager(HardwareSerial* _link): link(_link), active(false){}


// void DebugManager::stopDebug(){
	// active = false;
// }
// void DebugManager::startDebug(){
	// active = true;
// }
// void DebugManager::dprint(int& a){
	// if(this->active)
		// link->print(a);
// }
// void DebugManager::dprint(long int& a){
	// if(this->active)
		// link->print(a);
// }
// void DebugManager::dprint(int a){
	// if(this->active)
		// link->print(a);
// }
// void DebugManager::dprint(long unsigned int& a){
	// if(this->active)
		// link->print(a);
// }

// void DebugManager::dprint(float f){
	// if(this->active)
		// link->print(f);
// }
// void DebugManager::dprint(float f, unsigned int n){
	// if(this->active)
		// link->print(f, n);
// }
// void DebugManager::dprint(char c){
	// if(this->active)
		// link->print(c);
// }
// void DebugManager::dprint(const String s){
	// if(this->active)
		// link->print(s);
// }

// void DebugManager::dprintln(){
	// if(this->active)
		// link->println();
// }
// void DebugManager::dprintln(int& a){
	// if(this->active)
		// link->println(a);
// }
// void DebugManager::dprintln(long int& a){
	// if(this->active)
		// link->println(a);
// }
// void DebugManager::dprintln(long unsigned int& a){
	// if(this->active)
		// link->print(a);
// }
// void DebugManager::dprintln(float f){
	// if(this->active)
		// link->println(f);
// }
// void DebugManager::dprintln(float f, unsigned int n){
	// if(this->active)
		// link->println(f, n);
// }
// void DebugManager::dprintln(char c){
	// if(this->active)
		// link->println(c);
// }
// void DebugManager::dprintln(const String s){
	// if(this->active)
		// link->println(s);
// }
void DebugManager::dprint_time(unsigned long time){
	if(!this->active) return;
	link->print(hour(time));
	link->print(":");
	link->print(minute(time));
	link->print(":");
	link->println(second(time));	
}
void DebugManager::dprint_date(unsigned long time){
	if(!this->active) return;
	link->print(day(time));
	link->print(".");
	link->print(month(time));
	link->print(".");
	link->println(year(time));
	
}
void DebugManager::dprint_time_span(unsigned long qtime){
	if(!this->active) return;
	link->print(year(qtime)-1970);
	link->print(" years ");
	link->print(month(qtime)-1);
	link->print(" month ");
	link->print(day(qtime)-1);
	link->print(" days ");
	link->print(hour(qtime));
	link->print(" hours ");
	link->print(minute(qtime));
	link->print(" minutes ");
	link->print(second(qtime));
	link->println(" seconds");
}