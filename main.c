/*
 * File:   main.c
 * Author: Роман
 *
 * Created on 1 октября 2021 г., 19:52
 */

#include "main.h"

uint8_t map_int16_to_uint8(int16_t value);
static uint8_t enc_value_changed(uint8_t prev_value, bool btn_is_pressed, enum EncChangeDirection dir);
static uint8_t acc_data_resolution = 0;

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
    adxl345_set_resolution(DR_PlusMinus_2g);
    pwm_init_pin(PWM_Pin_PD6, 0);
    pwm_init_pin(PWM_Pin_PD5, 0);
    pwm_init_pin(PWM_Pin_PD3, 0);
    
    lcd_clear();
    
    lcd_set_cursor_pos(0, 0);
    lcd_draw_text("SENS: ... ADC: ");
    
    lcd_set_cursor_pos(1, 0);
    lcd_draw_text("X: ");
    lcd_set_cursor_pos(2, 0);
    lcd_draw_text("Y: ");
    lcd_set_cursor_pos(3, 0);
    lcd_draw_text("Z: ");
    
    hang_if_not(map_int16_to_uint8(0xAAFF) == 0x55);
    
    for (;;) {
        if (enc_value_updated()) {
            lcd_set_cursor_pos(0, 6);
            acc_data_resolution = get_enc_value();
            lcd_draw_uint8(acc_data_resolution);
        }
        
        adxl345_set_resolution(acc_data_resolution);
    
        /*
        uint16_t value = adc_scan_channel_uint16(ADCCH_0);
        value *= adc_amp_coeff;
        lcd_set_cursor_pos(0, 15);
        lcd_draw_uint16(value);
         */
        
        if (adxl345_has_unread_data()) {            
            struct AccValues acc = adxl345_get_XYZ_data();
            
            uint8_t x = map_int16_to_uint8(acc.x_mg);
            uint8_t y = map_int16_to_uint8(acc.y_mg);
            uint8_t z = map_int16_to_uint8(acc.z_mg);
            
            lcd_set_cursor_pos(1, 3);
            lcd_draw_int16((int16_t)x);
            lcd_set_cursor_pos(2, 3);
            lcd_draw_int16((int16_t)y);
            lcd_set_cursor_pos(3, 3);
            lcd_draw_int16((int16_t)z);
            
            pwm_set_pin_brightness(PWM_Pin_PD6, x);
            pwm_set_pin_brightness(PWM_Pin_PD5, y);
            pwm_set_pin_brightness(PWM_Pin_PD3, z);
        }
        
        _delay_ms(10); // 100 Hz
    }    
    
    return 0;
}

#define ENC_VALUE_STEP_SMALL 1
#define ENC_VALUE_STEP_BIG 3
#define ENC_VALUE_MAX 3
#define ENC_VALUE_MIN 0
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

uint8_t map_int16_to_uint8(int16_t value) {
    if (value < 0)
        value -= value;
    
    value >>= 4;
    
    uint8_t value8 = (uint8_t)value;
    return value8;
    
    // remove sign 
    uint16_t value_16 = (uint16_t)value, msk = 0x7FFF;
    value_16 &= msk;   
    // got 15 bit positve value
    // result = x * x^8 / 2^15 = x / 2^7
    value_16 >>= 7;
    // truncate
    uint8_t value_8 = (uint8_t)value_16;
    return value_8;
}

ISR(PCINT0_vect) {    
    enc_process_PCINT0_ISR();
    adxl345_process_PCINT0_ISR();
}
