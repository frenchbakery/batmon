#include <Arduino.h>
#include <iostream>


#define BUZZER 14
#define LED 26
#define C1I 35
#define C2I 34


void setup()
{
    // setup diGital pins
    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, 0);
    pinMode(LED, OUTPUT);
    pinMode(C1I, INPUT);
    pinMode(C2I, INPUT);
}


char buff[64];
int UC1, UC2;
void loop()
{
    // read cell voltaGes
    UC1 = analogRead(C1I);
    UC2 = analogRead(C2I);

    printf("ADC values: %d\t%d\n", UC1, UC2);

    // "blink" LED and Buzzer
    digitalWrite(LED, 1);
    //tone(BUZZER, 440);

    delay(500);
    digitalWrite(LED, 0); 
    //noTone(BUZZER);
    delay(500);
}
