/*
 * File:   main.c
 * Author: Роман
 *
 * Created on 1 октября 2021 г., 19:52
 */

#include "main.h"
    
#define ENC_VALUE_STEP_SMALL 1
#define ENC_VALUE_STEP_BIG 10
#define ENC_VALUE_MAX 64
#define ENC_VALUE_MIN 1
static uint8_t enc_value_changed(uint8_t prev_value, bool btn_is_pressed, enum EncChangeDirection dir) {
    uint8_t step = btn_is_pressed ? ENC_VALUE_STEP_BIG : ENC_VALUE_STEP_SMALL;    
    
    switch (dir) {
        case ECD_Dec: 
            if (prev_value < ENC_VALUE_MIN + step)
                prev_value = ENC_VALUE_MIN;
            else
                prev_value -= step; 
            break;
        
        case ECD_Inc: 
            if (prev_value + step > ENC_VALUE_MAX)
                prev_value = ENC_VALUE_MAX;
            else
                prev_value += step; 
            break;
    }
    
    return prev_value;
}

static uint8_t adc_amp_coeff = 0;

int main(void) {      
    // init display
    twi_init();
    ei();     
    lcd_init();  
       
    // show "Init..." on display
    lcd_clear();
    lcd_set_cursor_pos(0, 0);
    lcd_draw_text("Init...");
    
    // init all other peripherals
    adc_init();
    enc_init(&enc_value_changed); 
    adxl345_init();
    pwm_init();
    
    lcd_clear();
    
    lcd_set_cursor_pos(0, 0);
    lcd_draw_text("SENS: ... ADC: ");
    
    lcd_set_cursor_pos(1, 0);
    lcd_draw_text("X: ");
    lcd_set_cursor_pos(2, 0);
    lcd_draw_text("Y: ");
    lcd_set_cursor_pos(3, 0);
    lcd_draw_text("Z: ");
    
    for (;;) {
        if (enc_value_updated()) {
            lcd_set_cursor_pos(0, 6);
            adc_amp_coeff = get_enc_value();
            lcd_draw_uint8(adc_amp_coeff);
        }
        
        uint16_t value = adc_scan_channel_uint16(ADCCH_0);
        value *= adc_amp_coeff;
        lcd_set_cursor_pos(0, 15);
        lcd_draw_uint16(value);
        
        if (adxl345_has_unread_data()) {            
            struct AccValues acc = adxl345_get_XYZ_data();
            
            lcd_set_cursor_pos(1, 3);
            lcd_draw_int16(acc.x_mg);
            lcd_set_cursor_pos(2, 3);
            lcd_draw_int16(acc.y_mg);
            lcd_set_cursor_pos(3, 3);
            lcd_draw_int16(acc.z_mg);
            
            #define ABS(x) ((x) < 0 ? -(x) : (x))
            pwm_set_channel_brightness(PWM_Ch_0, (uint8_t)ABS(acc.x_mg >> 2));
            pwm_set_channel_brightness(PWM_Ch_1, (uint8_t)ABS(acc.y_mg >> 2));
            pwm_set_channel_brightness(PWM_Ch_2, (uint8_t)ABS(acc.z_mg >> 2));
        }
        
        _delay_ms(10); // 100 Hz
    }    
    
    return 0;
}

ISR(PCINT0_vect) {    
    enc_process_PCINT0_ISR();
    adxl345_process_PCINT0_ISR();
}
