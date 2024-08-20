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

#define BLINK_TASK_PRIORITY (tskIDLE_PRIORITY + 16UL)
#define BLINK_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

#define LED_DELAY_MS 20

void vApplicationIdleHook(void)
{
    gpio_put(11, 1);
    gpio_put(11, 0);
}

void vApplicationTickHook(void)
{
    gpio_put(13, 1);
    gpio_put(13, 0);
}

static void pico_set_led(bool led_on)
{
    gpio_put(12, led_on);
   // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
}

void blink_task(__unused void *params)
{
    vTaskSetApplicationTaskTag(NULL, (void *)1); // for logic analizer

    bool on = false;

    // hard_assert(cyw43_arch_init() == PICO_OK);
    pico_set_led(false); // make sure cyw43 is started

    while (true)
    {
        pico_set_led(on);
        on = !on;
        vTaskDelay(LED_DELAY_MS);
    }
}

void gpioSetup(uint pin)
{
    gpio_deinit(pin);
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0);
}

void TaskSwitchedIn(int tag)
{
    switch (tag)
    {
    case 0:
        gpio_put(15, 0);
        break;

    case 1:
        gpio_put(14, 0);
        break;

    case 2:
        gpio_put(13, 0);
        break;

    case 3:
        gpio_put(12, 0);
        break;
    }
}

void TaskSwitchedOut(int tag)
{
    switch (tag)
    {
    case 0:
        gpio_put(15, 1);
        break;
    case 1:
        gpio_put(14, 1);
        break;

    case 2:
        gpio_put(13, 1);
        break;

    case 3:
        gpio_put(12, 1);
        break;
    }
}

int main()
{
    timer_hw->dbgpause = 0x2 | 0x3;

    stdio_init_all();

    gpioSetup(11);
    gpioSetup(12);
    gpioSetup(13);
    gpioSetup(14);
    gpioSetup(15);

    TaskHandle_t task;
    xTaskCreate(blink_task, "BlinkThread", BLINK_TASK_STACK_SIZE, NULL, BLINK_TASK_PRIORITY, &task);

    gps_init();

    vTaskStartScheduler();

    while (1)
    {
    }

    return 0;
}
