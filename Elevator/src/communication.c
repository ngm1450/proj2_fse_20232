#include <communication.h>

static int uart0_filestream = -1;

unsigned char *makeMessage(int type, int valuePWW, float valueTemp) {
   
    unsigned char *tx_buffer = NULL;
    short crc;

    switch (type) {
        case 1:
            tx_buffer = (unsigned char *)malloc(9 * sizeof(unsigned char));
            tx_buffer[0] = 0x01;
            tx_buffer[1] = 0x23;
            tx_buffer[2] = 0xc1;
            tx_buffer[3] = 1;
            tx_buffer[4] = 2;
            tx_buffer[5] = 7;
            tx_buffer[6] = 7;
            crc = calcula_CRC(tx_buffer, 7);

            for (int i = 0; i < sizeof(crc); ++i) {
                tx_buffer[7 + i] = (unsigned char)((crc >> (8 * i)) & 0xFF);
            }
            break;
        case 2: 
            tx_buffer = (unsigned char *)malloc(10 * sizeof(unsigned char));
            tx_buffer[0] = 0x01;
            tx_buffer[1] = 0x03;
            tx_buffer[2] = 0x00;
            tx_buffer[3] = 0xB;
            tx_buffer[4] = 1;
            tx_buffer[5] = 2;
            tx_buffer[6] = 7;
            tx_buffer[7] = 7;
            crc = calcula_CRC(tx_buffer, 8);
            for (int i = 0; i < sizeof(crc); ++i) {
                tx_buffer[8 + i] = (unsigned char)((crc >> (8 * i)) & 0xFF);
            }
            break;
        case 3:
            tx_buffer = (unsigned char *)malloc(13 * sizeof(unsigned char));
            tx_buffer[0] = 0x01;
            tx_buffer[1] = 0x16;
            tx_buffer[2] = 0xC2;
            memcpy(&tx_buffer[3], &valuePWW, 4);
            tx_buffer[7] = 1;
            tx_buffer[8] = 2;
            tx_buffer[9] = 7;
            tx_buffer[10] = 7;
            crc = calcula_CRC(tx_buffer, 11);
            for (int i = 0; i < sizeof(crc); ++i) {
                tx_buffer[11 + i] = (unsigned char)((crc >> (8 * i)) & 0xFF);
            }
            break;
        case 4:
            tx_buffer = (unsigned char *)malloc(13 * sizeof(unsigned char));
            tx_buffer[0] = 0x01;
            tx_buffer[1] = 0x16;
            tx_buffer[2] = 0xD1;
            memcpy(&tx_buffer[3], &valueTemp, 4);
            tx_buffer[7] = 1;
            tx_buffer[8] = 2;
            tx_buffer[9] = 7;
            tx_buffer[10] = 7;
            crc = calcula_CRC(tx_buffer, 11);
            for (int i = 0; i < sizeof(crc); ++i) {
                tx_buffer[11 + i] = (unsigned char)((crc >> (8 * i)) & 0xFF);
            }
            break;
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
            tx_buffer = (unsigned char *)malloc(11 * sizeof(unsigned char));
            tx_buffer[0] = 0x01;
            tx_buffer[1] = 0x06;
            tx_buffer[2] = type - 10;
            tx_buffer[3] = 0x01;
            tx_buffer[4] = 0x00;
            tx_buffer[5] = 1;
            tx_buffer[6] = 2;
            tx_buffer[7] = 7;
            tx_buffer[8] = 7;
            crc = calcula_CRC(tx_buffer, 9);
            for (int i = 0; i < sizeof(crc); ++i) {
                tx_buffer[9 + i] = (unsigned char)((crc >> (8 * i)) & 0xFF);
            }
            break;
    }

    return tx_buffer;
}

void initUart(){
    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart0_filestream == -1)
    {
        printf("Erro - Não foi possível iniciar a UART.\n");
    }
    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);
}

void closeUart() {
    if (close(uart0_filestream) == -1) {
        perror("Erro ao fechar a UART");
    } else {
        printf("UART fechada com sucesso.\n");
    }
}

void printResponseData(unsigned char* response, int size) {
    printf("Response Data:");
    for (int i = 0; i < size; ++i) {
        printf("%02X ", response[i]);
    }

}

int uartSetup(unsigned char *message, int size, unsigned char* data) {

    if (uart0_filestream != -1)
    {
        if (write(uart0_filestream, message, size) <= 0) {
            printf("Erro de escrita na UART.\n");
            close(uart0_filestream);
            return -1;
        }
    }

    tcflush(uart0_filestream, TCIFLUSH);
    usleep(30000);

    if (uart0_filestream != -1){
        unsigned char rx_buffer[256];
        int rx_length = read(uart0_filestream, (void *)rx_buffer, 255);
        if (rx_length < 0){
            printf("Erro na leitura.\n");
        }else if (validateResponse(rx_buffer, rx_length) == 0){
        }else{
            rx_buffer[rx_length] = '\0';
            if(data != NULL){
                memcpy(data, rx_buffer, rx_length);
            }
        }
    }
    return 0;
}

int *processReceivedData(unsigned char *rx_data) {
    int *result = NULL;
    if (rx_data[2] == 0xC1) {
        unsigned int received_value = rx_data[3] | (rx_data[4] << 8) | (rx_data[5] << 16) | (rx_data[6] << 24);
        result = (int *)malloc(sizeof(int));
        if (result != NULL) {
            result[0] = (int)received_value;
        } 
    } else if (rx_data[1] == 0x03) {
        result = (int *)malloc(11 * sizeof(int));
        if (result != NULL) {
            for (int i = 0; i < 11; ++i) {
                result[i] = (int)rx_data[2 + i];
            }
        }
    }

    return result;
}

int validateResponse(unsigned char* response, int size){

    short receivedCrc = (response[size - 1] << 8) | response[size - 2];
    short crc = calcula_CRC(response, size - 2);
    
    if (receivedCrc == crc) {
        return 1;
    } else {
        return 0;
    }
}