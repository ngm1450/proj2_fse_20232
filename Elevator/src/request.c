
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <communication.h>
#include <lcd_driver.h>
#include <wiringPi.h>
#include <bmp280.h>

volatile int btn_data[11];
volatile int encoder_data;
volatile int pwm_data = 0;
volatile float temp_data = 0.0;
volatile char state[] = "init";
volatile int stop = 0;
volatile char location[] = "terreo";

pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;


void *pwmThread(void *arg) {
    while (stop!=1) {
        pthread_mutex_lock(&data_mutex);
        unsigned char *message = makeMessage(3, pwm_data, 0);   
        uartSetup(message, 13, NULL);
        pthread_mutex_unlock(&data_mutex);
        usleep(300000);
    }
    printf("Finalizando escrita de PWN thread\n");
    return NULL;
}

void *btnThread(void *arg) {
    unsigned char *message = makeMessage(2, 0,0);
    while (stop!=1) {
        pthread_mutex_lock(&data_mutex);

        char rx_data[256];

        uartSetup(message, 10, rx_data);

        int *temp_btn_data = processReceivedData(rx_data);

        if (temp_btn_data != NULL) {
            memcpy(btn_data, temp_btn_data, sizeof(int) * 11);
            free(temp_btn_data);
        }

        pthread_mutex_unlock(&data_mutex);

        usleep(100000);
    }
    printf("Finalizando leitura de botões thread\n");

    return NULL;
}

void *encoderThread(void *arg) {
    unsigned char *message = makeMessage(1, 0,0);

    while (stop!=1) {
        pthread_mutex_lock(&data_mutex);

        char rx_data[256];

        uartSetup(message, 9, rx_data);

        int *data = processReceivedData(rx_data);

        if (data != NULL) {
            encoder_data = data[0];
            free(data);
        }

        pthread_mutex_unlock(&data_mutex);
        usleep(50000);
    }
    printf("Finalizando leitura de encoder thread\n");

    return NULL;
}


void *tempThread(void *arg) {
    while (stop!=1) {
        pthread_mutex_lock(&data_mutex);

        temp_data = readTemperature();
        unsigned char *message = makeMessage(4, 0, temp_data);   
        uartSetup(message, 13, NULL);
        free(message);

        pthread_mutex_unlock(&data_mutex);
        usleep(500000);
    }
    printf("Finalizando escrita de temperatura thread\n");
    return NULL;
}

void *lcdThread(void *arg) {
    while (stop!=1) {
        lcd_write(state, temp_data, location);
        delay(2000);
    }
    printf("Finalizando escrita dados lcd thread\n");
    return NULL;
}

int turnOffBtn(int btn){
    int i = 0;
    pthread_mutex_lock(&data_mutex);

    unsigned char *req = makeMessage(btn + 10, 0,0);
    uartSetup(req, 11, NULL);
    
    free(req);
    i = 1;
    pthread_mutex_unlock(&data_mutex);
    return i;
}

void setupRequests(){
    initUart();
    lcd_init();
    pthread_t tempThreadId, btnThreadId, encoderThreadId, pwmThreadId, lcdThreadId;

    pthread_create(&tempThreadId, NULL, tempThread, NULL);
    pthread_create(&btnThreadId, NULL, btnThread, NULL);
    pthread_create(&encoderThreadId, NULL, encoderThread, NULL);
    pthread_create(&pwmThreadId, NULL, pwmThread, NULL);
    pthread_create(&lcdThreadId, NULL, lcdThread, NULL);
}