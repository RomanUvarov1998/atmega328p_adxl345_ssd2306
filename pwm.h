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

    void pwm_init(void);

    enum PWM_Channels {
        PWM_Ch_0 = 0,
        PWM_Ch_1 = 1,
        PWM_Ch_2 = 2,
    };
    void pwm_set_channel_brightness(enum PWM_Channels channel, uint8_t value);

#ifdef	__cplusplus
}
#endif

#endif	/* PWM_H */

