/****
 *
 * Test a new bendulum attached to a bendulum clock shield and adjust the drive current as need be.
 *
 *   DriveBendulum v1.0, Copyright 2014 by D. L. Ehnebuske 
 *   License terms: Creative Commons Attribution-ShareAlike 3.0 United States (CC BY-SA 3.0 US) 
 *                  See http://creativecommons.org/licenses/by-sa/3.0/us/ for specifics. 
 *
 ****/

/****
 *
 *    Hardware pin assignment constants -- what's hooked up to which Arduino pins
 *
 ****/
#define SENSE_PIN   (A2)                       // Bendulum coil sensing is through pin A2
#define LED_PIN    (13)                        // The built-in LED is on pin 13
#define KICK_PIN   (12)                        // The pin through which we kick the bendulum magnet through the coil is pin 12

/****
 *
 *    Bendulum-coil interaction parameters
 *
 ****/
                                               // Voltage spike detection parameters
#define NOISE_SIZE  (30)                       //   Peak value for expected input noise
#define SETTLE_TIME (125)                      //   Time (ms) to delay to let things settle before looking for voltage spike

                                               // Kick parameters
#define DELAY_TIME  (0)                        //   Time in ms by which to delay the start of the kick pulse
#define KICK_TIME   (50)                       //   Duration in ms of the kick pulse


/****
 *
 *  Initialization routine: Called once when power is applied or when the Arduino is reset
 *
 ****/

void setup() {
  analogReference(EXTERNAL);                   // We have an external reference: a 47k+47k voltage divider between 3.3V and Gnd
  pinMode(LED_PIN, OUTPUT);                    // We write to the LED pin
  digitalWrite(LED_PIN, LOW);                  // Turn LED off for sure
  pinMode(SENSE_PIN, INPUT);                   // We read the sense pin
  pinMode(KICK_PIN, INPUT);                    // Put the kick pin in input (high impedance) mode initially
  Serial.begin(9600);                          // Initialize serial communications
  Serial.println("Bendulum Driver v1.0");      // Announce outselves
}

/****
 *
 *   Watch for the bendulum to pass over the coil. Returns when it detects the bendulum has just passed.
 *
 *   When the bendulum approaches the center of its swing, the magnet on it induces a current spike in the coil which
 *   shows up as a voltage on the input pin. We wait for the peak of this spike to pass and then return. Peak passage is 
 *   indicated by a fall in voltage.
 *
 *   The value from analogRead(), in volts, is AREF/1024 where AREF is the voltage on that pin. AREF is set by a 1:1 
 *   voltage divider between the 3.3V pin and Gnd, so 1.65V. Más o menos. This makes each count about 1.6mV. The value 
 *   read is divided by NOISE_SIZE to eliminate false peak detections.
 *
 ****/
void watchBendulum() {
  static int currCoil = 0;                     // The scaled value read from SENSE_PIN.
  static int pastCoil = 0;                     // The pervious value of currCoil
  
  delay(SETTLE_TIME);                          // Wait for things to calm down from last time
  do {                                         // Wait for the voltage to fall to zero
    currCoil = analogRead(SENSE_PIN);
  } while (currCoil > 0);
  pastCoil = 0;                                // Start measurement history over
  while (currCoil >= pastCoil) {
                                               // While the bendulum magnet hasn't passed over coil
    pastCoil = currCoil;                       // Loop waiting for the induced value from the coil to begin to fall
    currCoil = analogRead(SENSE_PIN) / NOISE_SIZE;
  }
}

/****
 *
 *   Kick the bendulum magnet with a pulse to keep the bendulum going.
 *
 ****/
void kickBendulum() {
  pinMode(KICK_PIN, OUTPUT);                   // Prepare kick pin for output
  delay(DELAY_TIME);                           // Wait desired time before pin turn-on
  digitalWrite(LED_PIN, HIGH);                 // Turn LED on
  digitalWrite(KICK_PIN, HIGH);                // Turn kick pin on
  delay(KICK_TIME);                            // Wait for duration of pulse
  digitalWrite(KICK_PIN, LOW);                 // Turn it off
  digitalWrite(LED_PIN, LOW);                  // Turn LED off, too
  pinMode(KICK_PIN, INPUT);                    // Put kick pin back in high impedance mode
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
