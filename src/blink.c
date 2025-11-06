#include "blink.h"

/* infinite blinking loop for the blinky_thread */
void blink_loop(void *interval, void *, void *)
{
    while (1)
    {
        gpio_pin_toggle_dt(&led);
        k_msleep(*(uint32_t *)interval);
    }
}

/* blinking thread that handles led control */
void blink_thread()
{
    uint32_t blink_interval = 0;

    /* create a thread for blinking the led at a specific interval
    but immediately suspend it! we don't want the led to start blinking right away */
    blinky_thread_id = k_thread_create(&blinky_thread, blinky_thread_stack, K_THREAD_STACK_SIZEOF(blinky_thread_stack),
                                            blink_loop, (void *)&blink_interval, NULL, NULL, BLINK_THREAD_PRIO, 0, K_NO_WAIT);
    k_thread_suspend(blinky_thread_id);

    /* verify led gpio port is ready */
    if (!gpio_is_ready_dt(&led))
    {
        k_msgq_put(&usr_out, "led gpio port is not ready! terminating blink thread...\n\r", K_NO_WAIT);
        return;
    }

    /* configure led gpio pin for output */
    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT) != 0)
    {
        k_msgq_put(&usr_out, "failed to enable output for led gpio port! terminating blink thread...\n\r", K_NO_WAIT);
        return;
    }

    /* buffer to hold command tokens */
    char buf[TOKENS_BUF_SIZE];

    /* wait indefinitely for tokens to be placed in
    the tokens message queue, parsing them immediately
    as they arrive */
    while (k_msgq_get(&tokens, &buf, K_FOREVER) == 0)
    {
        if (strcmp(buf, "on") == 0)
        {
            /* suspend blinky thread (it may or may not already be suspended)
            and set led pin high */
            k_thread_suspend(blinky_thread_id);
            gpio_pin_set_dt(&led, 1);
        }
        else if (strcmp(buf, "off") == 0)
        {
            /* suspend blinky thread (it may or may not already be suspended)
            and set led pin low */
            k_thread_suspend(blinky_thread_id);
            gpio_pin_set_dt(&led, 0);
        }
        else if (strcmp(buf, "toggle") == 0)
        {
            /* suspend blinky thread (it may or may not already be suspended)
            and toggle led pin */
            k_thread_suspend(blinky_thread_id);
            gpio_pin_toggle_dt(&led);
        }
        else if (strcmp(buf, "blink") == 0)
        {
            /* get next token, which should be blinking interval in ms */
            k_msgq_get(&tokens, &buf, K_NO_WAIT);
            
            /* verify that the ENTIRE token is numeric */
            bool isnumeric = true;
            for (int i = 0; i < strlen(buf); i++)
            {
                if (!isdigit(buf[i]))
                {
                    k_msgq_put(&usr_out, "invalid blinking interval! value must be numeric\n\r"
                                         "use 'led help' for help\n\r", K_NO_WAIT);
                    isnumeric = false;
                    break;
                }
            }

            /* if entire token was numeric, update the blink interval
            and then resume the blinking thread (it may or may not already be active) */
            if (isnumeric)
            {
                blink_interval = atoi(buf);
                k_thread_resume(blinky_thread_id);
            }
        }
        else if (strcmp(buf, "help") == 0)
        {
            k_msgq_put(&usr_out, "supported commands:\n\r"
                                 "led on: turn the led on\n\r"
                                 "led off: turn the led off\n\r"
                                 "led toggle: toggle the led\n\r"
                                 "led blink <interval>: blink the led every <interval> ms\n\r"
                                 "led help: see this message\n\r", K_NO_WAIT);
        }
        else
        {
            k_msgq_put(&usr_out, "invalid option! use 'led help' for help\n\r", K_NO_WAIT);
        }

        /* purge any additional tokens that may exist; they are irrelevant */
        k_msgq_purge(&tokens);
    }
}