## Welcome to Blinky Zephyr!

This is a small project I made to learn the basics of using the Zephyr RTOS.
Specifically, I focused on how I can use threads to have various parts of code
running together. I was able to get a good baseline understanding of how the 
Zephyr RTOS codebase is organized, how Zephyr's build system works, and how
applications that use an RTOS can be structured differently compared to normal
C programs (i.e you don't need a `main()` function!!).

This code uses 3 threads, a sending thread (`tx_thread`), a receiving thread (`rx_thread`), and 
a led control thread (`blink_thread`), to make a very simple command line application over USART
to control an led. The led control thread `blink_thread` also spawns an additional thread,
`blinky_thread`, whose sole purpose is infinitely blinking an led.

## Basic Algorithm for each Thread
Sending Thread (`tx_thread`):
1. wait indefinitely to receive output data from other threads via the `usr_out` message queue
2. when data is received, print it via USART2
3. once output is finished, return to waiting state for more data to arrive

Receiving Thread (`rx_thread`):
1. when the thread is first started, it sets the USART2 ISR callback function and enables
receiving interrupts for USART2
2. wait indefinitely to receive user input via the `usr_in` message queue
3. when input is received, parse first token from `usr_in`, using `' '` (space) as the delimiter
4. if first token is "led":
    - parse remaining tokens and place them in the `tokens` message queue individually
    - return to waiting state for more user input
5. if first token is not "led":
    - place error message in the `usr_out` message queue
    - return to waiting state for more user input

Blinking Thread (`blink_thread`):
1. when the blinking thread is first started, it initializes `blinky_thread`, which is a separate
thread that handles blinking the led on an infinite loop
2. then, it initializes the GPIO port for the led, and set the led's gpio pin to output mode
3. wait indefinitely to receive command tokens via the `tokens` message queue
4. when tokens are received, read first token:
    - if token == "on": turn led on
    - if token == "of": turn led off
    - if token == "blink":
        - read third token for blinking interval
        - turn on led to blink at specified interval
    - else, send unrecognized command error message to sending thread
5. discard all other tokens from `tokens` that may have been sent
6. return to waiting state for more tokens to be received

## Data Passing
#### `usr_in` message queue

`usr_in` is the primary means for receiving input data from the user.

Each time data is received by USART2, an interrupt is generated, and the interrupt callback
reads characters from USART2's receiving buffer and places the resulting
string in `usr_in`.

Any messages placed in `usr_in` will be immediately parsed by the `rx_thread`.

If `usr_in` is full, the message will be discarded

#### `usr_out` message queue

`usr_out` is used by all three threads as the primary method for
outputting data to the user.

Any messages placed in the `usr_out` message queue will be immediately
printed to output by the `tx_thread`.

If `usr_out` is full, the message will be discarded

#### `tokens` message queue

`tokens` is used by the `rx_thread` to pass parsed command
tokens to the `blink_thread`.

Any messages placed in `tokens` will be immediately parsed by the
`blink_thread`.