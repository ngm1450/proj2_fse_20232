#include "pid.h"
#include <stdio.h>

static double saida_medida, sinal_de_controle;
static double referencia = 0.0;
static double Kp = 0.0;
static double Ki = 0.0;
static double Kd = 0.0;
static int T = 1;
static unsigned long last_time;
static double erro_total, erro_anterior = 0.0;
static int sinal_de_controle_MAX = 100.0;
static int sinal_de_controle_MIN = -100.0;

void pid_configura_constantes(double Kp_, double Ki_, double Kd_){
    Kp = Kp_;
    Ki = Ki_;
    Kd = Kd_;
}

void pid_atualiza_referencia(float referencia_){
    referencia = (double) referencia_;
}

double pid_controle(double saida_medida){

    double erro = referencia - saida_medida;

    erro_total += erro;

    if (erro_total >= sinal_de_controle_MAX) 
    {
        erro_total = sinal_de_controle_MAX;
    }
    else if (erro_total <= sinal_de_controle_MIN) 
    {
        erro_total = sinal_de_controle_MIN;
    }

    double delta_error = erro - erro_anterior;
    
    sinal_de_controle = Kp*erro + (Ki*T)*erro_total + (Kd/T)*delta_error;

    if (sinal_de_controle >= sinal_de_controle_MAX) 
    {
        sinal_de_controle = sinal_de_controle_MAX;
    }
    else if (sinal_de_controle <= sinal_de_controle_MIN) 
    {
        sinal_de_controle = sinal_de_controle_MIN;
    }

    erro_anterior = erro;

    return sinal_de_controle;
}