#include "adc.h"
#include "avr/io.h"

enum ADC_REFS {
    AR_AREF = 0x00,
    AR_AVcc_AREFcap = 0x01,
    AR_Internal_1_1V_AREFcap = 0x03
};

enum ADC_Prescaler {
    ADCPSC_2 = 0x01,
    ADCPSC_4 = 0x02,
    ADCPSC_8 = 0x03,
    ADCPSC_16 = 0x04,
    ADCPSC_32 = 0x05,
    ADCPSC_64 = 0x06,
    ADCPSC_128 = 0x07,
};

static void set_ADC_clk_prescaler(enum ADC_Prescaler psc) {
    uint8_t adcsra = ADCSRA;
    adcsra &= 0xF8;
    adcsra |= psc & 0x7F;
    ADCSRA = adcsra; 
}

void adc_init(void) {
    ADMUX &= ~(_BV(REFS1) | _BV(REFS0)); // AREF by default    
    ADMUX |= _BV(ADLAR); // left adjusted
    
    // Single conversion mode by default
    // No triggers are used
    
    set_ADC_clk_prescaler(ADCPSC_32);
} 

uint8_t adc_scan_channel(enum ADC_Channel channel) {
    while (ADCSRA & _BV(ADSC)); // wait for prev conversion end
    
    DIDR0 |= _BV(channel); // turn on power safe mode for pin
    
    uint8_t admux = ADMUX;
    admux &= 0xF0; // clear MUX bits
    admux |= channel & 0x0F;
    ADMUX = admux;
    
    ADCSRA |= _BV(ADSC) | _BV(ADEN); // start conversion
    while (ADCSRA & _BV(ADSC)); // wait for conversion end
           
    uint8_t result = ADCH;
    
    DIDR0 &= ~_BV(channel); // turn off power safe mode for pin
    ADCSRA &= ~_BV(ADEN); // turn ADC off
    
    return result;
}