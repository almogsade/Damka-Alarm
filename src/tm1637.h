#ifndef TM1637_H
#define TM1637_H

#include <stdint.h>

void tm1637_init(uint8_t clk_pin, uint8_t dio_pin);
void tm1637_set_brightness(uint8_t brightness);
void tm1637_display_segments(uint8_t s0,uint8_t s1,uint8_t s2,uint8_t s3);
void tm1637_clear(void);
void tm1637_display_time(uint8_t min, uint8_t sec, uint8_t colon);

#endif
