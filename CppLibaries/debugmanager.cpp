#include <debugmanager.h>
#include <Arduino.h>
#include <Timelib.h>
DebugManager::DebugManager(HardwareSerial* _link): link(_link), active(false){}
