#include "tx_rx.h"

/* print a string using a usart device */
static inline void uart_print_str(const struct device *dev, char *buf)
{
    int len = strlen(buf);

    while (len)
    {
        uart_poll_out(dev, *buf);
        buf++;
        len--;
    }
}

/* the tx thread handles transmitting data from the
usr_out message queue to the usart device */
void tx_thread()
{
    /* get buffer size from the usr_out message queue */
    struct k_msgq_attrs attrs;
    k_msgq_get_attrs(&usr_out, &attrs);
    char buf[attrs.msg_size];

    /* wait indefinitely for messages to be placed
    in the msgs message queue, messages are printed
    immediately as they are placed in the queue */
    while (k_msgq_get(&usr_out, &buf, K_FOREVER) == 0)
    {
        uart_print_str(usart2_dev, buf);
    }
}

/* the rx thread handles receiving input from the
usr_in message queue */
void rx_thread()
{
    /* set callback function for usart2 interrupts */
    uart_irq_callback_user_data_set(usart2_dev, uart_callback, (void *)usart2_dev);
    /* enable rx interrupts for usart2 */
    uart_irq_rx_enable(usart2_dev);

    char buf[USR_IN_BUF_SIZE];
    char *token;

    /* wait indefinitely for messages to be placed in the
    usr_in message queue, they are parsed immediately
    as they arrive */
    while (k_msgq_get(&usr_in, &buf, K_FOREVER) == 0)
    {
        token = strtok(buf, " ");

        /* check if first token is "led" */
        if (strcmp(token, "led") == 0)
        {
            /* if first token was "led", place all remaining
            tokens in tokens message queue */
            while (token != NULL)
            {
                token = strtok(NULL, " ");
                k_msgq_put(&tokens, token, K_NO_WAIT);
            }
        }
        else
        {
            /* send error message to sending thread */
            k_msgq_put(&usr_out, "unrecognized command\n\r"
                                 "use 'led help' for help\n\r", K_NO_WAIT);
        }
    }
}

/* callback function for uart interrupts */
/* this callback reads each character from the usart device
that generated the interrupt until a newline character is found
and then stores the resulting string into the usr_in message queue */
void uart_callback(const struct device *dev, void *user_data)
{
    char ch;
    static char buf[USR_IN_BUF_SIZE];
    static int buf_pos = 0; // index for buffer

    uart_irq_update(dev); // start processing interrupts
    if (uart_irq_rx_ready(dev) == 0) return; // check that there is data in the receive buffer

    /* read data from the receive buffer one char at a time */
    while (uart_fifo_read(dev, &ch, 1) == 1)
    {
        /* if a newline is read, null terminate the string and 
        place the string in the message queue */
        if ((ch == '\n' || ch == '\r') && (buf_pos > 0))
        {
            buf[buf_pos] = '\0';
            k_msgq_put(&usr_in, &buf, K_NO_WAIT); // msg is discarded if queue is full
            buf_pos = 0;
        }
        else if (buf_pos < (sizeof(buf) - 1))
        {
            buf[buf_pos] = ch;
            buf_pos++;
        }
    }
}