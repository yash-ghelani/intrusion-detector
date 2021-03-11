// main.cpp
// main entry points

#include "sketch.h"
#include <Arduino.h>
#include <Wire.h>
#include <esp_log.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "movingAvg.h"  // https://github.com/JChristensen/movingAvg
#include "private.h"


/////////////////////////////////////////////////////////////////////////////
// utilities

// delay/yield macros
#define WAIT_A_SEC   vTaskDelay(    1000/portTICK_PERIOD_MS); // 1 second
#define WAIT_SECS(n) vTaskDelay((n*1000)/portTICK_PERIOD_MS); // n seconds
#define WAIT_MS(n)   vTaskDelay(       n/portTICK_PERIOD_MS); // n millis

int firmwareVersion = 100; // used to check for updates
#define ECHECK ESP_ERROR_CHECK_WITHOUT_ABORT

// IDF logging
static const char *TAG = "main";


/////////////////////////////////////////////////////////////////////////////
// constants etc.

// Serial speed /////////////////////////////////////////////////////////////
#define SERIAL_SPEED 115200

// MAC address //////////////////////////////////////////////////////////////
char *getMAC(char *buf); // get the MAC address
extern char MAC_ADDRESS[13]; // MACs are 12 chars, plus the NULL terminator

// OTA support, WiFi ////////////////////////////////////////////////////////
const char *myName = "IFTTTThingIDF";
WiFiMulti WiFiMulti; // manage multiple access points

// ultrasonic sensor ////////////////////////////////////////////////////////
const int LEVEL_TRIG_PIN=A0;
const int LEVEL_ECHO_PIN=A2;
long getDistance();             // read the distance
uint32_t zeroesSinceLast = 0;
movingAvg distanceAvg(10);      // average of the last 10 values

// blink internal LED ///////////////////////////////////////////////////////
void blinkFeatherLED(uint8_t times);

// tweet via IFTTT
void setClock(); // we need the time to check certif expiry
void doPOST(String, String); // HTTPS POST
void iftttTweet(uint64_t, long, uint32_t, int); // IFTTT hookup


/////////////////////////////////////////////////////////////////////////////
// if we're an IDF build define app_main
#ifndef ARDUINO_IDE_BUILD

  extern "C" { void app_main(); }

  // main entry point
  void app_main() {
    // arduino land
    initArduino();
    setup();
    while(1)
      loop();
  } // app_main()

#endif

/////////////////////////////////////////////////////////////////////////////
// setup ////////////////////////////////////////////////////////////////////
void setup() {
  Wire.setPins(SDA, SCL);
  Wire.begin();

  Serial.begin(SERIAL_SPEED);           // init Serial
  Serial.printf("Serial initialised at %d\n", SERIAL_SPEED);

  getMAC(MAC_ADDRESS);                  // store the MAC
  Serial.printf("MAC address is %s\n", MAC_ADDRESS);

  esp_log_level_set("*", ESP_LOG_ERROR); // IDF logging

  // NOTE: doesn't work with hidden SSIDs; use WiFi.begin() instead for those
  /*
  // the WiFiMulti class tries multiple access points; define their SSIDs and
  // passkeys in private.h, e.g.
  //   #define _MULTI_SSID1 "my-hotspot"
  //   #define _MULTI_KEY1  "my-key"
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(_MULTI_SSID1, _MULTI_KEY1);
  WiFiMulti.addAP(_MULTI_SSID2, _MULTI_KEY2);
  WiFiMulti.addAP(_MULTI_SSID3, _MULTI_KEY3);
  Serial.print("waiting for WiFi to connect...");
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.print(".");
  }
  Serial.println(" connected");
  */
  WiFi.begin(_UOS_OTHER_SSID, _UOS_OTHER_KEY);
  delay(500); // let wifi settle

  pinMode(BUILTIN_LED, OUTPUT);          // turn built-in LED on

  pinMode(LEVEL_TRIG_PIN, OUTPUT);       // set up the ultrasonic...
  pinMode(LEVEL_ECHO_PIN, INPUT);        // ...sensor pins

  distanceAvg.begin();                   // init the moving average
  setClock();                            // get NTP to set the time

  blinkFeatherLED(3);                    // signal we've finished config
}


/////////////////////////////////////////////////////////////////////////////
// looooooooooooooooooooop //////////////////////////////////////////////////
uint64_t loopIteration = 0;
void loop() {
  int gesture = 50;                     // 50 = no, 150 = yes
  long distance = getDistance();        // current sensor reading

  if(distance == 0) {                   // ignore zero readings
    zeroesSinceLast++;
  } else {
    int avg = distanceAvg.reading(distance); // add reading to moving average

    if(avg - distance > 100) {          // big deviations = "gestures"
      gesture = 150;
      Serial.printf(                    // register a gesture
        "GESTURE! distance: %4ld zeroes: %4u rolling: %4d\n",
        distance, zeroesSinceLast, avg
      );

      iftttTweet(loopIteration, distance, zeroesSinceLast, avg); // tweet it
      delay(2000);                      // don't send too many tweets!
    }

    if(false) Serial.printf(                      // print data for plotter
      "distance: %4ld gesture: %4d zeroes: %4u rolling: %4d\n",
      distance, gesture, zeroesSinceLast, avg
    );
    zeroesSinceLast = 0;
  }
  delay(35);                            // prevent sensor bouncing
  loopIteration++;
}


/////////////////////////////////////////////////////////////////////////////
// talk to IFTTT ////////////////////////////////////////////////////////////
void iftttTweet(
  uint64_t loopIteration, long distance, uint32_t zeroesSinceLast, int avg
) {
  // create a string describing the gesture
  char buf[1024], body[1024];
  sprintf(
    buf,
    "loop#: %4llu distance: %4ld zeroes: %4u rolling: %4d",
    loopIteration, distance, zeroesSinceLast, avg
  );
  sprintf(
    body, "{ \"value1\": \"%s\", \"value2\": \"\", \"value3\": \"\" }\n", buf
  );
  Serial.printf("posting %s\n", body);

  String url(
    "https://maker.ifttt.com/trigger/waving-at-twitter/with/key/" _IFTTT_KEY
  );
  doPOST(url, String(body));
}

void doPOST(String url, String body) {
  char *iftttRootCertif = 
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDxTCCAq2gAwIBAgIBADANBgkqhkiG9w0BAQsFADCBgzELMAkGA1UEBhMCVVMx\n"
    "EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxGjAYBgNVBAoT\n"
    "EUdvRGFkZHkuY29tLCBJbmMuMTEwLwYDVQQDEyhHbyBEYWRkeSBSb290IENlcnRp\n"
    "ZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5MDkwMTAwMDAwMFoXDTM3MTIzMTIz\n"
    "NTk1OVowgYMxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6b25hMRMwEQYDVQQH\n"
    "EwpTY290dHNkYWxlMRowGAYDVQQKExFHb0RhZGR5LmNvbSwgSW5jLjExMC8GA1UE\n"
    "AxMoR28gRGFkZHkgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIw\n"
    "DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL9xYgjx+lk09xvJGKP3gElY6SKD\n"
    "E6bFIEMBO4Tx5oVJnyfq9oQbTqC023CYxzIBsQU+B07u9PpPL1kwIuerGVZr4oAH\n"
    "/PMWdYA5UXvl+TW2dE6pjYIT5LY/qQOD+qK+ihVqf94Lw7YZFAXK6sOoBJQ7Rnwy\n"
    "DfMAZiLIjWltNowRGLfTshxgtDj6AozO091GB94KPutdfMh8+7ArU6SSYmlRJQVh\n"
    "GkSBjCypQ5Yj36w6gZoOKcUcqeldHraenjAKOc7xiID7S13MMuyFYkMlNAJWJwGR\n"
    "tDtwKj9useiciAF9n9T521NtYJ2/LOdYq7hfRvzOxBsDPAnrSTFcaUaz4EcCAwEA\n"
    "AaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYE\n"
    "FDqahQcQZyi27/a9BUFuIMGU2g/eMA0GCSqGSIb3DQEBCwUAA4IBAQCZ21151fmX\n"
    "WWcDYfF+OwYxdS2hII5PZYe096acvNjpL9DbWu7PdIxztDhC2gV7+AJ1uP2lsdeu\n"
    "9tfeE8tTEH6KRtGX+rcuKxGrkLAngPnon1rpN5+r5N9ss4UXnT3ZJE95kTXWXwTr\n"
    "gIOrmgIttRD02JDHBHNA7XIloKmf7J6raBKZV8aPEjoJpL1E/QYVN8Gb5DKj7Tjo\n"
    "2GTzLH4U/ALqn83/B2gX2yKQOC16jdFU8WnjXzPKej17CuPKf1855eJ1usV2GDPO\n"
    "LPAvTK33sefOT6jEm0pUBsV/fdUID+Ic/n4XuKxe9tQWskMJDE32p2u0mYRlynqI\n"
    "4uJEvlz36hz1\n"
    "-----END CERTIFICATE-----\n";

  HTTPClient https;
  if(https.begin(url, iftttRootCertif)) { // HTTPS
    https.addHeader("User-Agent", "ESP32");
    https.addHeader("Content-Type", "application/json");

    Serial.printf("[HTTPS] POST %.62s...\n", url.c_str());
    //Serial.println(iftttRootCertif);

    // start connection and send HTTP header
    int httpCode = https.POST((uint8_t *) body.c_str(), strlen(body.c_str()));

    // httpCode will be negative on error
    if(httpCode > 0) {
      // HTTP header has been send and response header has been handled
      Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

      // file found at server
      if(
        httpCode == HTTP_CODE_OK ||
        httpCode == HTTP_CODE_MOVED_PERMANENTLY
      ) {
        String payload = https.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf(
        "[HTTPS] POST... failed, error: %s\n",
        https.errorToString(httpCode).c_str()
      );
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
} // doPOST()

// not sure if WiFiClientSecure checks the validity date of the certificate,
// but setting clock just to be sure...
void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print(F("waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("current time: "));
  Serial.print(asctime(&timeinfo));
}

/////////////////////////////////////////////////////////////////////////////
// read from ultrasonics ////////////////////////////////////////////////////
long getDistance() { // don't call again without e.g. 35 mS debounce delay
  long duration;
  int TIMEOUT = 15000;                          // how long to wait for pulse

  digitalWrite(LEVEL_TRIG_PIN, LOW);            // prepare for ping
  delayMicroseconds(2); // or 5?
  digitalWrite(LEVEL_TRIG_PIN, HIGH);           // start ping
  delayMicroseconds(10);                        // allow 10ms ping
  digitalWrite(LEVEL_TRIG_PIN, LOW);            // stop ping
  duration = pulseIn(LEVEL_ECHO_PIN, HIGH, TIMEOUT); // wait for response

  // distance = traveltime/2 (there and back)  x  speed of sound
  // (the speed of sound is: 343m/s = 0.0343 cm/uS = 1/29.1 cm/uS)
  return ( duration / 2 ) / 29.1; // cms
}

/////////////////////////////////////////////////////////////////////////////
// misc utils ///////////////////////////////////////////////////////////////

void blinkFeatherLED(uint8_t times) {
  for(int i=0; i<times; i++) {
    digitalWrite(BUILTIN_LED, LOW);
    delay(300);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(300);
  }
}
char MAC_ADDRESS[13]; // MAC addresses are 12 chars, plus the NULL terminator
char *getMAC(char *buf) { // the MAC is 6 bytes, so needs careful conversion..
  uint64_t mac = ESP.getEfuseMac(); // ...to string (high 2, low 4):
  char rev[13];
  sprintf(rev, "%04X%08X", (uint16_t) (mac >> 32), (uint32_t) mac);

  // the byte order in the ESP has to be reversed relative to normal Arduino
  for(int i=0, j=11; i<=10; i+=2, j-=2) {
    buf[i] = rev[j - 1];
    buf[i + 1] = rev[j];
  }
  buf[12] = '\0';
  return buf;
}
