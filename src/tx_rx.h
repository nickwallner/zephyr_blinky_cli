#ifndef TX_RX_H_
#define TX_RX_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>

extern struct k_msgq usr_in;
extern struct k_msgq usr_out;
extern struct k_msgq tokens;

#define USR_IN_BUF_SIZE 128

#define STACKSIZE 512 // stack size for threads

#define TX_THREAD_PRIO 5
#define RX_THREAD_PRIO 5

/* get usart2 device instance */
#define USART2_NODE DT_NODELABEL(usart2)
const struct device *usart2_dev = DEVICE_DT_GET(USART2_NODE);

void tx_thread(); // sending (transmission) thread entry function
void rx_thread(); // receiving thread entry function

void uart_callback(const struct device *, void *); // callback for uart interrupts

/* statically define and start threads for tx and rx, no delay */
K_THREAD_DEFINE(tx, STACKSIZE, tx_thread, NULL, NULL, NULL, TX_THREAD_PRIO, 0, 0);
K_THREAD_DEFINE(rx, STACKSIZE, rx_thread, NULL, NULL, NULL, RX_THREAD_PRIO, 0, 0);

#endif // TX_RX_H_