#ifndef IO_MAP_H
#define IO_MAP_H

#include <avr/io.h>

// --- UART / DFPLAYER (PD1/TXD) ---
// UART is handled by the hardware peripheral, but we define pin for reference
#define DF_UART_PORT   PORTD
#define DF_UART_DDR    DDRD
#define DF_UART_TX_PIN 1 

// --- TM1637 DISPLAY (PB0, PB1) ---
#define DISP_PORT      PORTB
#define DISP_DDR       DDRB
#define DISP_PIN_REG   PINB
#define DISP_CLK_PIN   0
#define DISP_DIO_PIN   1

// --- BUTTONS (PD2, PD3, PD4) ---
#define BTN_PORT       PORTD
#define BTN_PIN_REG    PIND
#define BTN_DDR        DDRD

// Mapping Physical Pins to Logical Names
// BTN_L (Left)   = PD2
// BTN_M (Middle) = PD3
// BTN_R (Right)  = PD4
#define PIN_BTN_L      2 
#define PIN_BTN_M      3 
#define PIN_BTN_R      4 

#endif