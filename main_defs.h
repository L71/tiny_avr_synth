
// global includes

// System wide clocking stuff

// MCU clock frequency
#define F_CPU 20000000UL

// interrupt timer divider
// F_CPU above is divided by this in timer1.
// Timer divider = 1000 -> 20KHz base playback sample rate
#define ISR_TIMER1_DIVIDER 1000


#include <stdint.h>

#include <avr/io.h>
#include <avr/cpufunc.h>
// #include <util/delay.h>
#include <util/atomic.h>
#include <avr/interrupt.h>
// #include <avr/pgmspace.h>

// utility delaying function
// void delayms(uint16_t millis) {
//   uint16_t loop;
//   while ( millis ) {
//     _delay_ms(1);
//     millis--;
//   }
// }





