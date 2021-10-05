#include "adc.h"
#include <avr/io.h>
#include <stdbool.h>

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
    
    // Single conversion mode by default
    // No triggers are used    
} 

static void select_channel(enum ADC_Channel channel) {
    while (ADCSRA & _BV(ADSC)); // wait for prev conversion end  
    
    uint8_t admux = ADMUX;
    admux &= 0xF0; // clear MUX bits
    admux |= channel & 0x0F;
    ADMUX = admux;
}

enum Alignment {
    AL_Left = _BV(ADLAR),
    AL_Right = ~_BV(ADLAR),
};

static void set_alignment(enum Alignment al) {
    switch (al) {
        case AL_Left: ADMUX |= _BV(ADLAR); break;
        case AL_Right: ADMUX &= ~_BV(ADLAR); break;
    }
}

static void set_digital_pin_enabled(bool enabled, enum ADC_Channel channel) {
    if (enabled)
        DIDR0 &= ~_BV(channel); // turn off power safe mode for pin
    else
        DIDR0 |= _BV(channel); // turn on power safe mode for pin  
}

static void do_conversion() {    
    ADCSRA |= _BV(ADEN); // turn ADC on
    ADCSRA |= _BV(ADSC); // start conversion
    while (ADCSRA & _BV(ADSC)); // wait for conversion end
    ADCSRA &= ~_BV(ADEN); // turn ADC off
}

uint8_t adc_scan_channel_uint8(enum ADC_Channel channel) {
    set_alignment(AL_Left); 
    set_digital_pin_enabled(false, channel);    
    set_ADC_clk_prescaler(ADCPSC_32);    
    select_channel(channel);    
    do_conversion();   
    
    uint8_t result = ADCH;  
    
    set_digital_pin_enabled(true, channel);    
    return result;
}

uint16_t adc_scan_channel_uint16(enum ADC_Channel channel) {
    set_alignment(AL_Right);
    set_digital_pin_enabled(false, channel);   
    set_ADC_clk_prescaler(ADCPSC_64);     
    select_channel(channel);       
    do_conversion();         
           
    uint16_t result = (uint16_t)ADCL;
    result |= (uint16_t)ADCH << 8;
    
    set_digital_pin_enabled(true, channel);    
    return result;
}