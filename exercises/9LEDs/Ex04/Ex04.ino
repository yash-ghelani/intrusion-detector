// main.cpp
// main entry points

#include "sketch.h"
#include <Arduino.h>
#include <Wire.h>
#include <esp_log.h>
#include <SPI.h>
#include <WiFi.h>

//set the pin numbers
const int pushButton = 14; 
const int ledPinR = 32;
const int ledPinY = 15;
const int ledPinG = 33;
int iteration = 0;

void setup() {
  //set the LED as the output 
  pinMode(ledPinR, OUTPUT);
  pinMode(ledPinY, OUTPUT);
  pinMode(ledPinG, OUTPUT);
  pinMode(pushButton, INPUT_PULLUP);

  // setting baud rate 
  Serial.begin(115200);
  
  //Traffic light starts off green
  digitalWrite(ledPinG, HIGH);
}

void lightChange(){

  Serial.println("\nLight change");
  
  if (digitalRead(ledPinG) == HIGH){ // if traffic light was green
      delay(400);
      digitalWrite(ledPinG, LOW);
      digitalWrite(ledPinY, HIGH);
      delay(400);
      digitalWrite(ledPinY, LOW);
      digitalWrite(ledPinR, HIGH);

    } else {
      delay(400);
      digitalWrite(ledPinR, LOW);
      digitalWrite(ledPinY, HIGH);
      delay(400);
      digitalWrite(ledPinY, LOW);
      digitalWrite(ledPinG, HIGH);
    }
}

void loop() {
  if (iteration % 10000000 == 0){
    lightChange();
  }
  if (digitalRead(pushButton) == LOW) {
    
    PRINT("\nButton was pressed at: ", iteration);
    lightChange();
       
  }
  iteration ++;
}
