/* 
 * File:   adxl345.h
 * Author: Роман
 *
 * Created on 7 октября 2021 г., 3:47
 */

#ifndef ADXL345_H
#define	ADXL345_H

#ifdef	__cplusplus
extern "C" {
#endif

    #include <stdbool.h>
    #include <avr\io.h>
    #include "twi.h"
    #include "utils.h"
    
    struct AccValues {
        int16_t x_mg;
        int16_t y_mg;
        int16_t z_mg;
    };
    
    void adxl345_init(void);
    bool adxl345_has_unread_data(void);
    struct AccValues adxl345_get_XYZ_data(void);
    void adxl345_process_PCINT0_ISR(void);
    
    enum DataResolution {
        DR_PlusMinus_2g = 0x00,
        DR_PlusMinus_4g,
        DR_PlusMinus_8g,
        DR_PlusMinus_16g,
    };
    void adxl345_set_resolution(enum DataResolution res);

#ifdef	__cplusplus
}
#endif

#endif	/* ADXL345_H */

