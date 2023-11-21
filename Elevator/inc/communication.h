
#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include <stdio.h>
#include <unistd.h>  //Used for UART
#include <fcntl.h>   //Used for UART
#include <termios.h> //Used for UART
#include <crc16.h>
#include <string.h>
#include <stdlib.h>

unsigned char *makeMessage(int type, int valuePWW, float valueTemp);
int uartSetup(unsigned char *message, int size, unsigned char* data);
void initUart();
int * processReceivedData(unsigned char *rx_data);
void closeUart();
#endif 