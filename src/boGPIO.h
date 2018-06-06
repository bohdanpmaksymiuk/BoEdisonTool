#ifndef BOGPIO_H
#define BOGPIO_H

#define HIGH 0x01
#define LOW  0x00

float readAPin(int iopin);
int readPin(int iopin,bool sc);
void writePin(int iopin,int valueiopin);
void digitalWrite(int iopin,int valueiopin);
void printAllPins();
void bsleep(int s);
void printPins();
void CNCtest();
#endif

