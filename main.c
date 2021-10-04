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
    ei();
    
    lcd_init();    
    
    lcd_clear();
    //while (1);
    
    lcd_draw_text("HELLO, WORLD!");
    
    PORTB |= (1 << PORTB5);
    while (1);    
    return 0;
}
