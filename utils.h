/* 
 * File:   utils.h
 * Author: Роман
 *
 * Created on 5 октября 2021 г., 8:11
 */

#ifndef UTILS_H
#define	UTILS_H

#ifdef	__cplusplus
extern "C" {
#endif

    #include <avr/io.h>
    #include <util/delay.h>

    enum ErrKind {
        ERR_TWI_CMD_BUF_OVERFLOW = 5,
        ERR_TWI_BUS_ERR = 6
    };

    void assert(bool expr) {
        if (expr) 
            return;
        
        DDRB |= _BV(DDB5);
        while (1) {
            PORTB |= _BV(PORTB5);
            _delay_ms(300);
            PORTB &= ~_BV(PORTB5);
            _delay_ms(300);
        }
    }

#ifdef	__cplusplus
}
#endif

#endif	/* UTILS_H */

