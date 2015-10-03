/****
 *
 * Sketch to dump and format the contents of the Arduino EEPROM used by BendulumClock
 *
 *   Copyright 2014-2015 by D. L. Ehnebuske 
 *   License terms: Creative Commons Attribution-ShareAlike 3.0 United States (CC BY-SA 3.0 US) 
 *                  See http://creativecommons.org/licenses/by-sa/3.0/us/ for specifics. 
 *
 ****/ 

#define SKETCH_ID  F("Dump BendulumClock EEPROM. V0.50")

#include <Escapement.h>                        // Escapement library (needed for settings_t definition)
#include <avr/eeprom.h>                        // EEPROM read write library
#include <Wire.h>;                             // Trick IDE into including Wire library (needed by Escapement)

settings_t eeprom;                             // The structure into which we read the EEPROM contents

// Setup function -- called upon power-on and reset
void setup() {
  Serial.begin(9600);                          // Prep for writing to console
  Serial.println(SKETCH_ID);                   // Say who we are
  eeprom_read_block((void*)&eeprom, (const void*)0, sizeof(eeprom)); // Read from EEPROM
  Serial.print(F("ID: 0x"));                   // Print ID
  Serial.print(eeprom.id, HEX);
  Serial.print(F(", bias: "));                 // Print bias
  Serial.print(eeprom.bias/10.0, 1);
  Serial.print(F(" sec/day, deltaUspb: "));    // Print deltaUspb
  Serial.print(eeprom.deltaUspb);
  Serial.print(F(" usec, "));
  if (eeprom.compensated) {
    Serial.println(F("temperature compensated. Calibration:"));
    Serial.println(F("Temp\tus/b\tSmoothing"));
    for (int i = 0; i < TEMP_STEPS; i++) {
      if (eeprom.uspb[i] > 0) {
        Serial.print(TEMP_MIN + 0.5 * i);
        Serial.print(F("\t"));
        Serial.print(eeprom.uspb[i]);
        Serial.print(F("\t"));
        Serial.println(eeprom.curSmoothing[i]);
      }
    }
  } else {
    Serial.print(F("not temperature compensated, Calibration: "));
    Serial.println(F("us/b\tSmoothing"));
    Serial.print(eeprom.uspb[0]);
    Serial.print(F("\t"));
    Serial.println(eeprom.curSmoothing[0]);
  }
}

// Loop function -- called over and over so long as the power is on and no reset
void loop() {
                                               // Talk about nothing to do
}
