/****
 *
 * Sketch to dump and format the contents of the Arduino EEPROM used by BendulumClock
 *
 *   Copyright 2014-2016 by D. L. Ehnebuske 
 *   License terms: Creative Commons Attribution-ShareAlike 3.0 United States (CC BY-SA 3.0 US) 
 *                  See http://creativecommons.org/licenses/by-sa/3.0/us/ for specifics. 
 *
 ****/ 

#define SKETCH_ID  F("Dump BendulumClock EEPROM. V0.85")

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
    float xSum = 0.0;				//   Have a go at calculating the linear least squares for the data so far
    float ySum = 0.0;
    float xxSum = 0.0;
    float xySum = 0.0;
    float slope;
    float yIntercept;
    int count = 0;
    for (int i = 0; i < TEMP_STEPS; i++) {
      if (eeprom.curSmoothing[i] > TGT_SMOOTHING) {
        float x = (((TEMP_MIN << 1) + i) << 7);
        float y = eeprom.uspb[i];
        count++;
        xSum += x;
        ySum += y;
        xxSum += x * x;
        xySum += x * y;
      }
    }
    if (count >= 1) {				//   If at least one bucket is complete, calculate
      slope = count > 1 ? (count * xySum - xSum * ySum) / (count * xxSum - xSum * xSum) : 0;
      yIntercept = (ySum - slope * xSum) / count;
      Serial.print(F("LSQ model: uspb = "));
      Serial.print(slope);
      Serial.print(F(" * t + "));
      Serial.println(yIntercept);
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
