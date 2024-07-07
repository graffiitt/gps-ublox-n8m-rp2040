/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "gps_ublox/gps.h"

int main()
{
    stdio_init_all();
    uart_configure();
    gps_init();
    
    while (1)
    {
    }

    return 0;
}
