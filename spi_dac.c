
void spi_dac_uart1_setup() {
	
	// setup UART1 as SPI master for DAC output
	// will use PD2 as output pin for chip enable (active low)..
	// TXD1 (PD3) is data output
	// XCK1	(PD4) is SPI clock

	// DAC chip enable bit mgmt:
	DDRD |= (1<<PD2);	// PD2 = output
	PORTD |= (1<<PD2);	// PD2 = high, don't select DAC intially
	
	// zero out data rate reg:
	// UBRR1 = 0;
        UBRR0H = 0x00;  // baud rate high byte
        UBRR0L = 0x00;	// baud rate low byte
        
	// set pin XCK1 as output; same with TXD1
	DDRD |= (1<<PD4)|(1<<PD3);

	// MSPI mode & SPI mode setup. 
	UCSR1C = (1<<UMSEL11)|(1<<UMSEL10)|(1<<UCPHA1)|(1<<UCPOL1);
	
	// Enable SPI transmitter (skip reciever - we'll use that IO bit for CE flag.
	// UCSR1B = (1<<TXEN1);
	UCSR1B = 0b00001000;
	
	// set baud rate
	// UBRR1 = 2;

        UBRR0H = 0x00;  // baud rate high byte
        UBRR0L = 0;	// baud rate low byte
	
}


void inline spi_dac_write(const uint16_t spiword) { 

	// send a 16-bit word to the SPI DAC
	
	// first raise PD2 to make DAC output previous data
	PORTD |= (1<<PD2);
	
	// take apart input word
	uint8_t dac_lsbyte = (uint8_t) spiword & 0x00ff;
	uint8_t dac_msbyte = (uint8_t) (spiword >> 8) & 0x0f;

	// PD2 -> low transition to enable transfer on DAC end.
	PORTD &= ~(1<<PD2);
	
	// add appropriate MCP49x1 control bits to the first 4 bits in the 16 bit data word
	dac_msbyte |= 0x70; 
	
	// send data word! This works due to double buffering in the AVR UARTs 
	UDR1 = dac_msbyte;
	UDR1 = dac_lsbyte;
	
	// done. raising PD2 to actually set the DAC output is done next time.
	// This way we don't have to wait for it here or do interrupts :)
}

