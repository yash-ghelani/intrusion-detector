// sketch.h
// when this is an Arduino IDE build define the relevant macro
// (this allows us to differentiate this type of build from an IDF build)

#define ARDUINO_IDE_BUILD

// Debugging switches and macros
#define DEBUG 1 // Switch debug output on and off by 1 or 0

#if DEBUG
#define dp(s)   { Serial.println(F(s)); }
#define dps(s,v)   { Serial.print(F(s)); Serial.println(v);}
#else
#define dp(s)
#define dps(s,v)
#endif
