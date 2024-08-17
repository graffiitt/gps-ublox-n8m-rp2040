#ifndef UART_CONFIG_H
#define UART_CONFIG_H

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
 
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "uart_rx.pio.h"
#include "pico/async_context_freertos.h"

// Priorities of our threads - higher numbers are higher priority
#define UART_TASK_PRIORITY (tskIDLE_PRIORITY + 4UL)

// Stack sizes of our threads in words (4 bytes)
#define UART_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_TX_PIN 16
#define UART_RX_PIN 17

#define FIFO_SIZE 80

void uart_configure();

#endif