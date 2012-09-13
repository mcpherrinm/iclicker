#include <avr/io.h>
#include <avr/delay.h>
#include <bitop.h>

void main() {
	// Set the LEDs as outputs
	DDRD = 0xF0;

	// Latch power
	DDRB = 0x02;
	PORTB = 0x02;
	
	DDRC = 0x00;
	PORTC = 0b00111111;
	
	// Wait until the button is not pressed, prevents premature shutdown
	PORTD = 0x70;
	while(!(PINC & 0b00000001));
	
	while(1) {
		// Always keep the power LED on
		PORTD |= 0x70;

		if(!(PINC & 0b00000001)) {
			_delay_ms(50);
			if(!(PINC & 0b00000001)) {
				PORTB = 0x00;
			}
		} else {
			PORTD |= (PINC & 0b00110000) + (0b11 << 6) & (PINC << 4);
		}
	}
}
