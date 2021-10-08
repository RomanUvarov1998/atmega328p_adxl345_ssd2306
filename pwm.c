#include "pwm.h"
#include <avr\io.h>

void pwm_init(void) {
    // write 0x07 to WGMx[2:0] to make timer count from bottom to OCRxn
    
    // for PD3: OC2B
    TCCR2A |= _BV(WGM20) | _BV(WGM21); //TCCR2B |= _BV(WGM22);
    OCR2B = 0;
    DDRD |= _BV(DDD3);
    PORTD &= ~_BV(PORTD3);
    TCCR2B |= _BV(COM2B1); // set on BOTTON, clear on compare match OCR0B
    
    // for PD5: OC0B
    TCCR0A |= _BV(WGM00) | _BV(WGM01); //TCCR0B |= _BV(WGM02);
    OCR0B = 100;
    DDRD |= _BV(DDD5);
    PORTD &= ~_BV(PORTD5);
    TCCR0B |= _BV(COM0B1); // set on BOTTON, clear on compare match OCR0B
        
    // for PD6: OC0A
    //TCCR0A |= _BV(WGM00) | _BV(WGM01); //TCCR0B |= _BV(WGM02);
    OCR0A = 100;
    DDRD |= _BV(DDD6);
    PORTD &= ~_BV(PORTD6);
    TCCR0A |= _BV(COM0A1); // set on BOTTON, clear on compare match OCR0B
    
    TCCR2B |= _BV(CS20) | _BV(CS22);
    TCCR0B |= _BV(CS00) | _BV(CS02);
}

void pwm_set_channel_brightness(enum PWM_Channels channel, uint8_t value) {
    switch (channel) {
        case PWM_Ch_0: OCR2B = value; break;
        case PWM_Ch_1: OCR0B = value; break;
        case PWM_Ch_2: OCR0A = value; break;
    }
}