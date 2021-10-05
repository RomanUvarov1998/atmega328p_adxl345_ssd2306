#include <util/twi.h>

#include "lcd.h"
#include "font.h"
#include "utils.h"
#define SLAVE_ADDR 0b0111100

static uint8_t cmd_buf[TWI_BUFFER_LENGTH];
static volatile uint8_t buf_size = 0;
static void cmd_buf_push(uint8_t cmd);
static void cmd_buf_clear();
static void cmd_buf_send();

#define DRAW_BUF_LENGTH SYMBOL_W_COLS
    
enum Cmd {
    CMD_ZERO = 0x00,
    CMD_DISPLAY_MODE_SLEEP = 0xAE,
    CMD_SET_OSC_FREQ = 0xD5,
    CMD_SET_MUX_RATIO = 0xA8,
    CMD_SET_DISPLAY_OFFSET = 0xD3,
    CMD_SET_DISPLAY_START_LINE = 0x40,
    CMD_SET_CHARGE_PUMP_ENABLED = 0x8D,
    CMD_SET_MEMORY_ADDRESSING_MODE = 0x20,
    CMD_SET_SEGMENT_REMAP_ENABLED = 0xA0,
    CMD_SET_COM_OUTPUT_SCAN_DIR = 0xC0,
    CMD_SET_COM_HW_PINS_CONFIG = 0xDA,
    CMD_SET_CONTRAST = 0xDA,
    CMD_SET_PRECHARGE_PERIOD = 0x81,
    CMD_SET_VCOMH_DESELECT_LEVEL = 0xDB,
    CMD_DEACTIVATE_SCROLL = 0x2E,
    CMD_DISPLAY_RAM_CONTENT = 0xA4,
    CMD_SET_INVERSE_MODE = 0xA6,
    CMD_SET_PAGE_START_ADDR = 0xB0,
    CMD_SET_COLUMN_RANGE_ADDRS = 0x21,
    CMD_SET_DISPLAY_START_LINE_REG = 0x40,
};
    
enum DisplayMode {
    DM_Sleep = 0x00,
    DM_Normal = 0x01
};
static void set_display_mode(enum DisplayMode mode);
static void set_clk_divide_ratio_or_osc_freq(uint8_t divide_ratio, uint8_t osc_freq);
static void set_multiplex_ratio(uint8_t ratio);
static void set_display_offset(uint8_t offset);
enum DisplayChargePump {
    DCP_Enabled = 0x14,
    DCP_Disabled = 0x10
};
static void set_charge_pump_enabled(enum DisplayChargePump mode);
enum MemoryAddressingMode {
    MAM_Horizontal = 0x00,
    MAM_Vertical = 0x01,
    MAM_Page = 0x02
};
static void set_memory_addressing_mode(enum MemoryAddressingMode mode);
static void set_segment_remap(bool do_remap);
enum COM_OutputScanDirection {
    COSD_Normal = 0x00,
    COSD_Remapped = 0x08,
};
static void set_com_output_scan_direction(enum COM_OutputScanDirection dir);
enum COM_PinConfig {
    CPC_Sequential = 0x00,
    CPC_Alternative = 0x10,
};
enum COM_LeftRightRemapState {
    CLRRS_Enable = 0x20,
    CLRRS_Disable = 0x00,
};
static void set_com_pins_hw_config(
    enum COM_PinConfig pin_cfg, 
    enum COM_LeftRightRemapState remap);
static void set_contrast_control(uint8_t contrast_value);
static void set_precharge_period(uint8_t phase1, uint8_t phase2);
static void set_Vcomh_deselect_level(uint8_t level);
static void deactivate_scroll();
enum DisplayFollow_RAM_Content {
    DRC_Follow = 0x00,
    DRC_Ignore = 0x01,
};
static void set_output_follow_RAM_content(enum DisplayFollow_RAM_Content mode);
enum NormalInverseMode {
    NI_Normal = 0x00,
    NI_Inverse = 0x01
};
static void set_inverse_mode(enum NormalInverseMode mode);
static void set_col_start_addr(uint8_t addr);
static void set_col_range_addr(uint8_t start_addr, uint8_t end_addr);
static void set_page_start_addr(uint8_t addr);
static void draw_symbol(const Symbol const* sym_buf);

#define SYM_COL_MAX 127
#define SYM_PAGE_MAX 3
static volatile int8_t sym_col = 0;
static volatile int8_t sym_page = 0;
static void go_to_next_symbol_pos();
static void reset_symbol_pos();

//
// -------------------- Commands ---------------------------
//
static void set_display_mode(enum DisplayMode mode) { 
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    
    uint8_t cmd = CMD_DISPLAY_MODE_SLEEP | mode;
    cmd_buf_push(cmd);
    
    cmd_buf_send();    
}

static void set_clk_divide_ratio_or_osc_freq(uint8_t divide_ratio, uint8_t osc_freq) { 
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    cmd_buf_push(CMD_SET_OSC_FREQ);
    
    uint8_t value = 0;
    value |= divide_ratio & 0x0F;
    value |= (osc_freq & 0x0F) << 4;
    cmd_buf_push(value);
    
    cmd_buf_send();    
}

static void set_multiplex_ratio(uint8_t ratio) { 
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    cmd_buf_push(CMD_SET_MUX_RATIO);
    
    if (ratio < 0x0F)
        ratio = 0x0F;
    cmd_buf_push(ratio & 0x1F);
    
    cmd_buf_send();    
}

static void set_display_offset(uint8_t offset) {
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    cmd_buf_push(CMD_SET_DISPLAY_OFFSET);
    cmd_buf_push(offset & 0x3F);
    cmd_buf_send(); 
}

static void set_display_start_line(uint8_t sl) {
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    
    uint8_t cmd = CMD_SET_DISPLAY_START_LINE;
    cmd |= sl & 0x3F;
    cmd_buf_push(cmd);
    
    cmd_buf_send(); 
}

static void set_charge_pump_enabled(enum DisplayChargePump mode) {
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    cmd_buf_push(CMD_SET_CHARGE_PUMP_ENABLED);
    cmd_buf_push(mode);    
    cmd_buf_send(); 
}

static void set_memory_addressing_mode(enum MemoryAddressingMode mode) {
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    cmd_buf_push(CMD_SET_MEMORY_ADDRESSING_MODE);
    cmd_buf_push(mode);    
    cmd_buf_send(); 
}

static void set_segment_remap(bool do_remap) {
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    
    uint8_t cmd = CMD_SET_SEGMENT_REMAP_ENABLED;
    if (do_remap)
        cmd |= 0x01;
    cmd_buf_push(cmd);
    
    cmd_buf_send(); 
}

static void set_com_output_scan_direction(enum COM_OutputScanDirection dir) {
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    
    uint8_t cmd = CMD_SET_COM_OUTPUT_SCAN_DIR;
    cmd |= dir;
    cmd_buf_push(cmd);
    
    cmd_buf_send(); 
}

static void set_com_pins_hw_config(
    enum COM_PinConfig pin_cfg, 
    enum COM_LeftRightRemapState remap) 
{
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    cmd_buf_push(CMD_SET_COM_HW_PINS_CONFIG);
    
    uint8_t cmd = pin_cfg | remap;
    cmd_buf_push(cmd);
    
    cmd_buf_send();
}

static void set_contrast_control(uint8_t contrast_value) {
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    cmd_buf_push(CMD_SET_CONTRAST);
    cmd_buf_push(contrast_value);
    cmd_buf_send();
}

static void set_precharge_period(uint8_t phase1, uint8_t phase2) {
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    cmd_buf_push(CMD_SET_PRECHARGE_PERIOD);
    
    uint8_t value = 0;
    if (phase1 == 0x00)
        phase1 = 0x01;
    value |= phase1 & 0x0F;
    if (phase2 == 0x00)
        phase2 = 0x01;
    value |= (phase2 & 0x0F) << 4;
    cmd_buf_push(value);
    
    cmd_buf_send();
}

static void set_Vcomh_deselect_level(uint8_t level) {
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    cmd_buf_push(CMD_SET_VCOMH_DESELECT_LEVEL);
    
    uint8_t value = 0;
    value |= level & 0x07;
    value <<= 4;
    cmd_buf_push(value);
    
    cmd_buf_send();
}

static void deactivate_scroll() {
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    cmd_buf_push(CMD_DEACTIVATE_SCROLL);    
    cmd_buf_send();
}

static void set_output_follow_RAM_content(enum DisplayFollow_RAM_Content mode) {
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    
    uint8_t cmd = CMD_DISPLAY_RAM_CONTENT | mode;
    cmd_buf_push(cmd);    
    
    cmd_buf_send();
}

static void set_inverse_mode(enum NormalInverseMode mode) {
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    
    uint8_t cmd = CMD_DISPLAY_RAM_CONTENT | mode;
    cmd_buf_push(cmd);    
    
    cmd_buf_send();
}

static void set_col_start_addr(uint8_t addr) {
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    
    uint8_t cmd_low = addr & 0x0F;    
    cmd_buf_push(cmd_low);
    
    cmd_buf_send();
    
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);    
    
    uint8_t cmd_high = ((addr & 0xF0) >> 4) | 0x10;
    cmd_buf_push(cmd_high); 
     
    cmd_buf_send();
}

static void set_col_range_addr(uint8_t start_addr, uint8_t end_addr) {    
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    cmd_buf_push(CMD_SET_COLUMN_RANGE_ADDRS);     
    cmd_buf_push(start_addr & 0x7F);       
    cmd_buf_push(end_addr & 0x7F);     
    cmd_buf_send();
}

static void set_page_start_addr(uint8_t addr) {
    cmd_buf_clear();
    cmd_buf_push(CMD_ZERO);
    
    uint8_t cmd = CMD_SET_PAGE_START_ADDR;
    cmd |= addr & 0x07;
    cmd_buf_push(cmd); 
    
    cmd_buf_send();
}

//
// -------------------- Operations -------------------------
//
void lcd_init() {
    set_display_mode(DM_Sleep);
    _delay_ms(100);
    set_clk_divide_ratio_or_osc_freq(0x00, 0x08);
    set_multiplex_ratio(0x3F);
    set_display_offset(0x00);
    set_display_start_line(0x00);
    set_charge_pump_enabled(DCP_Enabled);
    set_memory_addressing_mode(MAM_Horizontal);
    set_com_output_scan_direction(COSD_Normal);
    set_segment_remap(false);
    set_com_pins_hw_config(CPC_Sequential, CLRRS_Disable);
    set_contrast_control(0x8F);
    set_precharge_period(0x01, 0x0F);
    set_Vcomh_deselect_level(0x04);
    deactivate_scroll();
    set_output_follow_RAM_content(DRC_Follow);
    set_inverse_mode(NI_Normal);
    set_col_start_addr(0x00);
    set_page_start_addr(0x00);    
}

void lcd_clear(void) {
    set_col_start_addr(0x00);
    set_page_start_addr(0x00);   
    
    di();
    // start
    TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWSTA);
    while ((TWCR & _BV(TWINT)) == 0);
    assert(TW_STATUS == TW_START);
    
    // slaw
    uint8_t slaw = SLAVE_ADDR << 1;
    TWDR = slaw;
    TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN);
    while ((TWCR & _BV(TWINT)) == 0);
    assert(TW_STATUS == TW_MT_SLA_ACK);
        
    // cmd to write data
    TWDR = CMD_SET_DISPLAY_START_LINE_REG;
    TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN);
    while ((TWCR & _BV(TWINT)) == 0);
    assert(TW_STATUS == TW_MT_DATA_ACK);
    
    uint8_t page, col;
    for (page = 0; page < 8; ++page) {
        for (col = 0; col < 128; ++col) {
            TWDR = 0x00;
            TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN);
            while ((TWCR & _BV(TWINT)) == 0);
            assert(TW_STATUS == TW_MT_DATA_ACK);
        }
    }
    
    twi_stop();
    ei();
    
    set_display_mode(DM_Normal);
}

void lcd_draw_text(const char *const cstr) {    
    const char *ch = cstr;
    while (*ch != 0) {
        const Symbol const* sym_buf = get_char(*ch);
        draw_symbol(sym_buf);
        ch++;
    }
}

void lcd_draw_int(uint8_t value) {
    char buff[4] = {0};
    int8_t pos = 2;
    uint8_t div;
    
    while (pos >= 0) {
        div = value / 10;
        
        uint8_t div_x_10 = (div << 3) + (div << 1);
        uint8_t mod = value - div_x_10;
        
        if (mod == 0 && div == 0 && pos < 2) {
            buff[pos] = ' ';
        } else {
            buff[pos] = mod + '0';
        }
        
        value = div;
        --pos;
    }
    
    lcd_draw_text(buff);
}

void lcd_set_cursor_pos(uint8_t row, uint8_t col) {
    #define MAX_COL (127 / SYMBOL_W_COLS)
    #define MAX_ROW SYM_PAGE_MAX
    if (col > MAX_COL)
        col = MAX_COL;
    if (row > MAX_ROW)
        row = MAX_ROW;
    
    sym_col = col * SYMBOL_W_COLS;
    sym_page = row;  
}

//
// -------------------- Utils -----------------------------
//
static void cmd_buf_push(uint8_t cmd) { // fast blinking on buffer overflow
    assert(buf_size < TWI_BUFFER_LENGTH);
    cmd_buf[buf_size++] = cmd;
}

static void cmd_buf_clear() {
    buf_size = 0;
}

static void cmd_buf_send() {
    uint8_t result = twi_writeTo(SLAVE_ADDR, cmd_buf, buf_size, true, true);
    assert(result == 0);
}

static void draw_symbol(const Symbol const* sym_buf) {   
    set_col_range_addr(sym_col, sym_col + SYMBOL_W_COLS);
    set_page_start_addr(sym_page);
    
    cmd_buf_clear();
    
    uint8_t start_line = 0;
    uint8_t cmd = CMD_SET_DISPLAY_START_LINE_REG;
    cmd |= start_line & 0x3F;
    cmd_buf_push(cmd); 
    
    uint8_t i = 0;
    for (i = 0; i < DRAW_BUF_LENGTH; ++i)
        cmd_buf_push(sym_buf->bytes[i]); 
    
    cmd_buf_send();
    
    go_to_next_symbol_pos();
}

static void reset_symbol_pos() {
    sym_col = 0;
    sym_page = 0;
}
static void go_to_next_symbol_pos() {
    sym_col += SYMBOL_W_COLS;
    if (sym_col + SYMBOL_W_COLS > SYM_COL_MAX) {
        sym_col = 0;
        ++sym_page;
    }
    if (sym_page > SYM_PAGE_MAX) 
        reset_symbol_pos();
}
