/*
 * File:   main.c
 * Author: Роман
 *
 * Created on 1 октября 2021 г., 19:52
 */

#include "main.h"
    
int main(void) {      
    twi_init();
    ei();     
    lcd_init();  
    lcd_clear();
    lcd_set_cursor_pos(0, 0);
    lcd_draw_text("Init...");
    
    adc_init();
    enc_init(); 
    
    lcd_clear();
    
    lcd_set_cursor_pos(0, 0);
    lcd_draw_text("SENS: ...");
    
    for (;;) {
        if (enc_value_updated()) {
            lcd_set_cursor_pos(0, 6);
            lcd_draw_int(get_enc_value());
        }
        
        uint8_t value = adc_scan_channel(ADCCH_0);
        lcd_set_cursor_pos(1, 0);
        lcd_draw_int(value);
        
        _delay_ms(100);
    }    
    
    return 0;
}
