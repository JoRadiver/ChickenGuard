#include <extendedoutputs.h>
#include <Arduino.h>

ExtendedOutput::ExtendedOutput(int _latch, int _data, int _clock):data_pin(_data), clock_pin(_clock), latch_pin(_latch){}


void ExtendedOutput::setStep(bool dir){
	//Serial.print("sS");
	if(!dir){
		this->next_step_watcher = 1;
	}else if (current_step != 0){
		this->next_step_watcher = -1;
	}
	
	for(int i = 0; i<4; i++){
		digitalSet(i+4, this->steps[(current_step+next_step_watcher)%8] & (1<<i));
	}
}
void ExtendedOutput::step(bool dir){
	//Serial.print("s");
	setStep(dir);
	this->push();
}
void ExtendedOutput::digital_Write(unsigned int pin, bool value){
	//Serial.print("dW");
	digitalSet(pin, value);
	/*
	Serial.print("PinN: ");
	Serial.println(pin);
	for(auto x: out){
		Serial.print(x);
	}*/
	push();
}
void ExtendedOutput::digitalSet(unsigned int pin, bool value){
	//Serial.print("dS");
	if (pin > 7 ) {
		return;
	}
	//Serial.print('p');Serial.print(pin);
	//Serial.print('v');Serial.println(value);
	/*
	for(int x = 0; x<8 ; x++){
		out[x] = value;
	}*/
	out[pin] = value;
}


void ExtendedOutput::push(){
	this->current_step += next_step_watcher;
	next_step_watcher = 0;
	digitalWrite(this->latch_pin, 1);
	for(int i = 0; i<8; i++){
		digitalWrite(this->data_pin, out[i]);
		digitalWrite(this->clock_pin, 0);
		digitalWrite(this->clock_pin, 1);
	}
	digitalWrite(this->latch_pin, 1);
	digitalWrite(this->clock_pin, 0);
	digitalWrite(this->latch_pin, 0);
}
