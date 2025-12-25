#include "dfplayer.h"
#include "uart.h"
#include <util/delay.h>

// Packet Constants
#define DF_START_BYTE 0x7E
#define DF_VERSION    0xFF
#define DF_LEN        0x06
#define DF_FEEDBACK   0x00 // 0=No feedback, 1=Feedback
#define DF_END_BYTE   0xEF

// Internal Helper: Builds and sends the 10-byte stack
static void send_stack(uint8_t cmd, uint16_t param) {
    uint8_t highByte = (uint8_t)(param >> 8);
    uint8_t lowByte  = (uint8_t)(param & 0xFF);

    // Checksum Calculation
    // Formula: 0 - (Ver + Len + Cmd + Feedback + ParaH + ParaL)
    uint16_t sum = DF_VERSION + DF_LEN + cmd + DF_FEEDBACK + highByte + lowByte;
    uint16_t checksum = 0 - sum;
    
    UART_Tx(DF_START_BYTE);
    UART_Tx(DF_VERSION);
    UART_Tx(DF_LEN);
    UART_Tx(cmd);
    UART_Tx(DF_FEEDBACK);
    UART_Tx(highByte);
    UART_Tx(lowByte);
    UART_Tx((uint8_t)(checksum >> 8));
    UART_Tx((uint8_t)(checksum & 0xFF));
    UART_Tx(DF_END_BYTE);
    
    // Small delay ensures the module isn't flooded with commands
    _delay_ms(100); 
}

void DF_Init(void) {
    // The DFPlayer takes 1.5 - 3 seconds to boot up after power-on.
    // We handle this delay here so the main code doesn't have to guess.
    _delay_ms(3000);
}

void DF_PlayTrack(uint16_t trackNum) {
    send_stack(DF_CMD_PLAY_TRACK, trackNum);
}

void DF_SetVolume(uint8_t volume) {
    if (volume > 30) volume = 30; // Clamp max volume
    send_stack(DF_CMD_SET_VOL, volume);
    _delay_ms(200); // Extra delay for volume to apply
}

void DF_Pause(void) {
    send_stack(DF_CMD_PAUSE, 0);
}

void DF_Resume(void) {
    send_stack(DF_CMD_PLAY, 0);
}

void DF_Reset(void) {
    send_stack(DF_CMD_RESET, 0);
    _delay_ms(2000); // Wait for reboot
}