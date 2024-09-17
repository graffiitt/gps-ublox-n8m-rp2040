#include "gps.h"

static const uint8_t set5hz[] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xC8, 0x00, 0x01, 0x00, 0x01, 0x00, 0xDE, 0x6A}; //(5Hz)

uint8_t statusGPS = 0;
static uint8_t counter[2] = {0};
static double speed[SIZE_BUFFER_GPS_SP];

double speedGPS;
struct Position position;
struct Time timeGPS;

static void parse_RMC(uint8_t *data);
static void parse_VTG(uint8_t *data);
static double deg2rad(double);
static double rad2deg(double);
static bool checkCRC(const uint8_t *str);
static void avrSpeed();
static void avrPose();

// config pinouts
void gps_init()
{
    uart_configure();
    gpio_deinit(5);
    gpio_init(5);
    gpio_set_dir(5, GPIO_OUT);
}

void gps_configure()
{
    uart_write_blocking(UART_ID, set5hz, sizeof set5hz);
}

void gps_power(bool state)
{
    if (state) 
        gpio_put(5, 1);
    else
        gpio_put(5, 0);
}

bool checkCRC(const uint8_t *str)
{
    uint8_t lenght = strlen((const char *)str);
    if (lenght < 8)
        return 0;
    if (str[0] != '$' || str[lenght - 5] != '*')
        return 0;

    uint8_t checksum = 0;
    for (uint8_t i = 1; i < (lenght - 5); i++)
    {
        checksum ^= str[i];
    }
    char crc[] = {str[lenght - 4], str[lenght - 3]};
    int crcMSG = (int)strtol(crc, NULL, 16);
    if (crcMSG == checksum)
        return 1;
    return 0;
}

void nmea_parcer(uint8_t *str)
{
    if (!checkCRC(str))
        return;
    if (strstr((char *)str, "RMC") != NULL)
    {
        parse_RMC(str);
    }
    if (strstr((char *)str, "VTG") != NULL)
    {
        parse_VTG(str);
    }
}

void parse_RMC(uint8_t *data)
{
    uint8_t count_ch = 0;
    uint8_t num = 0;
    uint8_t i = 0;
    char buff[14];
    memset(buff, '\0', 14);

    while (data[count_ch - 1] != '\n')
    {
        if (data[count_ch] == ',' || data[count_ch] == '*')
        {
            if (buff[0] != '\0')
            {
                switch (num)
                {
                case TIME_RMC:
                {
                    char ch[] = {buff[0], buff[1]};
                    timeGPS.hours = atoi(ch);
                    timeGPS.hours = timeGPS.hours > 20 ? timeGPS.hours - 21 : timeGPS.hours + 3;
                    ch[0] = buff[2];
                    ch[1] = buff[3];
                    timeGPS.minutes = atoi(ch);
                    ch[0] = buff[4];
                    ch[1] = buff[5];
                    timeGPS.seconds = atoi(ch);
                    break;
                }
                case STATUS_RMC:
                {
                    if (buff[0] == 'A')
                    {
                        statusGPS = 1;
                    }
                    else
                    {
                        statusGPS = 0;
                        return;
                    }
                    break;
                }
                case LATITUDE_RMC:
                {
                    if (statusGPS)
                    {
                        char *p2 = strchr(buff, '.');
                        double latitude = atof(p2 - 2) / 60.0;
                        buff[p2 - 2 - buff] = '\0';
                        latitude += atof(buff);
                        position.latitude = latitude;
                    }
                    break;
                }
                case 4:
                    if (buff[0] == 'S')
                    {
                        position.longtitude = -position.longtitude;
                    }
                    break;
                case LONGTITUDE_RMC:
                    if (statusGPS)
                    {
                        char *p2 = strchr(buff, '.');
                        double longtitude = atof(p2 - 2) / 60.0;
                        buff[p2 - 2 - buff] = '\0';
                        longtitude += atof(buff);
                        position.longtitude = longtitude;
                    }

                    break;

                case 6:
                    if (buff[0] == 'W')
                    {
                        position.longtitude = -position.longtitude;
                    }
                    break;
                }
            }
            i = 0;
            num++;
            memset(buff, '\0', 14);
        }
        if (data[count_ch] != ',')
        {
            buff[i] = data[count_ch];
            i++;
        }
        count_ch++;
    }
}

void parse_VTG(uint8_t *data)
{
    uint8_t count_ch = 0;
    uint8_t num = 0;
    uint8_t i = 0;
    char buff[10];
    memset(buff, '\0', 10);

    while (data[count_ch - 1] != '\n')
    {

        if (data[count_ch] == ',' || data[count_ch] == '*')
        {
            if (buff[0] != '\0')
            {
                switch (num)
                {
                case 7:
                {
                    if ((buff[0] != '\0') || statusGPS)
                        speed[counter[1]] = atof(buff);
                    else
                        speed[counter[1]] = 0;
                    avrSpeed();
                    counter[1]++;
                    if (counter[1] >= SIZE_BUFFER_GPS_SP)
                        counter[1] = 0;
                    break;
                }
                }
            }
            i = 0;
            num++;
            memset(buff, '\0', 10);
        }
        if (data[count_ch] != ',')
        {
            buff[i] = data[count_ch];
            i++;
        }
        count_ch++;
    }
}

double calc_distance(double lon, double lat)
{
    static double longtitude = 0;
    static double latitude = 0;

    if (latitude == 0 && latitude == 0)
    {
        longtitude = lon;
        latitude = lat;
        return 0;
    }

    if ((lat == 0) || (lon == 0))
        return 0;

    if ((latitude == lat) && (longtitude == lon))
    {
        return 0;
    }
    else
    {
        double theta;
        double dist = 0.01f;
        theta = longtitude - lon;
        dist = sin(deg2rad(latitude)) * sin(deg2rad(lat)) + cos(deg2rad(latitude)) * cos(deg2rad(lat)) * cos(deg2rad(theta));
        dist = acos(dist);
        dist = rad2deg(dist);
        dist = dist * 60 * 1.1515 * 1.609344;

        if (isinf(dist) == 0)
        {
            longtitude = lon;
            latitude = lat;
            return dist;
        }

        else
            return 0;
    }
}

double deg2rad(double deg)
{
    return (deg * pi / 180);
}

double rad2deg(double rad)
{
    return (rad * 180 / pi);
}

void avrSpeed()
{
    double summ = 0.00f;
    for (int i = 0; i < SIZE_BUFFER_GPS_SP; i++)
    {
        summ += speed[i];
    }
    speedGPS = summ / SIZE_BUFFER_GPS_SP;
}

void avrPose()
{
}
