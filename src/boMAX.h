#ifndef BOMAX_H
#define BOMAX_H
typedef unsigned char byte;
void initMax (int din,int cs,int clk);
void maxAll(byte reg, byte col);
void maxOne(byte maxNr, byte reg, byte col);
void boMAX_TEST();

#endif

