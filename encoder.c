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
    
    /*======== rotating handle ==============*/
    // PB0 - pin with interrupt
    // PB1 - pin withthout interrupt, for comparison
    DDRB &= ~(_BV(DDB0) | _BV(DDB1)); // are input
    PORTB |= _BV(PORTB0) | _BV(PORTB1); // are pulled high
    
    PCICR |= _BV(PCIE0); // pin change interrupt for PB0-PB5 enabled
    PCMSK0 |= _BV(PCINT0); // PB0
    
    /*======== button ==============*/
    // PB2 - pin with interrupt
    DDRB &= ~_BV(DDB2); // is input
    PORTB |= _BV(PORTB2); // is pulled high
    
    PCICR |= _BV(PCIE0); // pin change interrupt for PB0-PB5 enabled
    PCMSK0 |= _BV(PCINT2); // PB2
}

bool enc_value_updated(void) {
    return !new_enc_value_is_read;
}
uint8_t get_enc_value(void) {
    new_enc_value_is_read = true;
    return enc_value;
}

#define B0_IS_HIGH ((PINB & _BV(PINB0)) > 0)
#define B1_IS_HIGH ((PINB & _BV(PINB1)) > 0)
static volatile bool B0_was_high = true;

#define B2_IS_HIGH ((PINB & _BV(PINB2)) > 0)

ISR(PCINT0_vect) {    
    /*======== rotating handle ==============*/
    if (B0_was_high && !B0_IS_HIGH) {
        enum EncChangeDirection dir = B1_IS_HIGH ? ECD_Dec : ECD_Inc;
        hang_if_not(value_change_cbk != 0);
        enc_value = value_change_cbk(enc_value, btn_is_pressed, dir);
        new_enc_value_is_read = false;
    }
    B0_was_high = B0_IS_HIGH;
    
    /*======== button ==============*/
    btn_is_pressed = !B2_IS_HIGH;
}
