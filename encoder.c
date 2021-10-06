#include "encoder.h"
#include "utils.h"

enum InterruptTrigger {
    IT_Low = 0x00,
    IT_Change = 0x01,
    IT_FallingEdge = 0x10,
    IT_RisingEdge = 0x11,
};

static volatile uint8_t enc_value = 1;
static volatile bool new_enc_value_is_read = false;
static volatile bool btn_is_pressed = false;

static volatile ValueChangeCbk value_change_cbk = 0;

void enc_init(ValueChangeCbk _value_change_cbk) {  
    value_change_cbk = _value_change_cbk;
    
    /*======== button ==============*/
    // external interrupt 0 is on PB0
    DDRB &= ~_BV(DDB0); // is input
    PORTB |= _BV(PORTB0); // is pulled high
    
    PCICR |= _BV(PCIE0); // pin change interrupt for PB0-PB5 enabled
    PCMSK0 |= (1 << 0); // PB0 - button
    
    /*======== rotating handle ==============*/
    // external interrupt 1 is on PD3
    // another pin is on PD4
    DDRD &= ~(_BV(DDD2) | _BV(DDD3)); // both are inputs
    PORTD |= _BV(PORTD2) | _BV(PORTD3); // both are pulled high
    
    PCICR |= _BV(PCIE2); // pin change interrupt for PD0-PD7 enabled
    PCMSK2 |= (1 << 3); // PD3 - button
}

bool enc_value_updated(void) {
    return !new_enc_value_is_read;
}
uint8_t get_enc_value(void) {
    new_enc_value_is_read = true;
    return enc_value;
}

//
// ------------- rotated handle --------------------
//
#define D2_IS_HIGH ((PIND & _BV(PIND2)) > 0)
#define D3_IS_HIGH ((PIND & _BV(PIND3)) > 0)

static volatile bool prev_D3_is_high = true;

ISR(PCINT2_vect) {     
    if (prev_D3_is_high && !D3_IS_HIGH) {
        enum EncChangeDirection dir = D2_IS_HIGH ? ECD_Inc : ECD_Dec;
        hang_if_not(value_change_cbk != 0);
        enc_value = value_change_cbk(enc_value, btn_is_pressed, dir);
        new_enc_value_is_read = false;
    }
    prev_D3_is_high = D3_IS_HIGH;
}

//
// ------------- button --------------------
//
#define B0_IS_HIGH ((PINB & _BV(PINB0)) > 0)

ISR(PCINT0_vect) {
    btn_is_pressed = !B0_IS_HIGH;
}
