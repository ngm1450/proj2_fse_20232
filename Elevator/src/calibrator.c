#include <wiringPi.h>
#include <stdio.h>
#include <pthread.h>
#include <softPwm.h>
#include <communication.h>

#define TERREO  1
#define ANDAR_1  4
#define ANDAR_2  5
#define ANDAR_3  6
#define PIN_SUBIR 28
#define PIN_DESCER 29
#define PIN_PWM 26
int encoder =0;
int EncoderFloorValue[4] = {0};
int breakThread = 0;

void *readEncoder(void *arg) {
    unsigned char *message = makeMessage(1, 0,0);
    while (breakThread == 0) {
        char rx_data[256];
        uartSetup(message, 9, rx_data);
        int *data = processReceivedData(rx_data);
        if (data != NULL) {
            encoder = data[0];
            free(data);
        }
       delay(200);
    }
}

void setupSensores() {

    pinMode(TERREO, INPUT);
    pinMode(ANDAR_1, INPUT);
    pinMode(ANDAR_2, INPUT);
    pinMode(ANDAR_3, INPUT);

    pinMode(PIN_SUBIR, OUTPUT);
    pinMode(PIN_DESCER, OUTPUT);
    softPwmCreate(PIN_PWM, 0, 100); 
}

void verificaSensores() {
    if (digitalRead(TERREO) == HIGH) {
        EncoderFloorValue[0] = (EncoderFloorValue[0] == 0) ? encoder : EncoderFloorValue[0];
    }
    if (digitalRead(ANDAR_1) == HIGH) {
        EncoderFloorValue[1] = (EncoderFloorValue[1] == 0) ? encoder : EncoderFloorValue[1];
    }
    if (digitalRead(ANDAR_2) == HIGH) {
        EncoderFloorValue[2] = (EncoderFloorValue[2] == 0) ? encoder : EncoderFloorValue[2];
    }
    if (digitalRead(ANDAR_3) == HIGH) {
        EncoderFloorValue[3] = (EncoderFloorValue[3] == 0) ? encoder : EncoderFloorValue[3];
    }
}

void initCalibration(){

    initUart();
    setupSensores();

    pthread_t encoderThread;
    pthread_create(&encoderThread, NULL, readEncoder, NULL);

    digitalWrite(PIN_SUBIR, HIGH);
    digitalWrite(PIN_DESCER, LOW);
    softPwmWrite(PIN_PWM, 3);

    printf("\nSubindo elevador e calibrando sensores de andar\n");
    while (encoder < 25500  && EncoderFloorValue[3] == 0) {
        verificaSensores();
        delay(20);
    }

    while(encoder > 0 ){
        digitalWrite(PIN_SUBIR, LOW);
        digitalWrite(PIN_DESCER, HIGH);
        softPwmWrite(PIN_PWM, 100);
    }

    digitalWrite(PIN_SUBIR, HIGH);
    digitalWrite(PIN_DESCER, HIGH);

    for(int i =0 ;i<4 ;i++){
        printf("Sensor %d valor: %d\n", i, EncoderFloorValue[i]);
    }

    breakThread = 1;
    pthread_join(encoderThread, NULL);
}