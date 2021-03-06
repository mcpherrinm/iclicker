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
#define LEDVOTEGREEN 5
#define LEDVOTERED 6
#define LEDPOWER 7
#define change_led(led,val) (change_bit(LEDPORT,led,!val))

//PORTB
#define VCCLATCH 1

#define SPIPORT PORTB
#define SPIPIN PINB
#define SPICS 2
#define SPIDOUT 3
#define SPIDIN 4
#define SPICLOCK 5

#include "spi.h"

void display_byte(unsigned char data) {
	for(unsigned char i = 0; i < 8; i++) {
		unsigned char tmp = (data & 0x80);
		data <<= 1;
		change_led(LEDBATTERY,1);
		if(tmp) {
			_delay_ms(1000);
		} else {
			_delay_ms(250);
		}
				
		change_led(LEDBATTERY, 0);
		
		if(i == 7) {
			change_led(LEDVOTEGREEN, 1);
		}

		_delay_ms(250);
		change_led(LEDVOTEGREEN, 0);
	}
}

void ioinit(void) {
	//Buttons - PINC
	DDRC=0; //Set everything input
	PORTC=(1<<BPOWER)|(1<<BA)|(1<<BB)|(1<<BC)|(1<<BD)|(1<<BE); //pullups
	
	//LEDS - LEDPORT
	DDRD=(1<<LEDBATTERY)|(1<<LEDVOTEGREEN)|(1<<LEDVOTERED)|(1<<LEDPOWER);
	PORTD = DDRD;
	
	//PORTB
	DDRB=(1<<VCCLATCH)|(1<<SPICS)|(1<<SPIDOUT)|(1<<SPICLOCK);
	PORTB=(1<<VCCLATCH);
	spi_enable(0);
	
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
	change_led(LEDPOWER,1);
	
	while(1) {
		if(checkbutton(BPOWER)) {
			clear_bit(PORTB,VCCLATCH);
		}
		
		//Init
		spi_write(0b00000001, 0b10000101);
		spi_write(0b00000010, 0b00011111);
		spi_write(0b00000011, 0b11110000);
		spi_write(0b00000100, 0b00000000);
		spi_write(0b00000101, 0b00000000);
		spi_write(0b00001110, 0b11000001);
		spi_write(0b00001111, 0b00000000);
		spi_write(0b00010000, 0b00000000);
		spi_write(0b00010001, 0b00000000);
		spi_write(0b00010010, 0b00000000);
		spi_write(0b00001001, 0b10001000);
		spi_write(0b00000110, 0b00110000);
		
		//Frequency Registers
		spi_write(0b00000111, 0b00001111);
		spi_write(0b00001000, 0b10100000);
		spi_write(0b00001010, 0b10100111);
		spi_write(0b00001011, 0b00101000);

		//Pattern Registers
		spi_write(0b00010011, 0b11111101);
		spi_write(0b00010100, 0b11111101);
		spi_write(0b00010101, 0b11111101);
		spi_write(0b00010110, 0b11110001);
		
		//Wake the chip
		spi_write(0b00000110, 0b11110000);

		spi_write(0b00000001, 0b10000000);
		_delay_ms(1);		
		spi_read(0b00000001);
		_delay_ms(10);
//		display_byte(spi_read(0b00001101));
	}
}
