/****
 *
 * Sketch to fill the Arduino EEPROM used by BendulumClock with a linear calibration model
 *
 *   Copyright 2015 by D. L. Ehnebuske 
 *   License terms: Creative Commons Attribution-ShareAlike 3.0 United States (CC BY-SA 3.0 US) 
 *                  See http://creativecommons.org/licenses/by-sa/3.0/us/ for specifics. 
 *
 ****/ 

#define SKETCH_ID  F("Fill BendulumClock EEPROM. V0.20")

#include <Escapement.h>                        // Escapement library (needed for settings_t definition)
#include <avr/eeprom.h>                        // EEPROM read write library
#include <Wire.h>;                             // Trick IDE into including Wire library (needed by Escapement)

settings_t eeprom;                             // The structure into which we read the EEPROM contents

// Linear model: y = m * x + b where x is the temperature in degrees Celcium, y is the beat duration in μs, m is the slobe and b is the y intercept
int m = 140;                                   // Slope
long b = 444657;                               // y intercept

// Setup function -- called upon power-on and reset
void setup() {
  Serial.begin(9600);                          // Prep for writing to console
  Serial.println(SKETCH_ID);                   // Say who we are
  eeprom_read_block((void*)&eeprom, (const void*)0, sizeof(eeprom)); // Read from EEPROM -- we assume that it's only the model we'll replace
  eeprom.speedAdj = 0;                         // No manual adjustment
  eeprom.compensated = true;                   // Temperature compensated model, for sure
  Serial.print(F("ID: 0x"));                   // Print ID
  Serial.print(eeprom.id, HEX);
  Serial.print(F(", bias: "));                 // Print bias
  Serial.print(eeprom.bias/10.0, 1);
  Serial.print(F(" sec/day, speedAdj: "));     // Print speedAdj
  Serial.print(eeprom.speedAdj/1.0);
  Serial.println(F(" usec, "));
  Serial.println(F("Temp\tus/b\tsampleCount"));
  for (int i = 0; i < TEMP_STEPS; i++) {       // Loop through the model calibration points
    eeprom.uspb[i] = long(m * (TEMP_MIN + 0.5 * i)) + b;  //   Set beat duration for i-th bucket
    eeprom.sampleCount[i] = TGT_SAMPLES + 1;   //   Indicate that calibration for this bucket is complete    
    Serial.print(TEMP_MIN + 0.5 * i);          //   Show our work
    Serial.print(F("\t"));
    Serial.print(eeprom.uspb[i]);
    Serial.print(F("\t"));
    Serial.println(eeprom.sampleCount[i]);
  }
eeprom_write_block((const void*)&eeprom, (void*)0, sizeof(eeprom)); // Write it to EEPROM
Serial.println(F("Written to eeprom"));
}

// Loop function -- called over and over so long as the power is on and no reset
void loop() {
                                               // Bored, nothing to do
}

