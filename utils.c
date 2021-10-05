#include "utils.h"

void hang_if_not(bool expr) {
    if (expr) 
        return;

    DDRB |= _BV(DDB5);
    while (1) {
        PORTB |= _BV(PORTB5);
        _delay_ms(300);
        PORTB &= ~_BV(PORTB5);
        _delay_ms(300);
    }
}
