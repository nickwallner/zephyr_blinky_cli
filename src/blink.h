#ifndef BLINK_H_
#define BLINK_H_

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include <ctype.h>
#include <stdlib.h>

extern struct k_msgq usr_out;
extern struct k_msgq tokens;

#define TOKENS_BUF_SIZE 128

#define STACKSIZE 512

#define BLINK_THREAD_PRIO 5
#define BLINKY_THREAD_PRIO 5

K_THREAD_STACK_DEFINE(blinky_thread_stack, STACKSIZE);

/* get gpio spec for led0 (green user led) */
#define LED_NODE DT_ALIAS(led0)
const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

typedef enum
{
    LED_ON     = 0,
    LED_OFF    = 1,
    LED_TOGGLE = 2,
    LED_BLINK  = 3,
} blink_mode;

struct k_thread blinky_thread;
k_tid_t blinky_thread_id;

void blink_loop(void *, void *, void *); // infinite blinking loop for blinky_thread
void blink_thread(); // thread that handles led control

K_THREAD_DEFINE(blink, STACKSIZE, blink_thread, NULL, NULL, NULL, BLINK_THREAD_PRIO, 0, 0);

#endif // BLINK_H_