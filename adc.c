#include "adc.h"
#include "avr/io.h"

enum ADC_REFS {
    AR_AREF = 0x00,
    AR_AVcc_AREFcap = 0x01,
    AR_Internal_1_1V_AREFcap = 0x03
};

void adc_init(void) {
    ADMUX &= ~(_BV(REFS1) | _BV(REFS0)); // AREF by default    
    ADMUX |= _BV(ADLAR); // left adjusted
    
    // Single conversion mode by default
    // No triggers are used
    
    // ADC clock prescaler to 128
    ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); 
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