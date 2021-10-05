#include "encoder.h"

enum InterruptTrigger {
    IT_Low = 0x00,
    IT_Change = 0x01,
    IT_FallingEdge = 0x10,
    IT_RisingEdge = 0x11,
};

static volatile uint8_t enc_value = 0;
static volatile bool new_enc_value_is_read = false;
static volatile bool btn_is_pressed = false;

void enc_init(void) {  
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

ISR(PCINT2_vect) {
    const uint32_t FAST = 10;
    const uint32_t SLOW = 1;    
    if (!D3_IS_HIGH) {
        uint8_t change_speed = btn_is_pressed ? FAST : SLOW;
        if (D2_IS_HIGH) {
            enc_value -= change_speed;
        } else {
            enc_value += change_speed;
        }
        new_enc_value_is_read = false;
    }
}

//
// ------------- button --------------------
//
#define B0_IS_HIGH ((PINB & _BV(PINB0)) > 0)

ISR(PCINT0_vect) {
    btn_is_pressed = !B0_IS_HIGH;
}
