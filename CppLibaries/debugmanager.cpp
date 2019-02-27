#include <debugmanager.h>
#include <Arduino.h>
#include <Timelib.h>
DebugManager::DebugManager(HardwareSerial* _link): link(_link), active(false){}

void DebugManager::activate(){
	active = true;
}
void DebugManager::stop(){
	active = false;
}
void DebugManager::dprintln(){
	if (active)
		this->link->println();
}
template <typename T> void DebugManager::dprint(T a){
	if(active)
		this->link->print(a);
}
template <typename T> void DebugManager::dprintln(T a){
	if(active)
		this->link->println(a);
}
template <typename X, typename Y> void DebugManager::dprint(X a, Y b){
	if(active)
		this->link->print(a, b);
}
template <typename X, typename Y> void DebugManager::dprintln(X a, Y b){
	if(active)
		this->link->println(a, b);
}