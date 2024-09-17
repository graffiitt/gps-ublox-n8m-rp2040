#ifndef GPS_UBLOX_H
#define GPS_UBLOX_H

#include "uart_config.h"
#include <math.h>
#include <stdlib.h>

#define pi 3.14159265358979323846
#define SIZE_BUFFER_GPS 5
#define SIZE_BUFFER_GPS_SP 50

#define TIME_RMC 1
#define STATUS_RMC 2 // A - sucsess V - bad
#define LATITUDE_RMC 3
#define LONGTITUDE_RMC 5

struct Time
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
};

struct Position
{
    double latitude;
    double longtitude;
};

void gps_init();
void gps_configure();
void gps_power(bool state);

void nmea_parcer(uint8_t *str);
double calc_distance(double lon, double lat);

#endif