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
    #define F_CPU 16000000
    #include <util/delay.h>
    #include <stdbool.h>
    
    enum ErrKind {
        ERR_TWI_CMD_BUF_OVERFLOW = 5,
        ERR_TWI_BUS_ERR = 6,
        ERR_COUNT
    };
    
    STATIC_ASSERT(ERR_COUNT)

    void hang_if_not(bool expr);
    void blink(void);

#ifdef	__cplusplus
}
#endif

#endif	/* UTILS_H */

