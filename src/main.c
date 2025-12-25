#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "tm1637.h"

/* ===================== PIN DEFINITIONS ===================== */

#define CLK_PIN     PB0
#define DIO_PIN     PB2
#define LED_PIN     PB1

#define BTN_INC     PB3
#define BTN_DEC     PB4
#define BTN_START   PB5

/* ===================== SYSTEM TIME ===================== */

volatile uint32_t system_ms = 0;

ISR(TIMER1_COMPA_vect)
{
    system_ms++;
}

void timer1_init(void)
{
    TCCR1A = 0;
    TCCR1B = 0;

    TCCR1B |= (1 << WGM12);                 // CTC
    TCCR1B |= (1 << CS11) | (1 << CS10);    // /64

    OCR1A = 124;                            // 1 ms
    TIMSK1 |= (1 << OCIE1A);
}

uint32_t millis(void)
{
    uint32_t t;
    cli();
    t = system_ms;
    sei();
    return t;
}

/* ===================== BUTTON ===================== */

typedef struct {
    uint8_t state;
    uint32_t last_change;
    uint32_t press_time;
    uint8_t long_handled;
} button_t;

uint8_t button_read(uint8_t pin)
{
    return (PINB & (1 << pin)) ? 1 : 0;
}

void button_update(button_t *b, uint8_t raw)
{
    uint32_t now = millis();

    if (raw != b->state && (now - b->last_change) > 30)
    {
        b->state = raw;
        b->last_change = now;

        if (raw == 0)      // pressed
        {
            b->press_time = now;
            b->long_handled = 0;
        }
    }
}

uint8_t button_short_press(button_t *b)
{
    if (b->state == 1 && !b->long_handled)
    {
        uint32_t dt = millis() - b->press_time;
        if (dt > 30 && dt < 700)
        {
            b->long_handled = 1;
            return 1;
        }
    }
    return 0;
}

uint8_t button_long_press(button_t *b)
{
    if (b->state == 0 && !b->long_handled)
    {
        if (millis() - b->press_time >= 700)
        {
            b->long_handled = 1;
            return 1;
        }
    }
    return 0;
}

/* ===================== APP STATE ===================== */

typedef struct {
    uint8_t minutes;
    uint8_t seconds;
    uint16_t countdown;
    uint8_t running;
    uint8_t edit_seconds;
    uint8_t colon;
    uint8_t blink;
} app_t;

/* ===================== LOGIC ===================== */

void handle_buttons(app_t *a,
                    button_t *inc,
                    button_t *dec,
                    button_t *start)
{
    button_update(start, button_read(BTN_START));
    button_update(inc,   button_read(BTN_INC));
    button_update(dec,   button_read(BTN_DEC));

    /* --- START button --- */
    if (!a->running)
    {
        if (button_long_press(start))
            a->edit_seconds ^= 1;

        if (button_short_press(start))
        {
            a->countdown = a->minutes * 60 + a->seconds;
            if (a->countdown > 0)
                a->running = 1;
        }
    }
    else
    {
        if (button_short_press(start))
            a->running = 0;
    }

    /* --- INC / DEC --- */
    if (!a->running)
    {
        if (button_short_press(inc))
        {
            if (a->edit_seconds)
                a->seconds = (a->seconds + 1) % 60;
            else
                a->minutes = (a->minutes + 1) % 60;
        }

        if (button_short_press(dec))
        {
            if (a->edit_seconds)
                a->seconds = (a->seconds + 59) % 60;
            else
                a->minutes = (a->minutes + 59) % 60;
        }
    }
}

void update_timer(app_t *a, uint32_t *last_sec)
{
    uint32_t now = millis();

    if (a->running && now - *last_sec >= 1000)
    {
        *last_sec += 1000;

        if (a->countdown > 0)
        {
            a->countdown--;
            a->colon ^= 1;
        }
        else
        {
            a->running = 0;
            a->colon = 1;

            PORTB |= (1 << LED_PIN);
            _delay_ms(300);
            PORTB &= ~(1 << LED_PIN);
        }
    }
}

void update_blink(app_t *a, uint32_t *last_blink)
{
    if (!a->running && millis() - *last_blink >= 500)
    {
        *last_blink += 500;
        a->blink ^= 1;
    }

    if (a->running)
        a->blink = 1;
}

void update_display(app_t *a)
{
    uint16_t t = a->running
        ? a->countdown
        : (a->minutes * 60 + a->seconds);

    uint8_t min = t / 60;
    uint8_t sec = t % 60;

    if (!a->blink)
    {
        if (a->edit_seconds)
            sec = 0xFF;
        else
            min = 0xFF;
    }

    tm1637_display_time(min, sec, a->colon);
}

/* ===================== MAIN ===================== */

int main(void)
{
    DDRB |= (1 << LED_PIN);
    PORTB &= ~(1 << LED_PIN);

    DDRB &= ~((1 << BTN_INC) | (1 << BTN_DEC) | (1 << BTN_START));
    PORTB |= (1 << BTN_INC) | (1 << BTN_DEC) | (1 << BTN_START);

    timer1_init();
    sei();

    tm1637_init(CLK_PIN, DIO_PIN);
    tm1637_set_brightness(4);

    app_t app = {
        .minutes = 0,
        .seconds = 30,
        .colon = 1,
        .blink = 1
    };

    button_t btn_inc   = {1};
    button_t btn_dec   = {1};
    button_t btn_start = {1};

    uint32_t last_sec = 0;
    uint32_t last_blink = 0;

    while (1)
    {
        handle_buttons(&app, &btn_inc, &btn_dec, &btn_start);
        update_timer(&app, &last_sec);
        update_blink(&app, &last_blink);
        update_display(&app);
    }
}
