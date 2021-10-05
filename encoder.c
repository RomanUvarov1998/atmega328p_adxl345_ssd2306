#include "encoder.h"

enum InterruptTrigger {
    IT_Low = 0x00,
    IT_Change = 0x01,
    IT_FallingEdge = 0x10,
    IT_RisingEdge = 0x11,
};

void enc_init(void) {    
    // external interrupt 0 is on PD2
    // another pin is on PD3
    DDRD &= ~(_BV(DDD2) | _BV(DDD3)); // both are inputs
    PORTD |= _BV(PORTD2) | _BV(PORTD3); // both are pulled high
    
    // external interrupt 0 is on falling edge
    EICRA = IT_Change;
    
    // enable external interrupt 0
    #define EXT_INT_0 0x01
    EIMSK |= EXT_INT_0;
}

static volatile uint8_t enc_value = 0;
static volatile bool new_enc_value_is_read = false;

bool enc_value_updated(void) {
    return !new_enc_value_is_read;
}
uint8_t get_enc_value(void) {
    new_enc_value_is_read = true;
    return enc_value;
}

#define D2_IS_HIGH ((PIND & _BV(PIND2)) > 0)
#define D3_IS_HIGH ((PIND & _BV(PIND3)) > 0)
ISR(INT0_vect) {
    if (!D2_IS_HIGH) {
        if (D3_IS_HIGH) {
            --enc_value;
        } else {
            ++enc_value;
        }
        new_enc_value_is_read = false;
    }
}