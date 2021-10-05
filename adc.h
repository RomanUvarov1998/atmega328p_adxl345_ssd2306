/* 
 * File:   adc.h
 * Author: Роман
 *
 * Created on 4 октября 2021 г., 8:54
 */

#ifndef ADC_H
#define	ADC_H

#ifdef	__cplusplus
extern "C" {
#endif

    #include <stdint.h>
    
    void adc_init(void);
    
    enum ADC_Channel {
        ADCCH_0 = 0,
        ADCCH_1,
        ADCCH_2,
        ADCCH_3,
        ADCCH_4,
        ADCCH_5,
    };
    uint8_t adc_scan_channel(enum ADC_Channel channel);

#ifdef	__cplusplus
}
#endif

#endif	/* ADC_H */

