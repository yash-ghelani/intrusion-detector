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
const int ledPin = 32; 

void setup() {
  //set the LED as the output 
  pinMode(ledPin, OUTPUT);
  pinMode(pushButton, INPUT_PULLUP); 
  Serial.begin(115200);  
}

void loop() {
  if (digitalRead(pushButton) == LOW) {
    Serial.println("Button was pressed");
    digitalWrite(ledPin, HIGH);
  }else{
    digitalWrite(ledPin, LOW);    
  }
  delay(100);
}
