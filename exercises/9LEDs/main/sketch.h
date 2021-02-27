// sketch.h


// the absence of the ARDUINO_IDE_BUILD macro
// indicates this is not an Arduino IDE build


// if not in the IDE, variant macros etc. are missing, so define them here

#ifndef ARDUINO_FEATHER_ESP32
  #define ARDUINO_FEATHER_ESP32
#endif

#ifndef ARDUINO_ARCH_ESP32
  #define ARDUINO_ARCH_ESP32
#endif

#ifndef ARDUINO
  #define ARDUINO 10900
#endif

#ifndef ARDUINO_BOARD
  #define ARDUINO_BOARD "FEATHER_ESP32"
#endif

#ifndef ARDUINO_VARIANT
  #define ARDUINO_VARIANT "feather_esp32"
#endif
