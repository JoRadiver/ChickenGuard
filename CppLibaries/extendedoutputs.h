#ifndef EXTENDED_OUTPUTS_HEADER
#define EXTENDED_OUTPUTS_HEADER
#include <Arduino.h>

#ifndef STEPS_NEEDED
#define STEPS_NEEDED 53379
#endif

class ExtendedOutput{
public:
	ExtendedOutput(int _latch, int _data, int _clock);
	//POST:: Pin is immediately set. requires some time
	void digital_Write(unsigned int pin, bool value);
	//POST:: sets a pin in the sendbyte. Only has an effect when it is sent, by caling push()
	void digitalSet(unsigned int pin, bool value);
	//POST:: sends the outbyte	
	void push();
	//POST:: Sets sendbyte for the stepper to do a step in the given direction. only executed when send is called
	void setStep(bool dir);
	//POST:: Steper motor instantly does a step in asked direction
	void step(bool dir);

	unsigned long current_step = 0;
	unsigned int steps_needed = STEPS_NEEDED;
private:
	bool out[8];
	int next_step_watcher;
	const int data_pin;
	const int clock_pin;
	const int latch_pin;
	//Stepper Motor
	const byte steps[8] = {0b1001,0b0001,0b0011,0b0010,0b0110,0b0100,0b1100,0b1000};
	
	
};
#endif





