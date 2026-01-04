#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint32_t system_millis = 0;

void timer_init(void) {
    // CTC Mode (Clear Timer on Compare Match)
    TCCR0A = (1 << WGM01);
    
    // Prescaler 64: 8MHz / 64 = 125,000 Hz (8us per tick)
    // 1ms target = 1000us / 8us = 125 ticks
    OCR0A = 133; // 0-124 is 125 counts
    
    TIMSK0 |= (1 << OCIE0A); // Enable Compare Match A Interrupt
    TCCR0B |= (1 << CS01) | (1 << CS00); // Start Timer, Prescaler 64
    
    sei(); // Ensure Global Interrupts are enabled
}

ISR(TIMER0_COMPA_vect) {
    system_millis++;
}

uint32_t millis(void) {
    uint32_t m;
    // Atomic read: disable interrupts while reading volatile multi-byte variable
    cli(); 
    m = system_millis;
    sei();
    return m;
}