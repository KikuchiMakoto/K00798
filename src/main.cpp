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
#define PIN_SIN_1  2
#define PIN_SIN_2  3
#define PIN_SIN_3  4
#define PIN_CLOCK  5
#define PIN_LATCH  6
#define PIN_STROBE 7
#endif

static uint16_t matrix_buffer[8*16];
static uint8_t base64_buffer[8*16*2];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(PIN_SIN_1, OUTPUT);
  pinMode(PIN_SIN_2, OUTPUT);
  pinMode(PIN_SIN_3, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_STROBE, OUTPUT);
  digitalWrite(PIN_STROBE, HIGH);
}

void loop() {
  for (int x=0; x<16; x++) {
    //noInterrupts();
    digitalWrite(PIN_STROBE, HIGH);
    digitalWrite(PIN_LATCH, HIGH);//LatchをHIGH
    for (int panel=0; panel<4; panel++) {
      uint16_t sin2 = matrix_buffer[16*2*(3-panel)+x+0];
      uint16_t sin3 = matrix_buffer[16*2*(3-panel)+x+1];
      for (int y=0; y<16; y++) {
        //uint8_t sin = panel.getSin(x, y);
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
    //interrupts();
    while (Serial.available())
    {
      static uint8_t base64_buffer_index = 0;
      unsigned char c = Serial.read();
      
      
      if (c == '\n') {
        int length = decode_base64(base64_buffer, (unsigned char *)&matrix_buffer[0]);
        Serial.print("length: ");
        Serial.println(length);
        base64_buffer_index = 0;
        memset(base64_buffer, 0, sizeof(base64_buffer));
      } else {
        base64_buffer[base64_buffer_index++] = c;
        if (base64_buffer_index == 8*16*2) {
          base64_buffer_index = 8*16*2-1;
        }
      }
    }
    
    delayMicroseconds(100);
  }
}