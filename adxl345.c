#include "adxl345.h"

#define SLAVE_ADDR 0x53

enum RegAddr {
    RA_BW_RATE = 0x2C,
    RA_POWER_CTL = 0x2D,
    RA_INT_ENABLE = 0x2E,
    RA_INT_MAP = 0x2F,
    RA_INT_SOURCE = 0x30,
    RA_DATA_FORMAT = 0x31,
    RA_DATA_BEGIN = 0x32
};
static void write_reg(enum RegAddr addr, uint8_t value);
static uint8_t read_reg(enum RegAddr reg_addr);
static void read_regs(enum RegAddr reg_addr, uint8_t* in_buf, uint8_t length);

typedef union {    
    struct {
        enum {
            DR_0_10_Hz = 0x00,
            DR_0_20_Hz,
            DR_0_39_Hz,
            DR_0_78_Hz,
            DR_1_56_Hz,
            DR_3_13_Hz,
            DR_6_25_Hz,
            DR_12_5_Hz,
            DR_25_Hz,
            DR_50_Hz,
            DR_100_Hz,
            DR_200_Hz,
            DR_400_Hz,
            DR_800_Hz,
            DR_1600_Hz,
            DR_3200_Hz,
        }       data_rate_hz : 4;
        bool    low_power : 1;
    }       as_bits;
    uint8_t as_byte;
} BW_Rate;
static void set_bw_rate(BW_Rate settings);

typedef union {
    struct {
        enum {
            WF_8_Hz = 0x00,
            WF_4_Hz,
            WF_2_Hz,
            WF_1_Hz,
        }       wakeup_freq_Hz : 2;
        bool    sleep : 1;
        bool    measure : 1;
        bool    auto_sleep : 1;
        bool    link : 1;
    }       as_bits;
    uint8_t as_byte;
} PowerCtl;
static void set_power_ctl(PowerCtl settings);

typedef union {
    struct {
        bool    overrun : 1;
        bool    watermark : 1;
        bool    free_fall : 1;
        bool    inactivity : 1;
        bool    activity : 1;
        bool    double_tap : 1;
        bool    single_tap : 1;
        bool    data_ready : 1;
    }       as_bits;
    uint8_t as_byte;
} IntEnable;
static void set_int_enable(IntEnable settings);

typedef IntEnable IntMap;
static void set_int_map(IntMap settings);

typedef IntEnable IntSource;
static IntSource get_int_source(void);

typedef union {
    struct {
        enum {
            DR_PlusMinus_2g = 0x00,
            DR_PlusMinus_4g,
            DR_PlusMinus_8g,
            DR_PlusMinus_16g,
        }       range : 2;
        bool    justify : 1;
        bool    full_res : 1;
        bool    dummy : 1;
        bool    int_invert : 1;
        bool    spi : 1;
        bool    self_test : 1;
    }       as_bits;
    uint8_t as_byte;
} DataFormat;
static void set_data_format(DataFormat settings);
static int16_t axis_data_to_mg(uint8_t data_0, uint8_t data_1);
static volatile DataFormat current_data_format = {0};

struct AccValues get_XYZ_data(void);

void adxl345_init(void) {
    // PB4 - pin with interrupt, that tells that data is ready
    DDRB &= ~_BV(DDB3); // is input
    PORTB &= ~_BV(PORTB3); // is pulled high
    
    PCICR |= _BV(PCIE0); // pin change interrupt for PB0-PB5 enabled
    PCMSK0 |= _BV(PCINT3); // PB3
    
    /*=================== Write to registers =================== */
    BW_Rate bw_rate = {
        .as_bits = {
            .data_rate_hz = DR_100_Hz,
            .low_power = 0,
        }
    };
    set_bw_rate(bw_rate);
    
    IntMap int_map = {
        .as_bits = {
            .overrun = 0,
            .watermark = 0,
            .free_fall = 0,
            .inactivity = 0,
            .activity = 0,
            .double_tap = 0,
            .single_tap = 0,
            .data_ready = 0 // map on int0 pin
        }
    };
    set_int_map(int_map);
    
    DataFormat data_format = {
        .as_bits = {
            .range = DR_PlusMinus_16g,
            .justify = 0, // right alignment
            .full_res = 1,
            .dummy = 0,
            .int_invert = 0, // pin gets low on interrupt
            .spi = 0,
            .self_test = 0
        }
    };
    set_data_format(data_format);
    
    IntEnable int_enable = {
        .as_bits = {
            .overrun = 1,
            .watermark = 0,
            .free_fall = 0,
            .inactivity = 0,
            .activity = 1,
            .double_tap = 0,
            .single_tap = 0,
            .data_ready = 0
        }
    };
    set_int_enable(int_enable);
    
    PowerCtl power_ctl = {
        .as_bits = {
            .wakeup_freq_Hz = WF_8_Hz,
            .sleep = 0,
            .measure = 1,
            .auto_sleep = 0,
            .link = 0
        }
    };
    set_power_ctl(power_ctl);
}

static volatile bool int1_occured = false;
bool adxl345_has_unread_data(void) {
    IntSource int_source = get_int_source();
    return int_source.as_bits.data_ready > 0;
    /*
    if (!int1_occured)
        return false;
    
    DDRB |= _BV(DDB5);
    while (1) {
        PORTB |= _BV(PORTB5);
        _delay_ms(1000);
        PORTB &= ~_BV(PORTB5);
        _delay_ms(1000);
    }
    
    IntSource int_source = get_int_source();
    return int_source.as_bits.data_ready > 0;
     */
}

struct AccValues adxl345_get_XYZ_data(void) {
    union {
        struct {
            uint8_t X_0;
            uint8_t X_1;
            uint8_t Y_0;
            uint8_t Y_1;
            uint8_t Z_0;
            uint8_t Z_1;
        } data;  
        uint8_t bytes[6];
    } buf = {0};
    
    read_regs(RA_DATA_BEGIN, buf.bytes, 6);
    
    struct AccValues acc = {
        .x_mg = axis_data_to_mg(buf.data.X_0, buf.data.X_1),
        .y_mg = axis_data_to_mg(buf.data.Y_0, buf.data.Y_1),
        .z_mg = axis_data_to_mg(buf.data.Z_0, buf.data.Z_1),
    };   
    
    return acc;
}

#define B3_IS_HIGH ((PINB & _BV(PINB3)) > 0)
void adxl345_process_PCINT0_ISR(void) {
    int1_occured = !B3_IS_HIGH;
}

//========================== Utils ==========================
static void set_bw_rate(BW_Rate settings) {
    write_reg(RA_BW_RATE, settings.as_byte);
}
static void set_power_ctl(PowerCtl settings) {
    write_reg(RA_POWER_CTL, settings.as_byte);
}
static void set_int_enable(IntEnable settings) {
    write_reg(RA_INT_ENABLE, settings.as_byte);
}
static void set_int_map(IntMap settings) {
    write_reg(RA_INT_MAP, settings.as_byte);
}
static IntSource get_int_source(void) {
    IntSource int_source = {0};
    int_source.as_byte = read_reg(RA_INT_SOURCE); 
    return int_source;
}
static void set_data_format(DataFormat settings) {
    write_reg(RA_DATA_FORMAT, settings.as_byte);
    current_data_format = settings;
}
static int16_t axis_data_to_mg(uint8_t data_0, uint8_t data_1) {
    uint16_t value = ((uint16_t)data_1 << 8) | (uint16_t)data_0;
    
    /*================== make LSB to be 4 mg ==================*/
    if (current_data_format.as_bits.justify) { // left justified
        // for full res and 10-bit mode bit for 4 mg is on the same place
        switch (current_data_format.as_bits.range) {
        case DR_PlusMinus_2g: value >>= 6; break; 
        case DR_PlusMinus_4g: value >>= 5; break; 
        case DR_PlusMinus_8g: value >>= 4; break; 
        case DR_PlusMinus_16g: value >>= 3; break; 
        }
    } else { // right justified
        // LSB is already at the right
        if (current_data_format.as_bits.full_res) { // full res      
            // LSB is already 4 mg
        } else { // 10-bit mode
            switch (current_data_format.as_bits.range) {
            case DR_PlusMinus_2g: value <<= 0; break; // cut 6 right bits
            case DR_PlusMinus_4g: value <<= 1; break; // cut 5 right bits
            case DR_PlusMinus_8g: value <<= 2; break; // cut 4 right bits
            case DR_PlusMinus_16g: value <<= 3; break; // cut 3 right bits
            }
        }
    }
          
    value <<= 2; // convert to mg by multiplying by 4
    int16_t value_mg = *(int16_t*)&value;
    return value_mg;
}

static void write_reg(enum RegAddr addr, uint8_t value) {    
    uint8_t write_buf[2] = { addr, value };
    uint8_t result = twi_writeTo(SLAVE_ADDR, write_buf, 2, true, true);
    hang_if_not(result == 0);
    
    uint8_t written_value = read_reg(addr);
    hang_if_not(value == written_value);
}

static uint8_t read_reg(enum RegAddr reg_addr) {
    uint8_t result, reg_value;
    
    // write reg addr
    result = twi_writeTo(SLAVE_ADDR, &reg_addr, 1, true, false);
    hang_if_not(result == 0);
    
    // read reg data
    result = twi_readFrom(SLAVE_ADDR, &reg_value, 1, true);
    hang_if_not(result == 1);
    
    return reg_value;
}

static void read_regs(enum RegAddr reg_addr, uint8_t* in_buf, uint8_t length) {
    uint8_t result;
    
    // write reg addr
    result = twi_writeTo(SLAVE_ADDR, &reg_addr, 1, true, false);
    hang_if_not(result == 0);
    
    // read reg data
    result = twi_readFrom(SLAVE_ADDR, in_buf, length, true);
    hang_if_not(result == length);  
}
