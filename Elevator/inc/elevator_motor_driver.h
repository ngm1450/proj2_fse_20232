#ifndef ELEVATOR_MOTOR_DRIVER_H_
#define ELEVATOR_MOTOR_DRIVER_H_

#include <request.h>
#include <wiringPi.h>

extern volatile int panic;

void enfileira(int floor);
int consomeProximoAndar();
void moveElevator(int floor);
void *setupElevator(void *arg);
void *monitorButtons(void *arg);
void printData();

#endif 