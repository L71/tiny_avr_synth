
// MIDI I/O controller


// MIDI state defines
#define MIDI_STATUS_NONE                0

#define MIDI_STATUS_KEYON               1
#define MIDI_STATUS_KEYOFF              2
#define MIDI_WAIT_FOR_BYTE1             3
#define MIDI_WAIT_FOR_BYTE2             4
#define MIDI_STATUS_CTRL		5
#define MIDI_STATUS_UNIMPLEMENTED       255

// MIDI command codes (channel info stripped; bits 4-7 only)
#define MIDI_KEYOFF_MSG                 0x80
#define MIDI_KEYON_MSG                  0x90
#define MIDI_AFTERTOUCH                 0xa0
#define MIDI_CTRL_MSG                   0xb0
#define MIDI_PRG_CHANGE                 0xc0
#define MIDI_CHAN_PRESSURE              0xd0
#define MIDI_PITCH_WHEEL                0xe0

// Sound event codes; internal events
#define MIDI_GOT_KEYON                  50
#define MIDI_GOT_KEYOFF                 51
#define MIDI_GOT_OTHER                  52
#define MIDI_GOT_CTRL			53
#define MIDI_INCOMPLETE                 54


#define MIDI_BUF_SIZE 64	// size of midi buffer

// Audio data output buffer, filled in here and read from main timer ISR
struct ringbuf midibuf_str; 
// the actual data buffer
uint8_t midibuf_data[MIDI_BUF_SIZE];


// USART0 HW setup ...
void midi_hw_init() {
        // configure USART0 for send/recieve
        // recieve setup
        DDRD &= ~(1<<PD0);
        // UCSR0A = ... // only polled  
        UCSR0B |= (0<<UCSZ02) | (1<<RXCIE0) | (1<<RXEN0) ;
        UCSR0C |= (0<<UMSEL01) | (0<<UMSEL00) | (0<<UPM01) | (0<<UPM00) | (0<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00) | (0<<UCPOL0);

        UBRR0H = 0x00;  // baud rate high byte
        UBRR0L = 40;	// baud rate low byte
}

ISR(USART0_RX_vect) {
	// This ISRs only job is to feed the MIDI in buffer whenever a byte has arrived
	write_byte(&midibuf_str,&midibuf_data[0],UDR0);
}


void process_midi_buffer() {
	// MIDI global state keeper:
	static uint8_t midi_main_state;
	static uint8_t midi_sub_state;
	uint8_t key_state = MIDI_GOT_OTHER;

	static uint8_t key_no;
	static uint8_t key_vel;
//	uint8_t i;
	uint8_t last_in;
// 	uint8_t last_in=uart_in;
	
	uint8_t maxframes=8;	// max bytes to process in one go.
	
	while (is_readable(&midibuf_str)) {
		ATOMIC_BLOCK(ATOMIC_FORCEON) {
			last_in=read_byte(&midibuf_str,&midibuf_data[0]);
		}
		maxframes -- ;
// 		if ( last_in != 0xfe ) {  // MIDI event counter. Skip "active sensing" messages (if any).
// 			midi_event_counter ++;
// 		}

		if ( last_in & 0x80 ) {	// If this is a status message... :
			switch ( last_in & 0xf0 ) {	// which voice/channel message is it?
				case MIDI_KEYON_MSG :	// Key On
					midi_main_state = MIDI_STATUS_KEYON;
					midi_sub_state = MIDI_WAIT_FOR_BYTE1;
					key_state = MIDI_INCOMPLETE;
					break;
				case MIDI_KEYOFF_MSG :	// Key Off
					midi_main_state = MIDI_STATUS_KEYOFF;
					midi_sub_state = MIDI_WAIT_FOR_BYTE1;
					key_state = MIDI_INCOMPLETE;
					break;
				case MIDI_PITCH_WHEEL :	// MIDI pitch wheel
				case MIDI_AFTERTOUCH :	// MIDI aftertouch
				case MIDI_CTRL_MSG :	// MIDI controller
					midi_main_state = MIDI_STATUS_CTRL;
					midi_sub_state = MIDI_WAIT_FOR_BYTE1;
					key_state = MIDI_INCOMPLETE;
					break;
				case MIDI_PRG_CHANGE :	// MIDI program change
				case MIDI_CHAN_PRESSURE :	// MIDI channel pressure
					midi_main_state = MIDI_STATUS_UNIMPLEMENTED ;
					key_state = MIDI_INCOMPLETE ;
					break;
				case 0xf0 :	// catch-all for system messages
					switch ( last_in & 0xf8 ) {
						case 0xf8 :	// 0xf8-0xff  system realtime msg, don't cancel any running status
							key_state = MIDI_INCOMPLETE ;
							break ;
						case 0xf0 :	// 0xf0-0xf7  system common msg, reset running status etc.
							midi_main_state = MIDI_STATUS_UNIMPLEMENTED ;
							key_state = MIDI_INCOMPLETE ;
							break ;
					}
			}
		}
		else	// no, other type of byte
		{
			switch ( midi_sub_state ) {
				case MIDI_WAIT_FOR_BYTE1 :
					key_no = last_in ;
					midi_sub_state = MIDI_WAIT_FOR_BYTE2 ;
					key_state = MIDI_INCOMPLETE ;
					break;
				case MIDI_WAIT_FOR_BYTE2 :
					key_vel = last_in ;
					midi_sub_state = MIDI_WAIT_FOR_BYTE1 ;
					// OK, here we should have a complete note on/off command
					if ( midi_main_state == MIDI_STATUS_KEYON ) { // key on
						key_state = MIDI_GOT_KEYON ;
					}
					if ( midi_main_state == MIDI_STATUS_KEYOFF ) { // key off
						key_state = MIDI_GOT_KEYOFF ;
					}
					if ( midi_main_state == MIDI_STATUS_KEYON && key_vel == 0 ) {
						// key on with velocity=0 is really a key off
						key_state = MIDI_GOT_KEYOFF ;
					}
					if ( midi_main_state == MIDI_STATUS_CTRL ) { // ctrl msg
						key_state = MIDI_GOT_CTRL ;
					}
					break;

			}
		}

		switch ( key_state ) {
			case MIDI_GOT_KEYON :
				// midi_state = MIDI_WAIT_NEW_EVENT;
				key_start_play(key_no,key_vel);
// 				for ( i=0 ; i <= 3 ; i++ ) {
// 					if ( midi_key[i] == 0 || midi_key[i] == key_no ) {
// 						// voice[i].key=key_no;
// 						midi_key[i]=key_no;
// 						voice[i].step=pgm_read_word(&(keystep[key_no]));
// 						// voice[i].step=keystep[key];
// 						voice[i].cur_pos=0;
// 						// voice[i].wave = 0;
// 						// voice[i].step=keystep[key];
// 						voice[i].play=key_vel;  // play must be activated last!
// 						break;
// 					}
// 				}
				key_no = 0;
				key_vel = 0;
				// midi_state = MIDI_WAIT_NEW_EVENT;
				// midi_state = MIDI_WAIT_FOR_KEYON_NO;
				break;

			case MIDI_GOT_KEYOFF :
				// midi_state = MIDI_WAIT_NEW_EVENT;
				key_stop_play(key_no);
// 				for ( i=0 ; i <= 3 ; i++ ) {
// 					if ( midi_key[i] == key_no ) {
// 						voice[i].play = 0; // play attr. must be cleared first!
// 						voice[i].step = 0;
// 						midi_key[i]=0;
// 						// voice[i].key = 0;
// 					}
// 				}
				key_no = 0;
				key_vel = 0;
				// midi_state = MIDI_WAIT_NEW_EVENT;
				// midi_state = MIDI_WAIT_FOR_KEYOFF_NO;
				break;
			case MIDI_GOT_CTRL :
				midi_set_ctrl(key_no,key_vel);	// not really key_no etc, but you get the idea... :)
				break;
		}
		key_state = MIDI_GOT_OTHER;
		if ( maxframes == 0 ) {
			break;
		}
	}
}


