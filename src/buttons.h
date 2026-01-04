#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

// Logical Button Names
typedef enum {
    BTN_NONE = 0,
    BTN_L, // Left
    BTN_M, // Middle
    BTN_R  // Right
} ButtonID;

void buttons_init(void);
ButtonID buttons_read(void);

#endif