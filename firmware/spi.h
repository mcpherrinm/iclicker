#ifndef SPI_H_
#define SPI_H_

#define WRITE 0b10000000
#define READ  0b10100000

///Enables spi communication, 1 for start, 0 for stop
void spi_enable(unsigned char enable) {
	change_bit(SPIPORT,SPICS,!enable);
}

void clockpulse(unsigned char rising) {
	change_bit(SPIPORT,SPICLOCK,!rising);
	_delay_us(100);
	//Switch here!
	
	change_bit(SPIPORT,SPICLOCK,rising);
	_delay_us(100);
}

void spi_send(unsigned char data) {
	for(signed char bit=7;bit>=0;bit--) {
		change_bit(SPIPORT,SPIDOUT,test_bit(data,bit));
		clockpulse(0);
	}
}

unsigned char spi_recieve(void) {
	unsigned char data=0;
	for(signed char bit=7;bit>=0;bit--) {
		clockpulse(0);
		change_bit(data,bit,test_bit(SPIPIN,SPIDIN));
	}
	return data;
}

void spi_write(unsigned char reg, unsigned char data) {
	spi_enable(1);
	
	//send register to write
	spi_send(WRITE|reg);
	
	//send data to write in register
	spi_send(data);
	
	spi_enable(0);
}

unsigned char spi_read(unsigned char reg) {
	//Read something
	spi_enable(1);
	
	//send register to read
	spi_send(READ|reg);
	
	//read the data
	unsigned char data=spi_recieve();
	spi_enable(0);
	return data;
}

#endif
