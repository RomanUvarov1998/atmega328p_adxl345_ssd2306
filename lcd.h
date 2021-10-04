/* 
 * File:   lcd.h
 * Author: Роман
 *
 * Created on 3 октября 2021 г., 22:45
 */

#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif

    #include "main.h"
    
    void lcd_init();
    void lcd_clear(void);
    void lcd_draw_text(const char *const cstr);
    void lcd_draw_int(uint8_t value);

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

