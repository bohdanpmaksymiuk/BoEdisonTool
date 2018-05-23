#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "edison-9dof-i2c.h"
#include "mraa.h"
#include "stdbool.h"
#include <errno.h>
#include <signal.h>

#define BAT_FULL        4180
#define BAT_NORMAL      3680
#define BAT_LOW         3400
#define BAT_CRIT        3250
#define BAT_DEAD        2950

#define BAT_FULL_PERCENT        100
#define BAT_NORMAL_PERCENT      50
#define BAT_LOW_PERCENT         10
#define BAT_DEAD_PERCENT        0

float interpolatePercentage(float level, float above, float below, float above_percent, float below_percent)
{
        float offset = level - below;
        float range = above - below;
        float fraction = offset / range;

        float percentage_range = above_percent - below_percent;
        float result = percentage_range * fraction;
        result += below_percent;
        return result;
}

float calculatePercentage(float level)
{
        if (level > BAT_FULL)
        {
                return 100;
        }
        else if (level > BAT_NORMAL)
        {
                return interpolatePercentage(level, BAT_FULL, BAT_NORMAL, BAT_FULL_PERCENT, BAT_NORMAL_PERCENT);
        }
        else if (level > BAT_LOW)
        {
                return interpolatePercentage(level, BAT_NORMAL, BAT_LOW, BAT_NORMAL_PERCENT, BAT_LOW_PERCENT);
        }
        else
        {
                return interpolatePercentage(level, BAT_LOW, BAT_DEAD, BAT_LOW_PERCENT, BAT_DEAD_PERCENT);
        }
}

// --------------------------------------------------------------------------
// Voltage
//
float read_raw();

float getVoltage()
{
    float raw = read_raw();
    return raw * 4500.0f / 1024.0f;
}


float getBatteryPercentage()
{
	return calculatePercentage(getVoltage()); 
}

int process_raw(char * str)
{
    char * start = strchr(str, '=');
    start += 2;
    char * end = strchr(start, '\n');
    *end = 0;
    return atoi(start);
}

float read_raw()
{
    // https://github.com/cjo20/EdisonVoltage
    // There is a system device, bcove_adc, that is connected to various voltage and temperature input lines.
    //  These can be read using /sys/devices/platform/bcove_adc/basincove_gpadc.
    //  /sys/devices/platform/bcove_adc/basincove_gpadc/channel reads a bitmask of channels to report. Channel 0 is the battery voltage.
    //  /sys/devices/platform/bcove_adc/basincove_gpadc/sample triggers an ADC conversion to take place.
    //  /sys/devices/platform/bcove_adc/basincove_gpadc/result contains the result of the ADC conversion.
    //  It is an integer between 0 and 1023. A result of 1023 is roughly equivalent to 4.5v.

    char exec = '1';
    int i = 0;

    int num_samples = 0;
    int total = 0;

    for (i = 0; i < 10; ++i)
    {
        int sample = open("/sys/devices/platform/bcove_adc/basincove_gpadc/sample", O_WRONLY);

        if (sample)
        {
            int channels = open("/sys/devices/platform/bcove_adc/basincove_gpadc/channel", O_WRONLY);
            write(channels, &exec, 1);
            close(channels);

            write(sample, &exec, 1);
            close(sample);

            int results = open("/sys/devices/platform/bcove_adc/basincove_gpadc/result", O_RDONLY);
                char data[1024];
                read(results, &data, 1024);
                close(results);
                total += process_raw(data);
            num_samples++;
        }

    }
        return total / (float)num_samples;
}
