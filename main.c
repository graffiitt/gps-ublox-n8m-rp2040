/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "gps_ublox/gps.h"
#include "pico/cyw43_arch.h"

#include "FreeRTOS.h"
#include "task.h"

#ifndef RUN_FREE_RTOS_ON_CORE
#define RUN_FREE_RTOS_ON_CORE 0
#endif

#define BLINK_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)
#define BLINK_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

#define LED_DELAY_MS 2000

static void pico_set_led(bool led_on)
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
}

void blink_task(__unused void *params)
{
    bool on = false;
    printf("blink_task starts\n");
#if defined(CYW43_WL_GPIO_LED_PIN)
    hard_assert(cyw43_arch_init() == PICO_OK);
    pico_set_led(false); // make sure cyw43 is started
#endif
    while (true)
    {
#if configNUMBER_OF_CORES > 1
        static int last_core_id = -1;
        if (portGET_CORE_ID() != last_core_id)
        {
            last_core_id = portGET_CORE_ID();
            printf("blink task is on core %d\n", last_core_id);
        }
#endif
        pico_set_led(on);
        on = !on;

        vTaskDelay(LED_DELAY_MS);
    }
}

int main()
{
    timer_hw->dbgpause = 0x2;

    stdio_init_all();
    TaskHandle_t task;
    xTaskCreate(blink_task, "BlinkThread", BLINK_TASK_STACK_SIZE, NULL, BLINK_TASK_PRIORITY, &task);

    gps_init();

    vTaskStartScheduler();

    while (1)
    {
    }

    return 0;
}
