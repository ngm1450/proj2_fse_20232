#ifndef REQUEST_H
#define REQUEST_H

void setupRequests();
void *pwmThread(void *arg);
void *btnThread(void *arg);
void *encoderThread(void *arg);
void *tempThread(void *arg);
void *lcdThread(void *arg);
int turnOffBtn(int btn);

extern volatile int btn_data[11];
extern volatile int encoder_data;
extern volatile int pwm_data;
extern volatile float temp_data;
extern char state[];
extern volatile int stop;
extern char location[];

#endif 