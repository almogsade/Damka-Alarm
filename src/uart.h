#ifndef UART_H
#define UART_H

#include <avr/io.h>
#include <stdint.h>

// --- CONFIGURATION ---
// Based on your calibration: 108 aligned perfectly with your 8MHz chip.
#define UART_CALIBRATED_UBRR 108

// --- API PROTOTYPES ---
void UART_Init(void);
void UART_Tx(uint8_t data);

#endif