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
		} else if (strcmp(argv[1],"-p") == 0)
		{
			printf("%.1f\n",getBatteryPercentage());

		} else if (strcmp(argv[1],"-t") == 0)
		{
			printf("%i\n",getTemp(file));
		} else if (strcmp(argv[1],"-w") == 0)
		{

			int iopin = strtol(argv[2], NULL, 10);
			int valueiopin = strtol(argv[3], NULL, 10);
			writePin(iopin,valueiopin);
		} else if (strcmp(argv[1],"-R") == 0)
       	{
			if (argc==2) 
			{
				printPins();
			} else
			{
	            		int iopin = strtol(argv[2], NULL, 10);
        	    		printf("%d\n",readPin(iopin,false));
			}
       	} else if (strcmp(argv[1],"-r") == 0)
        {
        	if (argc==2)
           	{
        		printPins();
        	} else
          	{
            	int iopin = strtol(argv[2], NULL, 10);
               	printf("%d\n",readPin(iopin,true));
        	}
        } else if (strcmp(argv[1],"-m")==0)
		{
			if  (argc==5)
			{
					initMax(strtol(argv[2], NULL, 10),strtol(argv[3], NULL, 10),strtol(argv[4], NULL, 10));
			}
			initMax (14,15,16);
 			maxOne(1,1,170);                       //  + - - - - - - -
	   	  	maxOne(1,2,85);                       //  - + - - - - - -
	      	maxOne(1,3,170);                       //  - - + - - - - -
    	   	maxOne(1,4,85);                       //  - - - + - - - -
        	maxOne(1,5,170);                      //  - - - - + - - -
  	    	maxOne(1,6,85);                      //  - - - - - + - -
        	maxOne(1,7,170);                      //  - - - - - - + -
        	maxOne(1,8,85);                     //  - - - - - - - +
		} else
		{
			boMAX_TEST();
					//printAllPins();
		}
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
		printAllPins();
	}
	return 0;
}
