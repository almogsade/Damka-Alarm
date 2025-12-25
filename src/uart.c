#include "uart.h"

void UART_Init(void) {
    // 1. Set the Calibrated Baud Rate
    UBRR0H = 0;
    UBRR0L = UART_CALIBRATED_UBRR;
    
    // 2. Enable Double Speed Mode (U2X0)
    // This reduces error rates for 8MHz internal oscillators.
    UCSR0A |= (1 << U2X0);
    
    // 3. Enable Transmitter
    // We only need TX to talk to the DFPlayer. RX is optional.
    UCSR0B = (1 << TXEN0);
    
    // 4. Set Frame Format: 8 Data bits, No Parity, 1 Stop bit (8N1)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_Tx(uint8_t data) {
    // Wait for empty transmit buffer (UDRE0 flag)
    while (!(UCSR0A & (1 << UDRE0)));
    
    // Put data into buffer, sending it
    UDR0 = data;
}