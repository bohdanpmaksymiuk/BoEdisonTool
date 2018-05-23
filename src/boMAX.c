#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "mraa.h"
#include "stdbool.h"
#include "boGPIO.h"

typedef unsigned char byte;

int DIN 	= 0;
int CS 		= 20;
int CLK 	= 14;
int maxInUse = 4;    //change this variable to set how many MAX7219's you'll use

 // define max7219 registers
byte max7219_reg_noop        = 0x00;
byte max7219_reg_digit0      = 0x01;
byte max7219_reg_digit1      = 0x02;
byte max7219_reg_digit2      = 0x03;
byte max7219_reg_digit3      = 0x04;
byte max7219_reg_digit4      = 0x05;
byte max7219_reg_digit5      = 0x06;
byte max7219_reg_digit6      = 0x07;
byte max7219_reg_digit7      = 0x08;
byte max7219_reg_decodeMode  = 0x09;
byte max7219_reg_intensity   = 0x0a;
byte max7219_reg_scanLimit   = 0x0b;
byte max7219_reg_shutdown    = 0x0c;
byte max7219_reg_displayTest = 0x0f;

void putByte(byte data)
{
  byte i = 8;
  byte mask;

  while(i > 0) {
  	mask = 0x01 << (i-1);     // get bitmask
    digitalWrite(CLK,0);     // tick
	if (data & mask)
	{
        printf("1");
    	digitalWrite(DIN,1);// send 1
    }
		else
	{
    	printf("0");
		digitalWrite(DIN,0); // send 0
	}
    digitalWrite(CLK,1);   // tock
	printf("\n");
    i=i-1;                       // move to lesser bit
  }
}
/**
void maxSingle(byte reg, byte col)
{
	//maxSingle is the "easy"  function to use for a     //single max7219
	digitalWrite(load, LOW);       // begin
  	putByte(reg);                  // specify register
  	putByte(col);//((data & 0x01) * 256) + data >> 1); // put dat
	// and load da shit
  digitalWrite(load,HIGH);
}
*/

void maxAll(byte reg, byte col)
{    // initialize  all  MAX7219's in the system
	int c = 0;
  	digitalWrite(CS, 0);
  	for ( c =1; c<= maxInUse; c++)
	{
		//printf("REG  %d:",reg);
 		//putByte(reg);  // specify register
		printf("DATA %d:",col);
  		putByte(col);//((data & 0x01) * 256) + data >> 1); // put data
        printf("REG  %d:",reg);
        putByte(reg);  // specify register

    }
  	digitalWrite(CS,1);

}

void maxOne(byte maxNr, byte reg, byte col)
{
	//maxOne is for adressing different MAX7219's,
	//whilele having a couple of them cascaded
	int c = 0;
  	digitalWrite(CS, LOW);
 	for ( c = maxInUse; c > maxNr; c--) {
    	putByte(0);    // means no operation
    	putByte(0);    // means no operation
  	}
	putByte(reg);  // specify register
  	putByte(col);//((data & 0x01) * 256) + data >> 1); // put data

  	for ( c =maxNr-1; c >= 1; c--) {
    	putByte(0);    // means no operation
    	putByte(0);    // means no operation
  	}
	digitalWrite(CS,HIGH);
}

void setup ()
{
	digitalWrite(CLK, HIGH);
	//maxAll(max7219_reg_displayTest, 0x01);
	//usleep(10000000);
	maxAll(max7219_reg_displayTest, 0x00);
  	//maxAll(max7219_reg_scanLimit, 0x07);   // 0x07
  	//maxAll(max7219_reg_decodeMode, 0x00);  // using an led matrix (not digits)
  	//maxAll(max7219_reg_shutdown, 0x01);    // 0x01 not in shutdown mode
  	//maxAll(max7219_reg_displayTest, 0x01); // no display test
 	byte e = 0;
	for (e=1; e<=8; e++)
	{    // empty registers, turn all LEDs off
    	maxAll(e,1);
  	}
  	//maxAll(max7219_reg_intensity, 0x07 & 0x0f);    // the first 0x0f is the value you can set
                                                  // range: 0x00 to 0x0f
}

void boMAX_TEST()
{
  	setup();

  	//if you use more then one max7219 the second one should look like this
/*	while (1)
	{
  		maxOne(2,1,1);                       //  + - - - - - - -
  		maxOne(2,2,2);                       //  - + - - - - - -
  		maxOne(2,3,4);                       //  - - + - - - - -
  		maxOne(2,4,8);                       //  - - - + - - - -
  		maxOne(2,5,16);                      //  - - - - + - - -
  		maxOne(2,6,32);                      //  - - - - - + - -
  		maxOne(2,7,64);                      //  - - - - - - + -
  		maxOne(2,8,128);                     //  - - - - - - - +
  		//
  		usleep(2000);
	}
*/
}
