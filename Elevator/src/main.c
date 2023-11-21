#include <stdio.h>
#include <wiringPi.h>
#include <elevator_motor_driver.h>
#include <request.h>
#include <communication.h>
#include <bmp280.h>
#include <calibrator.h>
#include <signal.h>

void handleCtrlC(int signum) {
    if (signum == SIGINT) {
        temp_data = 0.0;
        pwm_data = 0;
        delay(50);
        stop = 1;
        panic = 1;
        closeUart();
        printf("Recebido sinal Ctrl+C. Encerrando...\n");
        delay(2000);
    }
    exit(0);
}

int main(){
    signal(SIGINT, handleCtrlC);

    if (wiringPiSetup() == -1){
        return 1 ;
    }

    printf("Deseja calibrar os sensores?\n [0] - não [1] - sim :");

    int option;
    scanf("%d", &option);

    if(option == 1){
        initCalibration();
    }

    printf("Iniciando\n");
    setupElevator(NULL);
    return 0;
}
