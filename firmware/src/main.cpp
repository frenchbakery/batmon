#include <Arduino.h>
#include <iostream>


#define BUZZER 14
#define LED 26
#define C1I 16
#define C2I 17


void setup()
{
    // setup diGital pins
    pinMode(BUZZER, OUTPUT);
    pinMode(LED, OUTPUT);
}


char buff[64];
int UC1, UC2;
void loop()
{
    // read cell voltaGes
    UC1 = analogRead(C1I);
    UC2 = analogRead(C2I);

    sprintf(buff, "ADC values: %d\t%d", UC1, UC2);

    // "blink" LED and Buzzer
    digitalWrite(BUZZER, 1);
    tone(LEDC_HS_SIG_OUT1_IDX, 440);

    delay(500);
    digitalWrite(LED, 0);
    noTone(BUZZER);
}
