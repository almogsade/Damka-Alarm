#include "tm1637.h"
#include "io_map.h"      // <--- Now it knows about your board wiring
#include <avr/io.h>
#include <util/delay.h>

// 7-segment encoding 0-9
static const uint8_t digit_to_seg[10] = {
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F
};

// --- LOW LEVEL MACROS (Mapped to io_map.h) ---
// We use the definitions from io_map.h directly for maximum speed
#define CLK_HIGH()  (DISP_PORT |=  (1 << DISP_CLK_PIN))
#define CLK_LOW()   (DISP_PORT &= ~(1 << DISP_CLK_PIN))
#define DIO_HIGH()  (DISP_PORT |=  (1 << DISP_DIO_PIN))
#define DIO_LOW()   (DISP_PORT &= ~(1 << DISP_DIO_PIN))

// Input/Output Direction Macros
#define DIO_OUTPUT() (DISP_DDR |= (1 << DISP_DIO_PIN))
#define DIO_INPUT()  (DISP_DDR &= ~(1 << DISP_DIO_PIN))
#define DIO_READ()   (DISP_PIN_REG & (1 << DISP_DIO_PIN))


static void tm1637_start(void) {
    CLK_HIGH();
    DIO_HIGH();
    DIO_LOW();
    CLK_LOW();
}

static void tm1637_stop(void) {
    CLK_LOW();
    DIO_LOW();
    CLK_HIGH();
    DIO_HIGH();
}

static void tm1637_write_byte(uint8_t b) {
    for (uint8_t i = 0; i < 8; i++) {
        CLK_LOW();
        if (b & 0x01) DIO_HIGH();
        else          DIO_LOW();
        _delay_us(3);
        CLK_HIGH();
        _delay_us(3);
        b >>= 1;
    }

    // ACK Check
    CLK_LOW();
    DIO_INPUT();   // Float pin to listen for ACK
    DIO_LOW();     // Internal Pull-off (ensure strictly input)
    _delay_us(3);
    CLK_HIGH();
    _delay_us(3);  // Device pulls Low here if ACK
    CLK_LOW();
    DIO_OUTPUT();  // Reclaim control
}

// NOTE: We no longer need pin arguments! It's all in io_map.h
void tm1637_init(void) {
    // Set pins as Output
    DISP_DDR |= (1 << DISP_CLK_PIN) | (1 << DISP_DIO_PIN);
    
    // Set Defaults (High)
    CLK_HIGH();
    DIO_HIGH();

    tm1637_set_brightness(3);
}

void tm1637_set_brightness(uint8_t brightness) {
    tm1637_start();
    tm1637_write_byte(0x88 | (brightness & 0x07)); 
    tm1637_stop();
}

void tm1637_display_segments(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3) {
    tm1637_start();
    tm1637_write_byte(0x40); // Auto increment address
    tm1637_stop();

    tm1637_start();
    tm1637_write_byte(0xC0); // Start address 0
    tm1637_write_byte(s0);
    tm1637_write_byte(s1);
    tm1637_write_byte(s2);
    tm1637_write_byte(s3);
    tm1637_stop();
}

void tm1637_display_time(uint8_t min, uint8_t sec, uint8_t colon) {
    uint8_t s0 = digit_to_seg[min / 10];
    uint8_t s1 = digit_to_seg[min % 10];
    uint8_t s2 = digit_to_seg[sec / 10];
    uint8_t s3 = digit_to_seg[sec % 10];

    if (colon) s1 |= 0x80; 

    tm1637_display_segments(s0, s1, s2, s3);
}