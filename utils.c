#include "utils.h"

void hang_if_not(bool expr) {
    if (expr) 
        return;

    DDRB |= _BV(DDB5);
    while (1) {
        PORTB |= _BV(PORTB5);
        _delay_ms(100);
        PORTB &= ~_BV(PORTB5);
        _delay_ms(100);
    }
}

void blink(void) {
    DDRB |= _BV(DDB5);
    PORTB |= _BV(PORTB5);
    _delay_ms(300);
    PORTB &= ~_BV(PORTB5);
    _delay_ms(300);
}

uint8_t to_uint8(int16_t value) {
    if (value < 0)
        value = -value;
    
    uint16_t pos_value = (uint16_t)value;
    
    return pos_value;
}

