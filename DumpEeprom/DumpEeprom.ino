/****
 *
 * Sketch to dump and format the contents of the Arduino EEPROM used by BendulumClock
 *
 ****/ 

#define SKETCH_ID  F("Dump BendulumClock EEPROM. V0.10")

#include <Escapement.h>                        // Escapement library (needed for settings_t definition)
#include <avr/eeprom.h>                        // EEPROM read write library
#include <Wire.h>;                                 // Trick IDE into including Wire library (needed by Escapement)

	int id;									// ID tag to know whether data (probably) belongs to this sketch
	int bias;								// Empirically determined correction factor for the real-time clock in 0.1 s/day
	int peakScale;							// Empirically determined scaling factor for peak induced voltage
	long deltaUspb;							// Speed adjustment factor, μs per beat
	bool compensated;						// Set to true if the Escapement is temperature compensated, else false
	long uspb[TEMP_STEPS];					// Empirically determined, temp-dependent, μs per beat.
	int curSmoothing[TEMP_STEPS];			// Current temp-dependent smoothing factor


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
  Serial.print(F(" sec/day, peakScale: "));    // Print peakScale
  Serial.print(eeprom.peakScale);
  Serial.print(F(", deltaUspb: "));            // Print deltaUspb
  Serial.print(eeprom.deltaUspb);
  Serial.print(F(" usec, "));
  if (eeprom.compensated) {
    Serial.println(F("temperature compensated. Calibration:"));
    Serial.println(F("Temp\tus/b\tSmooth"));
    for (int i = 0; i < TEMP_STEPS; i++) {
      if (eeprom.curSmoothing[i] > 1) {
        Serial.print(TEMP_MIN + i*0.5, 1);
        Serial.print(F("\t"));
        Serial.print(eeprom.uspb[i]);
        Serial.print(F("\t"));
        Serial.println(eeprom.curSmoothing[i]);
      }
    }
  } else {
    Serial.print(F("not temperature compensated, Calibration: "));
    Serial.print(eeprom.uspb[0]);
    Serial.print(F(" us/beat, smoothing: "));
    Serial.println(eeprom.curSmoothing[0]);
  }
}

// Loop function -- called over and over so long as the power is on and no reset
void loop() {
}
