#include "pwm.h"
#include <avr\io.h>
#include <stdbool.h>
#include "utils.h"

enum PWM_Channel {
    PWM_Ch_OC0A = 0,
    PWM_Ch_OC0B,
    PWM_Ch_OC1A,
    PWM_Ch_OC1B,
    PWM_Ch_OC2A,
    PWM_Ch_OC2B,
};

// pin to channel
__memx const enum PWM_Channel channels_lookup[PWM_PINS_COUNT] = {
    [PWM_Pin_PD6] = PWM_Ch_OC0A,
    [PWM_Pin_PD5] = PWM_Ch_OC0B,
    [PWM_Pin_PB1] = PWM_Ch_OC1A,
    [PWM_Pin_PB2] = PWM_Ch_OC1B,
    [PWM_Pin_PB3] = PWM_Ch_OC2A,
    [PWM_Pin_PD3] = PWM_Ch_OC2B
};

enum WGM_Values {
    WGM_Normal_TOP_0xFF = 0x00,
    WGM_PWM_PhaseCorrect_TOP_0xFF = 0x01,
    WGM_CTC_TOP_OCRA = 0x02,
    WGM_PWM_Fast_TOP_0xFF = 0x03,
    WGM_PWM_PhaseCorrect_TOP_OCRA = 0x05,
    WGM_PWM_Fast_TOP_OCRA = 0x07,
};

enum COM_Values {
    COM_Normal = 0x00,
    COM_CompareMatchClear_BottomSet = 0x02,
    COM_CompareMatchSet_BottomClear = 0x03,
};

enum CS_Values {
    CS_NoClockSource = 0x00,
    CS_PSK_1,
    CS_PSK_8,
    CS_PSK_64,
    CS_PSK_256,
    CS_PSK_1024,
    CS_ExternalOn_T0_FallingEdge,
    CS_ExternalOn_T0_RisingEdge,
};

#define pwm_pin(ddr, ddr_bit, port, port_bit) do {  \
        ddr |= _BV(ddr_bit);                        \
        port &= ~_BV(port_bit);                     \
    } while (0)

#define wgm(mode_reg_A, mode_reg_B, wgm2, wgm1, wgm0, value) do {           \
        uint8_t tmp = mode_reg_A;                                           \
        tmp &= ~(_BV(wgm0) | _BV(wgm1));                                    \
        tmp |= value & 0x03;                                                \
        mode_reg_A = tmp;                                                   \
                                                                            \
        tmp = mode_reg_B;                                                   \
        tmp &= ~_BV(wgm2);                                                  \
        tmp |= ((value & 0x04) >> 2) << wgm2;                               \
        mode_reg_B = tmp;                                                   \
    } while(0)

#define com(mode_reg_A, com1, com0, value) do {                             \
        uint8_t tmp = mode_reg_A;                                           \
        tmp &= ~(_BV(com0) | _BV(com1));                                     \
        tmp |= (value & 0x03) << com0;                                      \
        mode_reg_A = tmp;                                                   \
    } while(0)

#define cs(mode_reg_B, cs2, cs1, cs0, value) do {                           \
        uint8_t tmp = mode_reg_B;                                           \
        tmp &= ~(_BV(cs2) | _BV(cs1) | _BV(cs0));                           \
        tmp |= (value & 0x07) << cs0;                                       \
        mode_reg_B = tmp;                                                   \
    } while (0)


void pwm_init_pin(enum PWM_Pin pin, uint8_t value) {       
    switch (pin) {
    case PWM_Pin_PD6: pwm_pin(DDRD, DDD6, PORTD, PORTD6); break;
    case PWM_Pin_PD5: pwm_pin(DDRD, DDD5, PORTD, PORTD5); break;
    case PWM_Pin_PB1: pwm_pin(DDRB, DDB1, PORTB, PORTB1); break;
    case PWM_Pin_PB2: pwm_pin(DDRB, DDB2, PORTB, PORTB2); break;
    case PWM_Pin_PB3: pwm_pin(DDRB, DDB3, PORTB, PORTB3); break;
    case PWM_Pin_PD3: pwm_pin(DDRD, DDD3, PORTD, PORTD3); break;
    case PWM_PINS_COUNT:
    default:
        hang_if_not(false);
    }
    
    pwm_set_pin_brightness(pin, value);

    enum PWM_Channel channel = channels_lookup[pin];
    switch (channel) {
    case PWM_Ch_OC0A: 
        wgm(TCCR0A, TCCR0B, WGM02, WGM01, WGM00, WGM_PWM_Fast_TOP_0xFF);
        com(TCCR0A, COM0A1, COM0A0, COM_CompareMatchClear_BottomSet); 
        cs(TCCR0B, CS02, CS01, CS00, CS_PSK_256);
        break;

    case PWM_Ch_OC0B: 
        wgm(TCCR0A, TCCR0B, WGM02, WGM01, WGM00, WGM_PWM_Fast_TOP_0xFF);
        com(TCCR0A, COM0B1, COM0B0, COM_CompareMatchClear_BottomSet);
        cs(TCCR0B, CS02, CS01, CS00, CS_PSK_256);
        break;

    case PWM_Ch_OC1A:
        wgm(TCCR1A, TCCR1B, WGM12, WGM11, WGM10, WGM_PWM_Fast_TOP_0xFF);
        com(TCCR1A, COM1A1, COM1A0, COM_CompareMatchClear_BottomSet);
        cs(TCCR1B, CS12, CS11, CS10, CS_PSK_256);
        break;

    case PWM_Ch_OC1B:
        wgm(TCCR1A, TCCR1B, WGM12, WGM11, WGM10, WGM_PWM_Fast_TOP_0xFF);
        com(TCCR1A, COM1B1, COM1B0, COM_CompareMatchClear_BottomSet);
        cs(TCCR1B, CS12, CS11, CS10, CS_PSK_256);
        break;

    case PWM_Ch_OC2A:
        wgm(TCCR2A, TCCR2B, WGM22, WGM21, WGM20, WGM_PWM_Fast_TOP_0xFF);
        com(TCCR2A, COM2A1, COM2A0, COM_CompareMatchClear_BottomSet);
        cs(TCCR2B, CS22, CS21, CS20, CS_PSK_256);
        break;

    case PWM_Ch_OC2B:
        wgm(TCCR2A, TCCR2B, WGM22, WGM21, WGM20, WGM_PWM_Fast_TOP_0xFF);
        com(TCCR2A, COM2B1, COM2B0, COM_CompareMatchClear_BottomSet);
        cs(TCCR2B, CS22, CS21, CS20, CS_PSK_256);
        break;
    }
}

void pwm_set_pin_brightness(enum PWM_Pin pin, uint8_t value) {
    
    
    enum PWM_Channel channel = channels_lookup[pin] ;
    switch (channel) {
        case PWM_Ch_OC0A: OCR0A = value; break;   
        case PWM_Ch_OC0B: OCR0B = value; break;            
        case PWM_Ch_OC1A: OCR1A = value; break;
        case PWM_Ch_OC1B: OCR1B = value; break;
        case PWM_Ch_OC2A: OCR2A = value; break;
        case PWM_Ch_OC2B: OCR2B = value; break;
    }
}
