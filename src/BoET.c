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
#include "boGPIO.h"
#include "boMAX.h"

// -----------------------------------------------------------------------------------------------------
// Main Program
//

int main(int argc, char * argv[])
{
	// ................................

	// ................................
	int file=init_device(I2C_DEV_NAME);

	// ................................

	mraa_init();

	// .............................
    if (argc > 1)
	{
		if (strcmp(argv[1],"-v") == 0)
		{
			printf("%.1f\n",getVoltage()/1000);
		} else
		if (strcmp(argv[1],"-p") == 0)
		{
			printf("%.1f\n",getBatteryPercentage());

		} else
		if (strcmp(argv[1],"-t") == 0)
		{
			printf("%i\n",getTemp(file));
		} else
		if (strcmp(argv[1],"-w") == 0)
		{

			int iopin = strtol(argv[2], NULL, 10);
			int valueiopin = strtol(argv[3], NULL, 10);
			writePin(iopin,valueiopin);
		} else
	        if (strcmp(argv[1],"-R") == 0)
        	{
			if (argc==2) 
			{
				printPins();
			} else
			{
	            		int iopin = strtol(argv[2], NULL, 10);
        	    		printf("%d\n",readPin(iopin,false));
			}
        	} else
		if (strcmp(argv[1],"-r") == 0)
        	{
        		if (argc==2)
            		{
            			printPins();
            		} else
            	{
            		int iopin = strtol(argv[2], NULL, 10);
                	printf("%d\n",readPin(iopin,true));
            	}
        	} else
		{
			//boMAX_TEST();
			printAllPins();
		}
/*
		{
			while (1)
			{
				Triplet a_bias = {0}, g_bias = {0}, m_bias = {0};
				FTriplet gyro, mag, acc;
				FTriplet m_scale = {1,1,1};
				init_gyro(file, GYRO_SCALE_245DPS); init_mag(file, MAG_SCALE_2GS); init_acc(file, ACCEL_SCALE_2G);
				read_gyro (file, g_bias, GYRO_SCALE_245DPS, &gyro);
			    read_mag (file, m_bias, m_scale, MAG_SCALE_2GS, &mag);
			    read_acc (file, a_bias, ACCEL_SCALE_2G, &acc);
			    printf ("gyro: %9.2f %9.2f %9.2f | ", gyro.x, gyro.y, gyro.z);
			    printf ("mag:  %9.2f %9.2f %9.2f | ", (mag.x)*1000, (mag.y)*1000, (mag.z)*1000);
			    printf ("acc:  %9.2f %9.2f %9.2f\n", (acc.x)*1000, (acc.y)*1000, (acc.z)*1000);
				usleep(500000);
			}
		}*/
	} else
	{
		printf("BoET v1.0\n");
		printf("By Bohdan P Maksymiuk\n");
		printf("--------------------------------------------------------------------\n");
		printf("MRAA Version: %s\n", mraa_get_version());
		printf("-v : \e[32;1m%4.1f\e[0m : Voltage\n",getVoltage()/1000);
		printf("-p : \e[32;1m%4.1f\e[0m : Battery Percentage\n",getBatteryPercentage());
		printf("-t : \e[32;1m%4i\e[0m : Temperature\n",getTemp(file));
		printf("-w <pin> <value>      : Write 1 or 0 to a pin, change ping to write mode \n");
		printf("-r <pin>              : Read 1 or 0 from a pin, change pin to read mode \n");
		printf("-R <pin>              : Read 1 or 0 from a pin, dont change pin to read only\n");
		printf("-R                    : Show all pin values, dont change pins to read only \n");
		printf("-r \n");
		printPins();
	}
	return 0;
}
