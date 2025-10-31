#include <zephyr/kernel.h>

/* message queue sizes */
#define USR_IN_BUF_SIZE 128
#define USR_OUT_BUF_SIZE 256
#define TOKENS_BUF_SIZE 128

/* message queues */
K_MSGQ_DEFINE(usr_in, USR_IN_BUF_SIZE, 10, 1); // queue for input msgs from user
K_MSGQ_DEFINE(usr_out, USR_OUT_BUF_SIZE, 10, 1); // queue for output msgs to user
K_MSGQ_DEFINE(tokens, TOKENS_BUF_SIZE, 10, 1); // queue for tokens from user input
