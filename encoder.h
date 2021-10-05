/* 
 * File:   encoder.h
 * Author: Роман
 *
 * Created on 5 октября 2021 г., 1:20
 */

#ifndef ENCODER_H
#define	ENCODER_H

#ifdef	__cplusplus
extern "C" {
#endif

    #include <stdint.h>
    #include <avr/io.h>
    #include <stdbool.h>
    #include <avr/interrupt.h>

    void enc_init(void);
    bool enc_value_updated(void);
    uint8_t get_enc_value(void);

#ifdef	__cplusplus
}
#endif

#endif	/* ENCODER_H */

