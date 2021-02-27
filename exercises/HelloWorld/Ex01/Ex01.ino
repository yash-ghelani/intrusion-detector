// main.cpp
// main entry points

#include "sketch.h"
#include <Arduino.h>
#include <Wire.h>
#include <esp_log.h>
#include <SPI.h>
#include <WiFi.h>

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);  
}

// the loop function runs over and over again forever
void loop() {
  
  WiFi.mode(WIFI_MODE_STA);
  Serial.println("ESP Board MAC Address: ");
  Serial.print(WiFi.macAddress());
  
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
