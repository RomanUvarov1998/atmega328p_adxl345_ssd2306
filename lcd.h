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
    void lcd_draw_uint8(uint8_t value);
    void lcd_draw_uint16(uint16_t value);
    void lcd_draw_int16(int16_t value);
    void lcd_set_cursor_pos(uint8_t row, uint8_t col);

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

