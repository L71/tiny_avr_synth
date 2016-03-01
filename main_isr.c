
// main timer ISR setup 

void isr_timer1_setup() {

	// Setup "main" clock ISR on Timer1
	
	TCCR1A=0x00;	// no PWM or other fancy stuff
	// TCCR1A |= (1<<WGM12);	// enable clear-on-compare 
	
	TCCR1B=0x09;	// run timer at full CPU clock (20Mhz) (0x01) and enable clear-on-compare (0x08)
	
	TCNT1=0x0000;	// clear counter register
	OCR1A=ISR_TIMER1_DIVIDER;
	
	// finally, enable interrupt on compare match
	TIMSK1 |= (1<<OCIE1A);	// enable timer1a compare match interrupt

}


ISR(TIMER1_COMPA_vect) {
	// compare match timer1a interrupt

	// light up ISR blinkenlight
	PORTD |= (1<<PD7);

	// send audio buffer word to SPI DAC
	if (is_readable(&audiobuf_str)) {
		spi_dac_write(read_word(&audiobuf_str,&audiobuf_data[0]));
	}

	// turn off ISR blinkenlight
	PORTD &= ~(1<<PD7);
}

