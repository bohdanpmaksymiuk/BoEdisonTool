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

int DIN=0;// 	= 14;
int CS=0;// 		= 15;
int CLK=0;// 	= 16;
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

//void setupMax(int din,int cs,int clk)
//{
//	DIN = din;
//	CS = cs;
//	CLK = clk;
//}

void putByte(byte data)
{
  byte i = 8;
  byte mask;
  while(i > 0)
  {
	mask = 0x01 << (i - 1);						// get bitmask
	digitalWrite(CLK, LOW);		// tick
	digitalWrite(DIN, (data & mask) ? HIGH : LOW);
	digitalWrite(CLK, HIGH);		// tock
	i--;
  }
}
/*
void fill(byte pattern)
{
	byte e = 0;
    for (e=1; e<=8; e++)
    {    // empty registers, turn all LEDs off
        //maxAll(e,1);
    }
} // fill
*/
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
{
  	digitalWrite(CS, LOW);
	int c =0;
  	for (c =1; c<= maxInUse; c++)
	{
 		putByte(reg);  // specify register
  		putByte(col);//((data & 0x01) * 256) + data >> 1); // put data
    }
  	digitalWrite(CS,HIGH);
	usleep(250); //time it takes for the register to "take"
}

void maxOne(byte maxNr, byte reg, byte col)
{
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

void initMax (int din,int cs,int clk)
{
	DIN = din;
	CS = cs;
	CLK= clk;

	digitalWrite(CLK, LOW);
	digitalWrite(DIN,LOW);
	digitalWrite(CLK,LOW);
	maxAll(max7219_reg_shutdown, 0x01);    // 0x01 not in shutdown mode
  	maxAll(max7219_reg_scanLimit, 0x07);   // 0x07
  	maxAll(max7219_reg_decodeMode, 0x00);  // using an led matrix (not digits)
  	maxAll(max7219_reg_shutdown, 0x01);    // 0x01 not in shutdown mode
  	maxAll(max7219_reg_displayTest, 0x00); // no display test
	maxAll(max7219_reg_intensity, 0x00 & 0x0f);    // the first 0x0f is the value you can set
	byte e = 0;
	for (e=1; e<=8; e++)
	{
     	maxAll(e,0);
  	}
}

void boMAX_TEST()
{
	printf("setup MAx \n");
  	initMax(16,15,14);

	printf("Setup Complete \n");
  	//if you use more then one max7219 the second one should look like this
	int i = 1;
	while (1)
	{
		maxAll(max7219_reg_shutdown, 0x01);
  		maxOne(i,1,170);                       //  + - - - - - - -
  		maxOne(i,2,85);                       //  - + - - - - - -
  		maxOne(i,3,170);                       //  - - + - - - - -
  		maxOne(i,4,85);                       //  - - - + - - - -
  		maxOne(i,5,170);                      //  - - - - + - - -
  		maxOne(i,6,85);                      //  - - - - - + - -
  		maxOne(i,7,170);                      //  - - - - - - + -
  		maxOne(i,8,85);                     //  - - - - - - - +
		bsleep(500);
		//maxAll(max7219_reg_shutdown, 0x01);
		i = i +1;
		if (i ==5) { i = 0; }
		maxOne(i,1,128);
		maxOne(i,2,64);
		maxOne(i,3,32);
		maxOne(i,4,16);
		maxOne(i,5,8);
		maxOne(i,6,4);
		maxOne(i,7,2);
		maxOne(i,8,1);
		bsleep(500);
		printf("loop /n");
//usleep(20000);
	}
}
