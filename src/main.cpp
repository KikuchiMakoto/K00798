#include <Arduino.h>
#include <Base64.hpp>

#ifdef __AVR__
#define PIN_SIN_1  2
#define PIN_SIN_2  3
#define PIN_SIN_3  4
#define PIN_CLOCK  5
#define PIN_LATCH  6
#define PIN_STROBE 7
#endif

#ifdef __RP2040__
#define PIN_SIN_1  0
#define PIN_SIN_2  1
#define PIN_SIN_3  2
#define PIN_CLOCK  3
#define PIN_LATCH  4
#define PIN_STROBE 5
#endif

static uint16_t matrix_buffer[4*2][16];

#define SIZE_RECV_BUFF 1024
static uint8_t recv_buff[SIZE_RECV_BUFF];
static uint16_t p_recv_buff = 0;

#define BASE64_BUFF_SIZE 768
static uint8_t base64_buff[BASE64_BUFF_SIZE];

void setup() {
  Serial.begin(921600);
}

void setup1(){
  pinMode(PIN_SIN_1, OUTPUT);
  pinMode(PIN_SIN_2, OUTPUT);
  pinMode(PIN_SIN_3, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_STROBE, OUTPUT);
  digitalWrite(PIN_STROBE, HIGH);

  for (int x=0; x<8; x++) {
    for (int y=0; y<16; y++) {
      matrix_buffer[x][y] = x==(y%8)? 0x5050: 0x0000;
    }
  }
}

void loop() {
  for (int x=0; x<16; x++) {
    digitalWrite(PIN_STROBE, HIGH);
    digitalWrite(PIN_LATCH, HIGH);//LatchをHIGH
    for (int panel=0; panel<4; panel++) {
      uint16_t sin2 = matrix_buffer[2*(3-panel)+0][x];
      uint16_t sin3 = matrix_buffer[2*(3-panel)+1][x];
      for (int y=0; y<16; y++) {
        digitalWrite(PIN_CLOCK, LOW); //clock
        digitalWrite(PIN_SIN_1, x==y? HIGH: LOW);
        digitalWrite(PIN_SIN_2, sin2&1? HIGH: LOW);
        digitalWrite(PIN_SIN_3, sin3&1? HIGH: LOW);
        digitalWrite(PIN_CLOCK, HIGH);//clock
        sin2 = sin2 >> 1;
        sin3 = sin3 >> 1;
      }
    }
    digitalWrite(PIN_LATCH, LOW); //Latch low 書き込み
    delayMicroseconds(1);
    digitalWrite(PIN_STROBE, LOW); //STLOBE HIGH
    delayMicroseconds(1);

    delayMicroseconds(200);
  }
}

void loop1() {
  while (Serial.available()){
    uint8_t temp = Serial.read();
    recv_buff[p_recv_buff] = temp;
    if (temp == '\r') {
      // Ignore 
    }
    else if (temp == '\n') {
      // Convert to base64 to binary
      int b64_len = decode_base64_length(recv_buff);
      if (b64_len == 256) {
        decode_base64(recv_buff, base64_buff);
        memcpy(&matrix_buffer[0][0], &base64_buff[0], 256);
      }
      p_recv_buff = 0;
      memset(&recv_buff[0], 0x00, SIZE_RECV_BUFF);
      memset(&base64_buff[0], 0x00, BASE64_BUFF_SIZE);
    } else {
      p_recv_buff++;
      if (p_recv_buff >= SIZE_RECV_BUFF) p_recv_buff = SIZE_RECV_BUFF - 1;
    }
  } 
}