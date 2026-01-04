#include "buttons.h"
#include "io_map.h"
#include "timer.h"

static uint8_t last_port_state = 0xFF; 
static uint32_t last_debounce_time = 0;
#define DEBOUNCE_DELAY 50 

void buttons_init(void) {
    // Set as Input
    BTN_DDR &= ~((1 << PIN_BTN_L) | (1 << PIN_BTN_M) | (1 << PIN_BTN_R));
    // Enable Internal Pull-ups
    BTN_PORT |= (1 << PIN_BTN_L) | (1 << PIN_BTN_M) | (1 << PIN_BTN_R);
}

ButtonID buttons_read(void) {
    uint32_t now = millis();
    if (now - last_debounce_time < DEBOUNCE_DELAY) return BTN_NONE;

    // Read Logic: Active LOW (0 = Pressed)
    uint8_t mask = (1 << PIN_BTN_L) | (1 << PIN_BTN_M) | (1 << PIN_BTN_R);
    uint8_t current_state = BTN_PIN_REG & mask;
    
    ButtonID detected = BTN_NONE;

    // Detect Falling Edge (Transition from 1 to 0)
    // Check Left
    if ((last_port_state & (1 << PIN_BTN_L)) && !(current_state & (1 << PIN_BTN_L))) {
        detected = BTN_L;
    }
    // Check Middle
    else if ((last_port_state & (1 << PIN_BTN_M)) && !(current_state & (1 << PIN_BTN_M))) {
        detected = BTN_M;
    }
    // Check Right
    else if ((last_port_state & (1 << PIN_BTN_R)) && !(current_state & (1 << PIN_BTN_R))) {
        detected = BTN_R;
    }

    if (current_state != last_port_state) {
        last_debounce_time = now;
        last_port_state = current_state;
    }

    return detected;
}