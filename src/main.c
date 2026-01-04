#include <avr/io.h>
#include <stdbool.h>

#include "io_map.h"

#include "uart.h"
#include "dfplayer.h"
#include "tm1637.h"
#include "timer.h"
#include "buttons.h"

// --- STATE DEFINITIONS ---
typedef enum {
    STATE_IDLE,
    STATE_SET_MIN,
    STATE_SET_SEC,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_ALARM
} TimerState;

TimerState currentState = STATE_IDLE;

// --- DATA VARIABLES ---
// 'stored' holds the config, 'live' holds the countdown
uint8_t stored_min = 0;
uint8_t stored_sec = 0;
int8_t  live_min = 0; // Signed to make underflow logic easier
int8_t  live_sec = 0;

// --- TIMING VARIABLES ---
uint32_t last_tick_time = 0;
uint32_t last_blink_time = 0;
bool blink_on = true;

// --- DISPLAY LOGIC ---
void refresh_display(void) {
    uint32_t now = millis();
    bool show_colon = true;
    
    // 500ms Slow Blink for UI
    if (now - last_blink_time >= 500) {
        blink_on = !blink_on;
        last_blink_time = now;
    }

    uint8_t d_min = (currentState == STATE_RUNNING || currentState == STATE_PAUSED) ? live_min : stored_min;
    uint8_t d_sec = (currentState == STATE_RUNNING || currentState == STATE_PAUSED) ? live_sec : stored_sec;

    // Visual Feedback based on State
    if (currentState == STATE_SET_MIN && !blink_on) {
        // While setting minutes, we could blank them or just blink colon rapidly
        // Let's blink the colon to indicate Edit Mode
        show_colon = false; 
    }
    else if (currentState == STATE_SET_SEC && !blink_on) {
        show_colon = false;
    }
    else if (currentState == STATE_PAUSED && !blink_on) {
        // Flash entire display in Pause
        tm1637_display_segments(0,0,0,0); 
        return;
    }
    else if (currentState == STATE_ALARM) {
         // Flash "00:00" rapidly
         if (!blink_on) {
             tm1637_display_segments(0,0,0,0);
             return;
         }
         d_min = 0; d_sec = 0;
    }

    tm1637_display_time(d_min, d_sec, show_colon);
}

int main(void) {
    // 1. Hardware Initialization via IO_MAP
    timer_init();
    buttons_init();
    UART_Init();
    
    // TM1637 Init (using pins from io_map.h)
    tm1637_init();

    // DFPlayer Init
    DF_Init();
    DF_SetVolume(18);

    while (1) {
        ButtonID btn = buttons_read();
        uint32_t now = millis();

        switch (currentState) {
            // --- IDLE STATE ---
            case STATE_IDLE:
                if (btn == BTN_L) {
                    currentState = STATE_SET_MIN; // Shortcut to Min
                } 
                else if (btn == BTN_R) {
                    currentState = STATE_SET_SEC; // Shortcut to Sec
                } 
                else if (btn == BTN_M) {
                    // Start Timer
                    if (stored_min > 0 || stored_sec > 0) {
                        live_min = stored_min;
                        live_sec = stored_sec;
                        currentState = STATE_RUNNING;
                        last_tick_time = now;
                    }
                }
                break;

            // --- CONFIGURATION STATES ---
            case STATE_SET_MIN:
                // L = Down, R = Up, M = Accept (Go to Sec)
                if (btn == BTN_L) {
                    if (stored_min == 0) stored_min = 99; else stored_min--;
                } 
                else if (btn == BTN_R) {
                    if (stored_min >= 99) stored_min = 0; else stored_min++;
                } 
                else if (btn == BTN_M) {
                    currentState = STATE_SET_SEC; 
                }
                break;

            case STATE_SET_SEC:
                // L = Down, R = Up, M = Accept (Go to IDLE)
                if (btn == BTN_L) {
                    if (stored_sec == 0) stored_sec = 59; else stored_sec--;
                } 
                else if (btn == BTN_R) {
                    if (stored_sec >= 59) stored_sec = 0; else stored_sec++;
                } 
                else if (btn == BTN_M) {
                    currentState = STATE_IDLE; 
                }
                break;

            // --- RUNNING STATE ---
            case STATE_RUNNING:
                // M = Pause
                if (btn == BTN_M) {
                    currentState = STATE_PAUSED;
                }
                // (Optional: BTN_L could be Stop/Reset if desired, keeping simple for now)

                // Timer Logic
                if (now - last_tick_time >= 1000) {
                    last_tick_time += 1000;
                    
                    if (live_sec == 0) {
                        if (live_min == 0) {
                            // TIMER FINISHED
                            currentState = STATE_ALARM;
                            DF_PlayTrack(1); // Play Alarm Sound
                        } else {
                            live_min--;
                            live_sec = 59;
                        }
                    } else {
                        live_sec--;
                    }
                }
                break;

            // --- PAUSED STATE ---
            case STATE_PAUSED:
                // M = Resume, L/R = Reset to IDLE?
                if (btn == BTN_M) {
                    currentState = STATE_RUNNING;
                    last_tick_time = now; // Prevent jump
                } 
                else if (btn == BTN_L || btn == BTN_R) {
                    currentState = STATE_IDLE; // Reset
                }
                break;

            // --- ALARM STATE ---
            case STATE_ALARM:
                // "Until any button is pressed"
                if (btn != BTN_NONE) {
                    DF_Pause(); // Stop Sound Immediately
                    currentState = STATE_IDLE;
                    // Reset live values is implied by reloading from 'stored' next run
                }
                // Note: If track finishes, DFPlayer stops. 
                // Ideally, use a looping track or send Loop Command if supported.
                break;
        }

        refresh_display();
    }
}