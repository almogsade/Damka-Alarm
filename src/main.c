#include <avr/io.h>
#include <stdbool.h>

#include "uart.h"
#include "dfplayer.h"
#include "tm1637.h"
#include "timer.h"
#include "buttons.h"

// --- STATE MACHINE DEFINITIONS ---
typedef enum {
    STATE_IDLE,
    STATE_SET_MIN,
    STATE_SET_SEC,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_ALARM
} TimerState;

TimerState currentState = STATE_IDLE;

// Time Variables
uint8_t minutes = 0;
uint8_t seconds = 0;
uint8_t stored_minutes = 0;
uint8_t stored_seconds = 0;

// Logic Variables
uint32_t last_second_tick = 0;
uint32_t last_blink_time = 0;
bool blink_state = false;

// --- DISPLAY HELPER ---
void update_display(void) {
    bool show_colon = true;
    uint32_t now = millis();

    // Blink Logic (approx 300ms toggle)
    if (now - last_blink_time >= 300) {
        blink_state = !blink_state;
        last_blink_time = now;
    }

    // -- Visual State Feedback --
    // If setting MIN, blink colon? Or we could blink the number digits if we modified the lib.
    // For now, let's just blink the colon to indicate "Edit Mode".
    if ((currentState == STATE_SET_MIN || currentState == STATE_SET_SEC) && !blink_state) {
        show_colon = false;
    }
    
    // If PAUSED, Blink the whole display (off/on)
    if (currentState == STATE_PAUSED && !blink_state) {
        tm1637_clear(); // You might need to add this to tm1637.c or use display_segments(0,0,0,0)
        return; 
    }

    tm1637_display_time(minutes, seconds, show_colon);
}

int main(void) {
    // 1. Initialize Subsystems
    timer_init();      // Starts the millisecond counter
    buttons_init();    // Configures Button Pins
    UART_Init();       // Comms
    tm1637_init(0, 1); // Display on PB0/PB1

    // 2. Initialize Player
    DF_Init(); 
    DF_SetVolume(20);
    DF_PlayTrack(1); // Boot Sound

    // 3. Application Loop
    while (1) {
        uint8_t btn = buttons_read(); // Non-blocking read
        uint32_t now = millis();

        switch (currentState) {
            
            case STATE_IDLE:
                if (btn == BTN_1) {
                    currentState = STATE_SET_MIN;
                } else if (btn == BTN_2) {
                    minutes = 0; seconds = 0; 
                } else if (btn == BTN_3) {
                    stored_minutes = minutes;
                    stored_seconds = seconds;
                    // Only start if time is set
                    if (minutes > 0 || seconds > 0) {
                        currentState = STATE_RUNNING;
                        last_second_tick = now;
                    }
                }
                break;

            case STATE_SET_MIN:
                if (btn == BTN_1) currentState = STATE_SET_SEC;
                else if (btn == BTN_2) { minutes = (minutes >= 99) ? 0 : minutes + 1; }
                else if (btn == BTN_3) { minutes = (minutes == 0) ? 99 : minutes - 1; }
                break;

            case STATE_SET_SEC:
                if (btn == BTN_1) currentState = STATE_IDLE;
                else if (btn == BTN_2) { seconds = (seconds >= 59) ? 0 : seconds + 1; }
                else if (btn == BTN_3) { seconds = (seconds == 0) ? 59 : seconds - 1; }
                break;

            case STATE_RUNNING:
                if (btn == BTN_1) currentState = STATE_PAUSED;
                
                // Non-blocking timer logic
                if (now - last_second_tick >= 1000) {
                    last_second_tick += 1000; // Exact interval tracking
                    
                    if (seconds == 0) {
                        if (minutes == 0) {
                            currentState = STATE_ALARM;
                            DF_PlayTrack(2); 
                        } else {
                            minutes--;
                            seconds = 59;
                        }
                    } else {
                        seconds--;
                    }
                }
                break;

            case STATE_PAUSED:
                if (btn == BTN_1) {
                    currentState = STATE_RUNNING;
                    last_second_tick = now; // Prevent immediate jump
                } else if (btn == BTN_2) {
                    currentState = STATE_IDLE;
                    minutes = 0; seconds = 0;
                }
                break;

            case STATE_ALARM:
                if (btn != BTN_NONE) {
                    DF_Pause(); 
                    currentState = STATE_IDLE;
                    minutes = stored_minutes;
                    seconds = stored_seconds;
                }
                break;
        }

        update_display();
    }
}