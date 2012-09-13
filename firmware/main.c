#include <avr/io.h>
#include <avr/delay.h>

#include "bitop.h"


#define BUTTONPIN PINC
#define BPOWER 0
#define BA     1
#define BB     2
#define BC     3
#define BD     5
#define BE     4
#define switchpressed(switch) (!test_bit(BUTTONPIN,switch))

#define LEDPORT PORTD
#define LEDBATTERY 4
#define LEDVOTE1 5
#define LEDVOTE2 6
#define LEDPOWER 7
#define change_led(led,val) (change_bit(LEDPORT,led,!val))

//PORTB
#define VCCLATCH 1

void ioinit(void) {
	//Buttons - PINC
	DDRB=0; //Set everything input
	PORTB=(1<<BPOWER)|(1<<BA)|(1<<BB)|(1<<BC)|(1<<BD)|(1<<BE); //pullups
	
	//LEDS - LEDPORT
	DDRB=(1<<LEDBATTERY)|(1<<LEDVOTE1)|(1<<LEDVOTE2)|(1<<LEDPOWER);
	
	//PORTB
	DDRB=(1<<VCCLATCH);
	PORTB=(1<<VCCLATCH);
	
	//Wait till power button is depressed
	while(switchpressed(BPOWER));
}

#define DEBOUNCE 50 //ms
unsigned char checkbutton(unsigned char button) {
	unsigned char counter=0;
	while(switchpressed(button)) {
		if(counter<255)
			counter++;
		_delay_ms(1);
	}
	return counter>DEBOUNCE;
}

int main() {
	ioinit();
	change_bit(LEDPORT,LEDPOWER,1);
	
	while(1) {
		if(checkbutton(BPOWER)) {
			clear_bit(PORTB,VCCLATCH);
		}
		
		change_led(LEDVOTE1,switchpressed(BA));
	}
}
