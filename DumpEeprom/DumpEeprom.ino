/****
 *
 * Sketch to dump and format the contents of the Arduino EEPROM used by BendulumClock; Calculate least squares model.
 *
 ****/ 

#define SKETCH_ID  F("Dump BendulumClock EEPROM. V0.30")

#include <Escapement.h>                        // Escapement library (needed for settings_t definition)
#include <avr/eeprom.h>                        // EEPROM read write library
#include <Wire.h>;                             // Trick IDE into including Wire library (needed by Escapement)

settings_t eeprom;                             // The structure into which we read the EEPROM contents

// Setup function -- called upon power-on and reset
void setup() {
  
  float temp = 0.0;
  float uspb = 0.0;
  float slope = 0.0;
  float intercept = 0.0;
  
//linear regression variables
  float xSum = 0.0;
  float ySum = 0.0;
  float xxSum = 0.0;
  float xySum = 0.0;
  int count = 0;

  Serial.begin(9600);                          // Prep for writing to console
  Serial.println(SKETCH_ID);                   // Say who we are
  eeprom_read_block((void*)&eeprom, (const void*)0, sizeof(eeprom)); // Read from EEPROM
  Serial.print(F("ID: 0x"));                   // Print ID
  Serial.print(eeprom.id, HEX);
  Serial.print(F(", bias: "));                 // Print bias
  Serial.print(eeprom.bias/10.0, 1);
  Serial.print(F(" sec/day, peakScale: "));    // Print peakScale
  Serial.print(eeprom.peakScale);
  Serial.print(F(", deltaUspb: "));            // Print deltaUspb
  Serial.print(eeprom.deltaUspb);
  Serial.print(F(" usec, "));
  if (eeprom.compensated) {
    Serial.println(F("temperature compensated. Calibration:"));
    Serial.println(F("Temp\tus/b"));
    for (int i = 0; i < TEMP_STEPS; i++) {
      if (eeprom.uspb[i] > 0) {
        temp = eeprom.temp[i] / 256.0;
        Serial.print(temp, 4);
        Serial.print(F("\t"));
        Serial.println(eeprom.uspb[i]);
        uspb = float (eeprom.uspb[i]);
        xSum += temp;
        ySum += uspb;
        xxSum += temp * temp;
        xySum += temp * uspb;
        count++;
      }
    }
    if (count > 1) {
      slope = (count * xySum - xSum * ySum) / (count * xxSum - xSum * xSum);
      intercept = (ySum - slope * xSum) / count;
      Serial.print(F("Based on "));
      Serial.print(count);
      Serial.print(F(" calibration points, a linear model says uspb = "));
      Serial.print(slope, 4);
      Serial.print(F(" * temp + "));
      Serial.println(intercept, 4);
    } else {
      Serial.println("Not enough calibration data to do a calibration fit.");
    }
  } else {
    Serial.print(F("not temperature compensated, Calibration: "));
    Serial.println(eeprom.uspb[0]);
  }
}

// Loop function -- called over and over so long as the power is on and no reset
void loop() {
}
