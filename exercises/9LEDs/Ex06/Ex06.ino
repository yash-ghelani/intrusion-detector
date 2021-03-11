#include "sketch.h"
#include <Arduino.h>
#include <Wire.h>
#include <esp_log.h>
#include <WiFi.h>
#include <WebServer.h>

WebServer webServer(80);

void setup() {
  // put your setup code here, to run once:
  // setting baud rate 
  Serial.begin(115200);

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
  webServer.begin();
  dp("Web server started");

}

void handleRoot(){
  dp("getting root page")
  String toSend = getPageTop();
  toSend += getPageBody();
  toSend += getPageFooter();

  webServer.send(200, "text/html", toSend);
}

String getPageTop() {
  return
    "<html><head><title>COM3506 IoT </title>\n"
    "<meta charset=\"utf-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "\n<style>body{background:#FFF; color: #000; "
    "font-family: sans-serif; font-size: 150%;}</style>\n"
    "</head><body>\n"
  ;
};
String getPageBody() {
  return "<h2>Welcome to Thing!</h2>\n";
}
String getPageFooter() {
  return "\n<p><a href='/'>Home</a>&nbsp;&nbsp;&nbsp;</p></body></html>\n";
}


String rootPage(){
  return
    "<html>\n"
    "<head>\n"
    "<title>COM3506 IoT</title>\n"
    "<meta charset=\"utf-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "\n<style>body{background:#FFF; color: #000; font-family: sans-serif; font-size: 150%;}</style>\n"
    "</head>\n"
    "<body>\n"
    "<h2>Welcome to Thing!</h2>\n"
    "<p>\n"
    "<a href='/'>Home</a>&nbsp;&nbsp;&nbsp;</p></body></html>\n"
  ;

}


void loop() {
  // put your main code here, to run repeatedly:
  
  // deal with any pending web requests
  webServer.handleClient(); 
}
