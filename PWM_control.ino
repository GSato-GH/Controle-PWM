byte interruptPin = 3;
#include "TimerOne.h"

/* Definicao de um buffer */
#define MAX_BUFFER_SIZE 31
typedef struct {
  char data[MAX_BUFFER_SIZE];
  unsigned int tam_buffer;
} serial_buffer;

serial_buffer Buffer;

/* Limpa buffer */
void buffer_clean() {
  Buffer.tam_buffer = 0;
}

/* Adiciona caractere ao buffer */
int buffer_add(char c_in) {
  if (Buffer.tam_buffer < MAX_BUFFER_SIZE) {
    Buffer.data[Buffer.tam_buffer++] = c_in;
    return 1;
  }
  return 0;
}

/* Funcao que compara duas strings */
int str_cmp(char *s1, char *s2, int len) {
  int i;
  for (i=0; i<len; i++) {
    if (s1[i] != s2[i]) return 0;
    if (s1[i] == '\0') return 1;
  }
  return 1;
}

/* Configuracao dos pinos para operacao do sensor */
int flag_check_command = 0;
void setup() {
 Serial.begin(9600);
 pinMode(interruptPin, INPUT_PULLUP);
 attachInterrupt(digitalPinToInterrupt(3), infrared_read, RISING);
 Timer1.initialize(250000*4);
 Timer1.attachInterrupt(temporizador);
}

/* Ao receber evento da UART */
void serialEvent() {
  char c;
  while (Serial.available()) {
    c = Serial.read();
    if (c=='\n') {
      buffer_add('\0'); /* Se recebeu um fim de linha, coloca um terminador de string no buffer */
      flag_check_command = 1;
    } else {
     buffer_add(c);
    }
  }
}

unsigned long flag = 0;
volatile unsigned long count = 0;
int rpm_read;
int rpm_request = 0;
float pwm_atual = 0;
int pwm_atual_int = 0;

/* Recebimento de valor de RPM desejado pelo usuario */
void loop() {
  if (flag_check_command == 1) {
      sscanf(Buffer.data, "%d", &rpm_request);
  }
}

/* Contagem do numero de interrupcoes do sensor infravermelho */
void infrared_read(){
  count++;
}

/* PWM */
void temporizador(){
  char out_buffer[127];
  rpm_read = count*30;
  count = 0;
  long e = rpm_request - rpm_read;
  float kp = 0.0045; /* Ganho proporcional do feedback negativo */

  pwm_atual = pwm_atual + e*kp; /* Feedback negativo */
  pwm_atual_int = (int)pwm_atual;
  long e_percent = abs(e*100/rpm_request); /* Erro percentual */
  if(e == 0){
    e_percent = 0;
  }
  analogWrite(2,pwm_atual); /* Sinal PWM enviado ao motor */
  sprintf(out_buffer, "RPM Requested: %-8d RPM read: %-8d PWM atual: %-8d Erro: %ld%% \n", rpm_request, rpm_read, pwm_atual_int, e_percent);
  Serial.print(out_buffer);
  buffer_clean();

}

