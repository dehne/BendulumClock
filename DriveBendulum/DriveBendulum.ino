/****
 *
 * Test a new bendulum attached to a bendulum clock shield and adjust the drive current as need be.
 *
 *   Copyright 2014 by D. L. Ehnebuske 
 *   License terms: Creative Commons Attribution-ShareAlike 3.0 United States (CC BY-SA 3.0 US) 
 *                  See http://creativecommons.org/licenses/by-sa/3.0/us/ for specifics. 
 *
 ****/

/****
 *
 *    Hardware pin assignment constants -- what's hooked up to which Arduino pins
 *
 ****/
#define COIL_PIN   (A2)                        // Bendulum coil sensing is through pin A2
#define LED_PIN    (13)                        // The built-in LED is on pin 13
#define KICK_PIN   (12)                        // The pin through which we kick the bendulum magnet through the coil is pin 12

/****
 *
 *  Initialization routine: Called once when power is applied or when the Arduino is reset
 *
 ****/

void setup() {
  analogReference(EXTERNAL);                   // We have an external reference: a 47k+47k voltage divider between 3.3V and Gnd
  pinMode(LED_PIN, OUTPUT);                    // We write to the LED pin
  digitalWrite(LED_PIN, LOW);                  // Turn LED off for sure
  pinMode(COIL_PIN, INPUT);                    // We read the coil pin
  pinMode(KICK_PIN, INPUT);                    // Put the kick pin in input (high impedance) mode initially
  Serial.begin(9600);                          // Initialize serial communications
  Serial.println("Bendulum Driver v1.0");      // Announce outselves
}

/****
 *
 *   Watch for the bendulum to pass over the coil. Returns when it detects the bendulum has just passed.
 *
 *   When the bendulum approaches the center of its swing, the magnet on it induces a current spike in the coil which
 *   shows up as a voltage on the input pin. We wait for the peak of this spike to pass and then return. This is 
 *   indicated by a fall in voltage.
 *
 ****/
void watchBendulum() {
                                               // Watch parameters
  const int noiseSize = 10;                    // Peak value for expected input noise
  const int settleTime = 250;                  // Time (ms) to delay to let things settle before looking for voltage spike

  static int currCoil = 0;                     // The current value read from coilPin. The value read here, in volts, is
                                               // 1024/AREF where AREF is the voltage on that pin. AREF is set by a 1:1
                                               // voltage divider between the 3.3V pin and Gnd, so 1.65V. Más o menos.
  static int pastCoil = 0;                     // The pervious value of currCoil
  
  delay(settleTime);                           // Wait for things to calm down
  do {                                         // Wait for the voltage to fall to zero
    currCoil = analogRead(COIL_PIN);
  } while (currCoil > 0);
  pastCoil = 0;                                // Start measurement history over
  while (currCoil >= pastCoil) {
                                               // While the bendulum magnet hasn't passed over coil
    pastCoil = currCoil;                       // Loop waiting for the induced value from the coil to begin to fall
    currCoil = analogRead(COIL_PIN) / noiseSize;
  }
}

/****
 *
 *   Kick the bendulum magnet with a pulse to keep the bendulum going.
 *
 ****/
void kickBendulum() {
                                               // Kick parameters
  const int delayTime = 0;                     // Time in ms by which to delay the start of the kick pulse
  const int kickTime = 50;                     // Duration in ms of the kick pulse
  
  pinMode(KICK_PIN, OUTPUT);                   // Prepare kick pin for output
  delay(delayTime);                            // Wait desired time before pin turn-on
  digitalWrite(LED_PIN, HIGH);                 // Turn LED on
  digitalWrite(KICK_PIN, HIGH);                // Turn kick pin on
  delay(kickTime);                             // Wait for duration of pulse
  digitalWrite(KICK_PIN, LOW);                 // Turn it off
  digitalWrite(LED_PIN, LOW);                  // Turn LED off, too
  pinMode(KICK_PIN, INPUT);                    // Put kick pin in high impedance mode
}

/****
 *
 *   Main loop: Called over and over until power goes off or the Arduino is reset
 *
 ****/

void loop() {
  
  watchBendulum();                             // Watch until the bendulum passes dead center
  kickBendulum();                              // Give the bendulum a kick
}
