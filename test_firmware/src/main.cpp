/**
 * @file main.cpp
 * @author Nilusink
 * @brief 
 * @version 0.1
 * @date 2023-07-03
 * 
 * @copyright Copyright FrenchBakery (c) 2023
 * 
 */

#include <Arduino.h>
#include <iostream>

#include "pins.h"
#include "player.hpp"


void setup()
{
    // setup diGital pins
    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, 0);
    pinMode(LED, OUTPUT);
    pinMode(WRONG_C1I, INPUT);
    pinMode(WRONG_C2I, INPUT);
    pinMode(C1I, INPUT);
    pinMode(C2I, INPUT);

    Player::start();
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
