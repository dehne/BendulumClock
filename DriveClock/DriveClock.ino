/****
 *
 * Directly drive a Lavet motor clock mechanism attached to a bendulum shield.
 *
 *   Copyright 2014 by D. L. Ehnebuske 
 *   License terms: Creative Commons Attribution-ShareAlike 3.0 United States (CC BY-SA 3.0 US) 
 *                  See http://creativecommons.org/licenses/by-sa/3.0/us/ for specifics. 
 *
 ****/

/****
 *    Constants
 ****/
#define LED_PIN (13)                           // LED is on pin 13
#define TICK_PIN (11)                          // The pin to pulse to tick the clock forward one second
#define TOCK_PIN (10)                          // The pin to pulse to tock the clock forward one second


void setup() {
  pinMode(LED_PIN, OUTPUT);                     // We write to the LED pin
  digitalWrite(LED_PIN, LOW);                   // Turn LED off for sure
  pinMode(TICK_PIN, OUTPUT);                    // Set up the tick pin of the clock's Lavet motor
  digitalWrite(TICK_PIN, LOW);
  pinMode(TOCK_PIN, OUTPUT);                    // Set up the tock pin of the clock's Lavet motor
  digitalWrite(TOCK_PIN, LOW);
  Serial.begin(9600);                           // Set up serial communications
  Serial.println("Clock Driver v1.0");          // Announce outselves
}

/****
 *
 *  Move the clock's Lavet motor one step (i.e., one second) forward.
 *
 *  The Lavet motor of the clock movement is connected, with a series resistor, to pins tickPin 
 *  and tockPin. On a tick, a pulse goes to tickPin (and gets sunk on tockPin). On a tock, the pulse 
 *  goes the other way.
 *
 ****/

void stepClock() {
                                                // Step parameters
  const int pulseTime = 40;                     // Duration in ms of pulses
  
  static boolean tick = true;                   // Whether the lavet motor needs a tick or a tock
  
  digitalWrite(LED_PIN, HIGH);                  // Turn on LED
  if (tick) {
    digitalWrite(TICK_PIN, HIGH);               // Issue a tick pulse
    delay(pulseTime);
    digitalWrite(TICK_PIN, LOW);
  } else {
    digitalWrite(TOCK_PIN, HIGH);               // Issue a tock pulse
    delay(pulseTime);
    digitalWrite(TOCK_PIN, LOW);
  }
  digitalWrite(LED_PIN, LOW);                   // Turn off LED
  tick = !tick;                                 // Switch from tick to tock or vice versa
}

/****
 *
 *  Main loop
 *
 ****/

void loop() {
  
  stepClock();                                 // Step the clock forward by one second
  delay(970);                                  // Delay a little less than 1 sec
}
