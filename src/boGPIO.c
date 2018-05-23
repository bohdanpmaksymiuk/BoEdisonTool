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
#include "boVoltage.h"


// ----------------------------------------------
float readAPin(int iopin)
{
    mraa_aio_context aio;
    aio = mraa_aio_init(iopin);
    if (aio == NULL) { fprintf(stderr,"Pin not valid\n"); exit(1); }
    float value = mraa_aio_read_float(aio);
    mraa_aio_close(aio);
    return value;
}


int readPin(int iopin,bool sc)
{
    mraa_gpio_context gpio;
    gpio = mraa_gpio_init(iopin);
	if (gpio == NULL)
    {
    	fprintf(stderr, "Pin not valid\n");
        exit(1);
    }
    if (sc)
    {
        mraa_result_t r = mraa_gpio_dir(gpio, MRAA_GPIO_IN);
        if (r != MRAA_SUCCESS)  { mraa_result_print(r); exit(1); }
    }
    int v = mraa_gpio_read(gpio);
    return v;
}

void writePin(int iopin,int valueiopin)
{
    mraa_gpio_context gpio;
    gpio = mraa_gpio_init(iopin);
    if (gpio == NULL) { fprintf(stderr, "Pin not valid\n"); exit(1); }
    mraa_result_t r = mraa_gpio_dir(gpio, MRAA_GPIO_OUT);
    if (r != MRAA_SUCCESS)  { mraa_result_print(r); exit(1); }
    r = mraa_gpio_mode(gpio, MRAA_GPIO_STRONG);
    if (r != MRAA_SUCCESS)  { mraa_result_print(r); exit(1); }
	r = mraa_gpio_write(gpio, valueiopin);
    if (r != MRAA_SUCCESS)  { mraa_result_print(r); exit(1); }
}

void digitalWrite(int iopin,int valueiopin)
{
	//usleep(1000);
	writePin(iopin,valueiopin);
	//usleep(1000);
}

int printPin(int iopin)
{
    mraa_gpio_context gpio;
    gpio = mraa_gpio_init(iopin);
    if (gpio == NULL)
	{
		//printf(" %4d \e[90;1m%6s\e[0m \e[90;1m%-2s\e[0m (\e[32;1m%s\e[0m)",iopin,"","","-");
		return 0;
	} else
	{
    	int v = mraa_gpio_read(gpio);
    	mraa_gpio_dir_t dir = MRAA_GPIO_IN;
    	mraa_result_t r =mraa_gpio_read_dir(gpio, &dir);
	    if (r != MRAA_SUCCESS)  { mraa_result_print(r); exit(1); }
    	char* dir1= "";
    	if (dir==MRAA_GPIO_IN)      {dir1="I";}
    	if (dir==MRAA_GPIO_OUT)     {dir1="O";}
    	if (dir==MRAA_GPIO_OUT_HIGH)    {dir1="OH";}
    	if (dir==MRAA_GPIO_OUT_LOW)     {dir1="OL";}
    	printf(" %4d \e[90;1m%6s\e[0m \e[90;1m%-2s\e[0m (\e[32;1m%d\e[0m)",iopin,mraa_get_pin_name(iopin),dir1,v);
		return 1;
	}
}

void printAllPins()
{
	int i=0;
	int j=0;
	while (i<500)
	{
		j = j+ printPin(i);
		if (j >= 4)
		{
			printf("\n"); j = 0;
		}
		i=i+1;
	}
}

void printPins()
{
    printf(" %4s \e[90;1m%6s\e[0m \e[90;1m%-2s\e[0m (\e[32;1m%s\e[0m)","3v","","","1");
    printf(" %4s \e[90;1m%6s\e[0m \e[90;1m%-2s\e[0m (\e[32;1m%s\e[0m)\n","GND","","","0");
    printf(" %4s \e[90;1m%6s\e[0m \e[90;1m%-2s\e[0m  \e[32;1m%s\e[0m  ","1v","","","");
    printf(" %4s \e[90;1m%6s\e[0m \e[90;1m%-2s\e[0m (\e[32;1m%2.2f\e[0m)\n","VSYS","","",getVoltage()/1000);
    printPin(35); printPin(31); printf("\n");
    printPin(26); printPin(45); printf("\n");
    printPin(25); printPin(32); printf("\n");
    printPin(13); printPin(46); printf("\n");
    printPin(21); printPin(33); printf("\n");
    printPin(0); printPin(47); printf("\n");
    printPin(20); printPin(48); printf("\n");
    printPin(14); printPin(36); printf("\n");
}
