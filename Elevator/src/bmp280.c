#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp280.h"

#define PRESSURE_FILE "/sys/bus/iio/devices/iio:device0/in_pressure_input"
#define TEMPERATURE_FILE "/sys/bus/iio/devices/iio:device0/in_temp_input"

float readPressure() {
    FILE *file = fopen(PRESSURE_FILE, "r");
    if (file == NULL) {
        perror("Error opening pressure file");
        exit(EXIT_FAILURE);
    }

    float pressure;
    fscanf(file, "%f", &pressure);
    fclose(file);

    // Multiplica por 10 para obter o valor correto
    return pressure * 10.0;
}

float readTemperature() {
    FILE *file = fopen(TEMPERATURE_FILE, "r");
    if (file == NULL) {
        perror("Error opening temperature file");
        exit(EXIT_FAILURE);
    }

    int temp;
    fscanf(file, "%d", &temp);
    fclose(file);

    // Calcula o valor da temperatura
    float temperature = (((float)temp / 1000.0) * 100.0 + 0.5) / 100.0;
    return temperature;
}