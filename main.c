
// global parameters etc.
#include "main_defs.h"

// utility subsystems
#include "buffers.c"

// sound engine
#include "synth_engine.c"

// MIDI engine
#include "midi.c"

// close-to-hardware code
#include "spi_dac.c"
#include "main_isr.c"

int main() {

	// PD5 = output
	DDRD |= (1<<PD5);
	// PD7 = output too
	DDRD |= (1<<PD7);

	buffer_init(&audiobuf_str, SYN_AUDIOBUF_SIZE);
	buffer_init(&midibuf_str,MIDI_BUF_SIZE);
	

	// SPI DAC output go!
	spi_dac_uart1_setup();
	
	// MIDI HW init
	midi_hw_init();

	// start main ISR timer
	isr_timer1_setup();

	// All systems go!
	// global interrupt enable
	SREG |= 0x80 ;
	
	
// 	voice_start_play(0,57);
// 	voice_start_play(1,67);
// 	voice_start_play(2,35);
// 	voice_start_play(3,50);
	
// 	key_start_play(57,127);
// 	key_start_play(67,127);
// 	key_start_play(35,127);
// 	key_start_play(50,127);
	// main loop
	while ( 1 )  {
		render_sound();
		
		process_midi_buffer();
		
// 		PORTD |= (1<<PD5);
// 		// spi_dac_write(0x0755);
// 		if (is_writeable(&audiobuf_str)) {
// 			write_word(&audiobuf_str,&audiobuf_data[0],0x0000);
// 		}
// 		delayms(5);
// 		// spi_dac_write(0x0f55);
// 		if (is_writeable(&audiobuf_str)) {
// 			write_word(&audiobuf_str,&audiobuf_data[0],0x0fff);
// 		}
// 		PORTD &= ~(1<<PD5);
// 		delayms(5);

	}
  
}

