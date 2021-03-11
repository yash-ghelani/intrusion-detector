#include "sketch.h"
#include <Arduino.h>
#include <Wire.h>
#include <esp_log.h>
#include <WiFi.h>
#include <WebServer.h>

// Webserver
WebServer webServer(80);

// LEDs
const int ledPinR = 32;
const int ledPinY = 15;
const int ledPinG = 33;
bool go = true;

const char *styleArr[] = { // boilerplate: constants & pattern parts of template         // 8
  "html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n",
  "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n",
  ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n",
  ".button-on {background-color: #3498db;}\n",
  ".button-on:active {background-color: #2980b9;}\n",
  ".button-off {background-color: #34495e;}\n",
  ".button-off:active {background-color: #2c3e50;}\n",
  "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n",
  
};

const char *bodyArr[] = { // boilerplate: constants & pattern parts of template
  
  "<h1>ESP32 Web Server</h1>\n",
  "<h3>Using Access Point(AP) Mode</h3>\n",
  "<p>LED1 Status: ON</p>\n",
  "<a class=\"button button-off\" href=\"/change\">Change</a>\n",

};

void setup() {
  // put your setup code here, to run once:
  // setting baud rate 
  Serial.begin(115200);

  //set the LED as the output 
  pinMode(ledPinR, OUTPUT);
  pinMode(ledPinY, OUTPUT);
  pinMode(ledPinG, OUTPUT);

  //Traffic light starts off green
  digitalWrite(ledPinG, HIGH);
  
  dp("starting webserver")
  
  if(! WiFi.mode(WIFI_AP_STA)){
    dp("failed to set Wifi mode");
  }
  if(! WiFi.softAP("ssid", "password")){
    dp("failed to start soft AP");
  }

  startWebServer();
}

void startWebServer(){
  webServer.on("/", handleRoot);
  webServer.on("/change", handleChange);
  webServer.begin();
  dp("Web server started");

}

void handleRoot(){
  dp("getting root page")

  
  String toSend = getPageTop("COM3505 - IoT");
  toSend += getPageStyle();
  toSend += getPageBody();
  toSend += getPageFooter();

  webServer.send(200, "text/html", toSend);
}

void handleChange(){
  dp("changing lights")
  lightChange();
  String toSend = getPageTop("COM3505 - IoT");
  toSend += getPageStyle();
  toSend += getPageBody();
  toSend += getPageFooter();

  webServer.send(200, "text/html", toSend);
}


String getPageTop(String title) {
  return
    "<html><head><title>"+title+" </title>\n"
    "<meta charset=\"utf-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
    "<style>\n"
  ;
};

String getPageStyle(){
  String style = "";
  for(int i = 0; i < 8; i++) {
      style.concat(styleArr[i]);
  }
  style.concat("</style></head><body>\n");
  return style;
}

String getPageBody() {

  String body = "";
  for(int i = 0; i < 4; i++) {
      body.concat(bodyArr[i]);
  }
  return body;
}

String getPageFooter() {
  return "\n<p><a href='/'>Home</a>&nbsp;&nbsp;&nbsp;</p></body></html>\n";
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
  // put your main code here, to run repeatedly:
  
  // deal with any pending web requests
  webServer.handleClient(); 
}
