#include "tm1637.h"
#include <avr/io.h>
#include <util/delay.h>

static uint8_t CLK_PINNUM, DIO_PINNUM;
static volatile uint8_t *CLK_PORT, *CLK_DDR, *DIO_PORT, *DIO_DDR, *DIO_PIN_REG;

// 7-segment encoding 0-9
static const uint8_t digit_to_seg[10] = {
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F
};

// low-level functions
#define SET_HIGH(port,pin) (*(port) |= (1<<(pin)))
#define SET_LOW(port,pin) (*(port) &= ~(1<<(pin)))
#define READ_PIN(pinreg,pin) ((*(pinreg) & (1<<(pin))) !=0)

static void tm1637_start(void)
{
    SET_HIGH(CLK_PORT,CLK_PINNUM);
    SET_HIGH(DIO_PORT,DIO_PINNUM);
    SET_LOW(DIO_PORT,DIO_PINNUM);
    SET_LOW(CLK_PORT,CLK_PINNUM);
}

static void tm1637_stop(void)
{
    SET_LOW(CLK_PORT,CLK_PINNUM);
    SET_LOW(DIO_PORT,DIO_PINNUM);
    SET_HIGH(CLK_PORT,CLK_PINNUM);
    SET_HIGH(DIO_PORT,DIO_PINNUM);
}

static void tm1637_write_byte(uint8_t b)
{
    for(uint8_t i=0;i<8;i++)
    {
        SET_LOW(CLK_PORT,CLK_PINNUM);

        if(b & 0x01) SET_HIGH(DIO_PORT,DIO_PINNUM);
        else SET_LOW(DIO_PORT,DIO_PINNUM);

        _delay_us(3);
        SET_HIGH(CLK_PORT,CLK_PINNUM);
        _delay_us(3);

        b >>=1;
    }

    // ACK
    SET_LOW(CLK_PORT,CLK_PINNUM);
    *DIO_DDR &= ~(1 << DIO_PINNUM);   // INPUT
    _delay_us(3);

    SET_HIGH(CLK_PORT,CLK_PINNUM);
    _delay_us(3);

    SET_LOW(CLK_PORT,CLK_PINNUM);
    *DIO_DDR |= (1 << DIO_PINNUM);    // OUTPUT
}


void tm1637_init(uint8_t clk_pin, uint8_t dio_pin)
{
    CLK_PINNUM = clk_pin;
    DIO_PINNUM = dio_pin;

    // Map pins
    CLK_PORT = &PORTB;
    CLK_DDR  = &DDRB;
    DIO_PORT = &PORTB;
    DIO_DDR  = &DDRB;
    DIO_PIN_REG = &PINB;

    // Set as output
    *CLK_DDR |= (1<<CLK_PINNUM);
    *DIO_DDR |= (1<<DIO_PINNUM);

    tm1637_set_brightness(3);
}

void tm1637_set_brightness(uint8_t brightness)
{
    tm1637_start();
    tm1637_write_byte(0x88 | (brightness & 0x07)); // display on + brightness
    tm1637_stop();
}

void tm1637_display_segments(uint8_t s0,uint8_t s1,uint8_t s2,uint8_t s3)
{
    tm1637_start();
    tm1637_write_byte(0x40); // auto address
    tm1637_stop();

    tm1637_start();
    tm1637_write_byte(0xC0);
    tm1637_write_byte(s0);
    tm1637_write_byte(s1);
    tm1637_write_byte(s2);
    tm1637_write_byte(s3);
    tm1637_stop();
}


void tm1637_display_time(uint8_t min, uint8_t sec, uint8_t colon)
{
    uint8_t s0 = digit_to_seg[min / 10];
    uint8_t s1 = digit_to_seg[min % 10];
    uint8_t s2 = digit_to_seg[sec / 10];
    uint8_t s3 = digit_to_seg[sec % 10];

    if (colon) s1 |= 0x80;   // colon bit

    tm1637_display_segments(s0, s1, s2, s3);
}