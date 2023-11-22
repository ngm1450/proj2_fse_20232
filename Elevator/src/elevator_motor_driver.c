#include "elevator_motor_driver.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <pid.h>
#include <lcd_driver.h> 
#include <string.h>
#include <stdlib.h>
#include <softPwm.h>
#include <calibrator.h>

#define NUM_FLOORS 11

pthread_mutex_t btn_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
int queue_floors[NUM_FLOORS] = {0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}; 
int size_queue = 1;

volatile int panic = 0;

#define PIN_SUBIR 28
#define PIN_DESCER 29
#define PIN_PWM 26

void enqueue(int floor) {
    pthread_mutex_lock(&queue_mutex);
    int eq = 0;
    for(int i = 0; i < NUM_FLOORS; i++) {
        if (queue_floors[i] == floor) {
            eq = 1;
        }
    }
    if(eq != 1) {
        queue_floors[size_queue++] = floor;
    }   
    pthread_mutex_unlock(&queue_mutex);
}

void updateLocation(int numBtn) {
    const char* locations[] = {"Terreo", "1 Andar", "1 Andar", "2 Andar", "2 Andar", "3 Andar", "Emergencia", "Terreo", "1 Andar", "2 Andar", "3 Andar"};

    if (numBtn >= 0 && numBtn <= 10) {
        strcpy(location, locations[numBtn]);
    } 
}

int consumeNext() {
    pthread_mutex_lock(&queue_mutex);

    if (size_queue > 0) {
        int next_floor = queue_floors[0];
        for (int i = 0; i < size_queue - 1; ++i) {
            queue_floors[i] = queue_floors[i + 1];
        }
        size_queue--;

        pthread_mutex_unlock(&queue_mutex);
        return next_floor;
    } else {
        pthread_mutex_unlock(&queue_mutex);
        return -1;
    }
}

int peekFIla(){
    pthread_mutex_lock(&queue_mutex);
    if (size_queue > 0){
        int next_floor = queue_floors[0];

        pthread_mutex_unlock(&queue_mutex);
        return next_floor;
    } else {
        pthread_mutex_unlock(&queue_mutex);
        return -1;
    }
}

void moveElevator(int floor) {

    pid_atualiza_referencia((float) floor);

    int error;
    if (floor < 2000) {
        error = 76;
    } else if (floor < 7000) {
        error = 75;
    } else if (floor < 13000) {
        error = 80;
    } else {
        error = 90;
    }

    while (1) {
        if ((encoder_data >= floor - error && encoder_data <= floor + error + 20) || panic !=0) {
            strcpy(state, "PARADO");
            digitalWrite(PIN_SUBIR, HIGH);
            digitalWrite(PIN_DESCER, HIGH);
            pwm_data = 0;
            break;
        }
        if(encoder_data < 0 || encoder_data > 25500){
            continue;
        }
        double control_signal = pid_controle((double) encoder_data);

        if (control_signal < 0){
            pwm_data = control_signal * (-1);
        }else{
            pwm_data = control_signal;
        }
        
        if (control_signal > 0) {
            digitalWrite(PIN_SUBIR, HIGH);
            digitalWrite(PIN_DESCER, LOW);
            if (strcmp(state, "SUBINDO") != 0) {
                strcpy(state, "SUBINDO");
            }
        } else if (control_signal < 0) {
            digitalWrite(PIN_SUBIR, LOW);
            digitalWrite(PIN_DESCER, HIGH);
            control_signal *= (-1); 
            if (strcmp(state, "DESCENDO") != 0) {
                strcpy(state, "DESCENDO");
            }
        } else {
            digitalWrite(PIN_SUBIR, HIGH);
            digitalWrite(PIN_DESCER, HIGH);
            if (strcmp(state, "PARADO") != 0) {
                strcpy(state, "PARADO");
            }
        }

        int pwm_value = (int) control_signal;
        softPwmWrite(PIN_PWM, pwm_value);
    }
}

void *monitorButtons(void *arg) {
    while (1) {
        pthread_mutex_lock(&btn_mutex);
        for (int floor = 0; floor < NUM_FLOORS; ++floor) {
            if(btn_data[floor]>1) break;
            if (btn_data[floor] == 1) {
                enqueue(floor);
            }
            if(btn_data[6] == 1){
                printf("Emergencia");
                panic = 1;
                delay(5000);
                exit(0);
            }
        }

        pthread_mutex_unlock(&btn_mutex);
        delay(100);  
    }
    return NULL;
}

void *setupElevator(void *arg) {
    setupRequests();

    pid_configura_constantes(0.6, 0.03, 15);

    pinMode(PIN_SUBIR, OUTPUT);
    pinMode(PIN_DESCER, OUTPUT);

    softPwmCreate(PIN_PWM, 0, 300); 

    pthread_t btn_thread;
    pthread_create(&btn_thread, NULL, monitorButtons, NULL);

    while (1) {
        int next_floor = peekFIla();
        int floor=0;
        if (next_floor != -1) {
            switch (next_floor) {
                case 0:
                case 7:       
                    if (EncoderFloorValue[0] == 0){ 
                        floor = 1971;
                    }else{
                        floor = EncoderFloorValue[0];
                    }
                    break;
                case 1:
                case 2:
                case 8:
                    if (EncoderFloorValue[1] == 0){ 
                        floor = 4915;
                    }else{
                        floor = EncoderFloorValue[1];
                    }
                    break;
                case 3:
                case 4:
                case 9:
                    if (EncoderFloorValue[2] == 0){ 
                        floor = 12095;
                    }else{
                        floor = EncoderFloorValue[2];
                    }
                    break;
                case 5:
                case 10:
                    if (EncoderFloorValue[3] == 0){ 
                        floor = 18896;
                    }else{
                        floor = EncoderFloorValue[3];
                    }
                    break;
                default:
                    digitalWrite(PIN_SUBIR, HIGH);
                    digitalWrite(PIN_DESCER, HIGH);
                    exit(0);
            }
            moveElevator(floor);
            while(turnOffBtn(next_floor) != 1){}
            updateLocation(consumeNext());
            sleep(9);  
        }
        delay(50);
    }

    pthread_join(btn_thread, NULL);
    return NULL;
}
void printData() {
    setupRequests();
    while (1) {
        sleep(1);
        printf("Button Data: ");
        for (int i = 0; i < 11; ++i) {
            printf("%d ", btn_data[i]);
        }
        printf("\n");
        printf("Encoder Data: %d\n", encoder_data);
    }
}