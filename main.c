/*
 * File:   main.c
 * Author: Роман
 *
 * Created on 1 октября 2021 г., 19:52
 */

#include "main.h"
    
int main(void) {    
    DDRB |= (1 << DDB5);
    PORTB &= ~(1 << PORTB5);
    
    twi_init();
    //adc_init();
    enc_init();
    ei();
    
    lcd_init();    
    
    lcd_clear();
    
    lcd_draw_text("COUNTER: ...");
    
    PORTB |= (1 << PORTB5);
    while (1) {
        if (enc_value_updated()) {
            lcd_set_cursor_pos(0, 9);
            lcd_draw_int(get_enc_value());
        }
        _delay_ms(16);
        //uint8_t value = adc_scan_channel(ADCCH_0);
        //lcd_draw_int(value);
    }    
    return 0;
}
