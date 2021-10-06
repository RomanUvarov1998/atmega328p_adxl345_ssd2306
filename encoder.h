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
    #define F_CPU 16000000
    #include <avr/io.h>
    #include <stdbool.h>
    #include <avr/interrupt.h>

    enum EncChangeDirection { ECD_Inc, ECD_Dec };
    typedef uint8_t (*ValueChangeCbk)(uint8_t prev_value, bool btn_is_pressed, enum EncChangeDirection dir);

    void enc_init(ValueChangeCbk _change_cbk);
    bool enc_value_updated(void);
    uint8_t get_enc_value(void);
    void enc_process_PCINT0_ISR(void);

#ifdef	__cplusplus
}
#endif

#endif	/* ENCODER_H */

