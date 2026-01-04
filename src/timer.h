#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// Initialize Timer0 for 1ms interrupts
void timer_init(void);

// Get current milliseconds since startup
uint32_t millis(void);

#endif