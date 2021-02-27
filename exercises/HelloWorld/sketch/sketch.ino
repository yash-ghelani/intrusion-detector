// main.cpp
// main entry points

#include "sketch.h"
#include <Arduino.h>
#include <Wire.h>
#include <esp_log.h>

// to blink or not to blink...
bool doBlinking = true;

// MAC address 
uint32_t chipId = 0;

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
// arduino-land entry points

void setup() {
  // IDF 4.3 Wire.setPins(SDA, SCL);
  // Wire.begin();

  Serial.begin(115200);
  Serial.println("arduino started");
  Serial.printf("\nwire pins: sda=%d scl=%d\n", SDA, SCL);

  pinMode(BUILTIN_LED, OUTPUT);
} // setup

void loop() {
  printf("\nahem, hello world\n");

  #ifdef ESP_IDF_VERSION_MAJOR
    printf( // IDF version
      "IDF version: %d.%d.%d\n",
      ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR, ESP_IDF_VERSION_PATCH
    );
  #endif
  #ifdef ESP_ARDUINO_VERSION_MAJOR
    printf(
      "ESP_ARDUINO_VERSION_MAJOR=%d; MINOR=%d; PATCH=%d\n",
      ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR,
      ESP_ARDUINO_VERSION_PATCH
    );
  #endif
  #ifdef ARDUINO_FEATHER_ESP32
    printf("ARDUINO_FEATHER_ESP32 is defined\n");
  #endif
  #ifdef ARDUINO_ARCH_ESP32
    printf("ARDUINO_ARCH_ESP32 is defined\n");
  #endif
  #ifdef ESP_PLATFORM
    printf("ESP_PLATFORM is defined\n");
  #endif
  #ifdef ESP32
    printf("ESP32 is defined\n");
  #endif
  #ifdef IDF_VER
    printf("IDF_VER=%s\n", IDF_VER);
  #endif
  #ifdef ARDUINO
    printf("ARDUINO=%d\n", ARDUINO);
  #endif
  #ifdef ARDUINO_BOARD
    printf("ARDUINO_BOARD=%s\n", ARDUINO_BOARD);
  #endif
  #ifdef ARDUINO_VARIANT
    printf("ARDUINO_VARIANT=%s\n", ARDUINO_VARIANT);
  #endif
  #ifdef ARDUINO_SERIAL_PORT
    printf("ARDUINO_SERIAL_PORT=%d\n", ARDUINO_SERIAL_PORT);
  #endif
  #ifdef ARDUINO_IDE_BUILD
    printf("ARDUINO_IDE_BUILD is defined\n");
  #else
    printf("no definition of ARDUINO_IDE_BUILD\n");
  #endif

  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  Serial.print("Chip ID: "); Serial.println(chipId);

  if(doBlinking) digitalWrite(BUILTIN_LED, HIGH);
  WAIT_SECS(2)
  if(doBlinking) digitalWrite(BUILTIN_LED, LOW);
  WAIT_SECS(2)
} // loop


/////////////////////////////////////////////////////////////////////////////
// if we're an IDF build define app_main
// (TODO probably fails to identify a platformio *idf* build)

#if ! defined(ARDUINO_IDE_BUILD) && ! defined(PLATFORMIO)
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
