#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include "dfplayer.h"

int main(void) {
    // 1. Initialize Hardware
    UART_Init();
    
    // 2. Initialize Player (Includes the 3s boot delay)
    DF_Init();
    
    // 3. Setup (Good Practice)
    // Set a known volume level immediately upon startup.
    DF_SetVolume(20); 

    // 4. Play "Start Up" Sound (0001.mp3)
    DF_PlayTrack(1);

    while (1) {

    }
    
    return 0;
}