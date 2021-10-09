/* 
 * File:   pwm.h
 * Author: Роман
 *
 * Created on 8 октября 2021 г., 1:24
 */

#ifndef PWM_H
#define	PWM_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    #include <stdint.h>

    enum PWM_Pin {
        PWM_Pin_PD6 = 0, // OC0A
        PWM_Pin_PD5, // OC0B
        PWM_Pin_PB1, // OC1A
        PWM_Pin_PB2, // OC1B
        PWM_Pin_PB3, // OC2A
        PWM_Pin_PD3, // OC2B
        PWM_PINS_COUNT
    };
    void pwm_init_pin(enum PWM_Pin pin, uint8_t value);
    void pwm_set_pin_brightness(enum PWM_Pin pin, uint8_t value);

#ifdef	__cplusplus
}
#endif

#endif	/* PWM_H */

