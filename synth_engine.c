
// synth engine main code

// MIDI to key frequency step table
// A4-pitch = 440.0 Hz
uint16_t keystep[128] = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 
0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 107 , 114 , 120 , 127 , 135 , 143 , 152 , 161 , 
170 , 180 , 191 , 202 , 214 , 227 , 241 , 255 , 270 , 286 , 303 , 321 , 340 , 360 , 382 , 405 , 
429 , 454 , 481 , 510 , 540 , 572 , 606 , 642 , 680 , 721 , 764 , 809 , 857 , 908 , 962 , 1020 , 
1080 , 1144 , 1212 , 1284 , 1361 , 1442 , 1528 , 1618 , 1715 , 1817 , 1925 , 2039 , 2160 , 2289 , 2425 , 2569 , 
2722 , 2884 , 3055 , 3237 , 3429 , 3633 , 3849 , 4078 , 4320 , 4577 , 4850 , 5138 , 5443 , 5767 , 6110 , 6473 , 
6858 , 7266 , 7698 , 8156 , 8641 , 9155 , 9699 , 10276 , 10887 , 11534 , 12220 , 12947 , 0 , 0 , 0 , 0 , 
0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 };

#define SYN_AUDIOBUF_SIZE 64	// size of audio output buffer
#define POLYPHONY 4			// number of voices played in parallell

// Audio data output buffer, filled in here and read from main timer ISR
struct ringbuf audiobuf_str; 

// the actual data buffer
uint16_t audiobuf_data[SYN_AUDIOBUF_SIZE];	


// sound engine data structures
struct osc_set_str {
	// uint8_t midi_key;	 // midi mey number (if any)
        uint16_t cur_pos1;       // current position in wave (1)
        uint16_t step1;          // oscillator forward step (1)
        uint16_t cur_pos2;       // current position in wave (2)
        uint16_t step2;          // oscillator forward step (2)
};
struct osc_set_str voice[POLYPHONY];

// sound engine control structures
struct osc_control_str {
	volatile uint8_t volume;	// play volume
	volatile uint8_t key_velocity;	// key velocity
};
struct osc_control_str voice_ctrl[POLYPHONY];

// global controller store:
struct global_ctrl_str {
	volatile uint8_t modwheel;
	volatile uint8_t volume;
} global_ctrl ;

volatile uint8_t midi_key[POLYPHONY];	// MIDI key to voice generator map

// initialize a voice oscillator
void key_start_play(uint8_t key_n, uint8_t key_v) {
	uint8_t i=0;
	for ( i=0 ; i < POLYPHONY ; i++ ) {
		if ( midi_key[i] == 0 || midi_key[i] == key_n ) {
			voice_ctrl[i].volume=0;		// keep it quiet for now
			voice[i].cur_pos1=0;		// initial start point = 0
			voice[i].step1=keystep[key_n];	// load step value
			voice[i].cur_pos2=0;		// initial start point = 0
			voice[i].step2=keystep[key_n+1];	// load step value
			midi_key[i]=key_n;
			// load initial player volume
			voice_ctrl[i].volume=100;
			break;
		}
	}
}

void key_stop_play(uint8_t key_n) {
	uint8_t i=0;
	for ( i=0 ; i < POLYPHONY ; i++ ) {
		if ( midi_key[i] == key_n ) {
			midi_key[i]=0;
			voice_ctrl[i].volume=0;	// will stop it playing ASAP.
			voice[i].step1=0;
			voice[i].step2=0;
			voice[i].cur_pos1=0;
			voice[i].cur_pos2=0;
		}
	}
}

void midi_set_ctrl(uint8_t ctrl_no, uint8_t value) {
	
	switch ( ctrl_no ) {
		case 0x01:	// mod wheel
			global_ctrl.modwheel = value;
			break;
		case 0x07:	// volume
			global_ctrl.volume = value;
			break;
	}
}

// main synth sound rendering function
void render_sound () {
	
	uint8_t voice_index=0;	// voice processor index
	int16_t waveproc=0;	// temp processing storage
	int16_t wavemix = 2047; // output signal; set to "zero level" initially.

	uint16_t waveptr;       // wave pointer (16bit) temp data

	uint8_t maxframes=16; 	// render frame counter; render will pause when 0.
	
	while(is_writeable(&audiobuf_str)) {

		PORTD |= (1<<PD5);
		for ( voice_index=0; voice_index < POLYPHONY ; voice_index++ ) {
			if ( voice_ctrl[voice_index].volume != 0) {	// OK, actually play this one
				// copy current pointer
				// waveptr = voice[voice_index].cur_pos1;
				
				// step forward & store for next time
				voice[voice_index].cur_pos1 += voice[voice_index].step1;

				// copy current pointer
				// waveptr = voice[voice_index].cur_pos2;
				
				// step forward & store for next time
				voice[voice_index].cur_pos2 += voice[voice_index].step2;
				
				// scale down pointer to 8 bits
				// waveptr >>= 8;
				
				// effectively use wave ptr as saw wave source
				// waveproc=(int16_t)(127-waveptr);

				if ( voice[voice_index].cur_pos1 & 0x8000 ) {
					waveproc = 63; 
				} else {
					waveproc = -63;
				}
				if ( voice[voice_index].cur_pos1 & 0x2000 ) {
					waveproc += (global_ctrl.modwheel/2); 
				} else {
					waveproc -= (global_ctrl.modwheel/2);
				}
				
// 				if ( waveptr & 0x80 ) {
// 					waveproc = -127;
// 				} else {
// 					waveproc = 127;
// 				}
				waveproc *= voice_ctrl[voice_index].volume;
				waveproc >>= 8;
			
				wavemix += waveproc;
			}
		}
		ATOMIC_BLOCK(ATOMIC_FORCEON) {
			write_word(&audiobuf_str,&audiobuf_data[0],wavemix);
		}
		PORTD &= ~(1<<PD5);
		maxframes--;
		if (maxframes == 0) {
			break;
		}
	}
	
}

