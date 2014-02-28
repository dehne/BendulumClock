/****
 *
 *   Arduino Realtime Clock Calibration v0.10
 *
 *   Copyright 2014 by D. L. Ehnebuske 
 *   License terms: Creative Commons Attribution-ShareAlike 3.0 United States (CC BY-SA 3.0 US) 
 *                  See http://creativecommons.org/licenses/by-sa/3.0/us/ for specifics. 
 *
 *   This sketch implements a means to calibrate the realtime clock in an Arduino Uno used to drive a bendulum or
 *   pendulum clock using a bendulum clock shield and the Electronic Escapement sketch. One of the steps in setting 
 *   up such a clock is to run "autocal" which uses the Arduino's realtime clock to determine the period of the 
 *   bendulum or pendulum. Inherent in that process is the asumption that the Arduino's realtime clock is accurate. 
 *   It turns out that the realtime clock is good enough for its intended purpose, but not for time keeping because 
 *   it is based on a ceramic resonator, not a quartz crystal. As a result, the autocal can be off by seven minutes or
 *   more. This can be corrected for by fine tuning the clocks run rate, but it would be nice if autocal worked beter.
 *
 *   While the ceramic resonator in an Arduino is not particularly accurate, it's pretty good when it comes to 
 *   stability. Put another way, What a given Arduino's realtime clock thinks is a microsecond, might be wrong, its
 *   wrong by pretty close to the same amount every time. We can take advantage of this by calibrating the realtime
 *   clock. That's what this sketch does.
 *
 *   Basically, we use the realtime clock to drive a clock display directly instead of using bendulum or pendulum as 
 *   the isochronous element. We let the clock run, and use the IR remote to adjust its run rate in the same way we 
 *   use it to adjust the run rate of the clock when it's under control of the bendulum or pendulum. The adjustment 
 *   factor developed in this process is correction for the Arduino's realtime clock. It's stored in EEPROM where
 *   the Electronic Escapement sketch can pick it up and use it during autocal.
 *
 *   As with the Electronic Escapement, regulation and adjustment of the escapement is controlled by using one of 
 *   the little red SparkFun IR remotes. The remote has the following buttons and purposes:
 *
 *           Button        Function
 *           ======        ========================================================================================
 *           Power         Turn on/off clock adjustments. Make and store any pending speed adjustment in EEPROM
 *           A             Decrement adjustment step size
 *           B             Cancel any pending speed adjustment or on-going display adjustment
 *           C             Increment adjustment step size
 *           Up            Add step interval to pending speed adjustment
 *           Down          Sobtract step interval from pending speed adjustment
 *           Left          Pause time display for the amount of the adjustment step
 *           Right         Fast-forward the time display by the amount of the adjustment step
 *           Select        No function
 *
 *   The size of the current adjustment step is indicated by the color of the LED. The colors and corresponding step 
 *   sizes are:
 *
 *           Color     Meaning
 *           ========= ============================================================================================
 *           Red       0.1 sec
 *           Yellow    1 sec
 *           Green     10 sec
 *           Cyan      1 min
 *           Blue      10 min
 *           Magenta   1 hr
 *
 *   The LED is dark when adjustment is turned off. Press Power to turn it back on. The LED flashes white each time a 
 *   command is received from the remote control.
 *
 *   In other words, adjustment works the same way as it does for the electronic escapement. See that sketch for more
 *   details.
 *
 ****/

/****
 *
 *    Library header includes
 *
 ****/

#include <avr/eeprom.h>                        // EEPROM read write library
#include <Clock.h>                             // Lavet motor clock library
#include <IRremote.h>                          // IR remote control library (http://github.com/shirriff/Arduino-IRremote)
#include <RgbLed.h>                            // Simple RGB LED library

/**** 
 *
 *    Hardware pin assignment constants -- what's hooked up to which Arduino pins
 *
 ****/
#define CS_PIN       (2)                       // The jumper that tells whether we should do a cold start is on pin 2
#define IR_PIN       (3)                       // The IR receiver is attached to pin 9
#define LED_R_PIN    (9)                       // The pin to which the red LED is hooked
#define LED_G_PIN    (6)                       // The pin to which the green LED is hooked
#define LED_B_PIN    (5)                       // The pin to which the blue LED is hooked

/****
 *
 * Constants for the SparkFun IR remote control
 *
 ****/
#define SFIRR_REPEAT_PAUSE (3)                 // Number of repeat codes to ignore before deciding the user means it
#define SFIRR_POWER  0x10EFD827                // "Power" button IR code
#define SFIRR_A      0x10EFF807                // "A" button IR code
#define SFIRR_B      0x10EF7887                // "B" button IR code
#define SFIRR_C      0x10EF58A7                // "C" button IR code
#define SFIRR_UP     0x10EFA05F                // "Up" button IR code
#define SFIRR_DOWN   0x10EF00FF                // "Down" button IR code
#define SFIRR_LEFT   0x10EF10EF                // "Left" button IR code
#define SFIRR_RIGHT  0x10EF807F                // "Right" button IR code
#define SFIRR_SELECT 0x10EF20DF                // "Select" button IR code

/****
 *
 * Other constants
 *
 ****/
#define SETTINGS_TAG (0xA020)                  // Tag marking the settings structure as ours
#define VERSION_STRING "RTC Calibrate v0.10."  // Name and version of this sketch

/****
 *
 *    EEPROM data structure definition
 *
 ****/
struct settings_t {                            // Structure of data stored in EEPROM
  int id;                                      // ID tag to know whether data (probably) belongs to this sketch
  int rtcCorrection;                           // Emprically determined correction factor for the Arduino rtc (tenths of a sec/day)
  int peakScale;                               // Empirically determined scaling factor for peak induced voltage
  long uspb;                                   // Emprically determined duration of a beat in μs
};

/****
 *
 *    Global variables
 *
 ****/
Clock c;                                       // Instantiate a clock object on digital pins 10 and 11
IRrecv irRecv(IR_PIN);                         // Instantiate an IRrecv object for our receiver
RgbLed led(LED_R_PIN, LED_G_PIN, LED_B_PIN);   // Instantiate a simple RGB LED
decode_results results;                        // The decode_results variable into which we place the results
static byte stepix = 3;                        // The adjustment stepsize index (3 = 1 minute)
const long stepSize[6] = {                     // The adjustment stepsize in tenths of a second
  1, 10, 100, 600, 6000, 36000                 // 0.1s, 1s, 10s, 1m, 10m, 1h
};
static boolean adjustable = false;             // Whether the adjustments are enabled (by Power button)
static long speedAdj = 0;                      // The amount of the pending speed adjustment in tenths of a second; 0 if none
settings_t settings;

/****
 *
 * Functions to read and write EEPROM
 *
 ****/
void readEEPROM() {
  eeprom_read_block((void*)&settings, (void*)0, sizeof(settings));
}
void writeEEPROM() {
  settings.id = SETTINGS_TAG;                    // Mark the settings structure as ours
  eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));
}

/****
 *
 *  Initialization routine: Called once when power is applied or when the Arduino is reset
 *
 ****/

void setup() {
  irRecv.enableIRIn();                         // Start the IR receiver
  pinMode(CS_PIN, INPUT);                      // Set the cold start pin to INPUT.
  digitalWrite(CS_PIN, HIGH);                  // Turn on pullup resistor. Installing cold start jumper takes this to Gnd
  Serial.begin(9600);                          // Initialize serial communications
  Serial.println(VERSION_STRING);              // Announce ourselves
  if ((digitalRead(CS_PIN) == HIGH)) {         // If it's not a cold start
    Serial.println("Loading clock correction.");
    readEEPROM();
    if (settings.id == SETTINGS_TAG) {         //   If it looks like it our info
      Serial.print("Arduino real-time clock correction set to ");
      Serial.print(settings.rtcCorrection/10.0, 1);
      Serial.println(" seconds per day");      //      say we did it.
    } else {                                   //   else (invalid data in EEPROM)
      Serial.println("Arduino clock correction invalid. Cold starting.");
    }
  } else {                                     // else (cold start)
    Serial.println("Cold start.");
  }
}

/*
 *
 *   The onButton function.
 *
 *   This function is called repeatedly in the main loop. If a key has been pressed on the SparkFun IR remote, 
 *   it calls whichever function from the passed array of functions corresponds to the key that was pressed. 
 *   If no key has been pressed, onButton simply returns without calling a function. It handles the error case 
 *   of receiving an IR code that the SparkFun IR remote doesn't generate by logging to Serial and then 
 *   ignoring the unrecognized code. 
 *
 *   Holding down a button generates one instance of the button code followed by the code for "repeat" over 
 *   and over. We handle this by behaving as though the button had been pressed repeatedly once the button 
 *   has been held down for a bit. The delay is to avoid unintended repeats.
 *
 */

void onButton(void (*fButton[])()) {
  static int keyIx = 0;                        // Which button on the IR remote got pushed last
  static int repeat = 0;                       // How many times the "repeat" code was received in a row
  byte cix = led.getColor();                   // The color the led is currently set to
  
  if (irRecv.decode(&results)) {               // If an IR code was received
    switch (results.value) {                   //   Do different cases depending on the code received
      case SFIRR_POWER:                        //     Except for the repeat code and the default case,
        keyIx = 0;                             //     the cases all set the value of keyIx to the array 
        repeat = 0;                            //     index of the function array corresponding to the 
        break;                                 //     button that was pushed and set repeat to 0 to turn 
      case SFIRR_A:                            //     off repeat processing
        keyIx = 1;
        repeat = 0;
        break;
      case SFIRR_B:
        keyIx = 2;
        repeat = 0;
        break;
      case SFIRR_C:
        keyIx = 3;
        repeat = 0;
        break;
      case SFIRR_UP:
        keyIx = 4;
        repeat = 0;
        break;
      case SFIRR_DOWN:
        keyIx = 5;
        repeat = 0;
        break;
      case SFIRR_LEFT:
        keyIx = 6;
        repeat = 0;
        break;
      case SFIRR_RIGHT:
        keyIx = 7;
        repeat = 0;
        break;
      case SFIRR_SELECT:
        keyIx = 8;
        repeat = 0;
        break;
      case REPEAT:                             //   Case "repeat code"
        if (++repeat < SFIRR_REPEAT_PAUSE) {   //     Force user to hold down the same key for more than a
          irRecv.resume();                     //     fraction of a second by ignoring the first few repeat
          return;                              //     codes
        }
        break;
      default:                                 // Case unrecognized code; dump diag info
        Serial.println("IR Code not recognized.");
        Serial.print("decode_type: 0x");
        Serial.print(results.decode_type, HEX);
        Serial.print(", value: 0x");
        Serial.print(results.value, HEX);
        Serial.print(", bits: ");
        Serial.println(results.bits);
        irRecv.resume();
        return;
    }
    led.setColor(RGB_LED_BLACK);               //   White flash led then return it to its former color to let
    delay(40);                                 //   user know we got a function request
    led.setColor(RGB_LED_WHITE);
    delay(60);
    led.setColor(cix);
    fButton[keyIx]();                          //   Do whatever it is we're s'posed to do
    irRecv.resume();                           //   Watch for another button press
  }
}

/****
 *
 *   Button functions -- one for each button on the remote. Called (by onButton) when the user presses the
 *   corresponding button on the remote.
 *
 ****/

//  Power: Flip adjustable; If going off, write speed adjustment, if any, to EEPROM
void fPower() {
  if (adjustable) {
    if (speedAdj != 0) {
      Serial.print("Changing realtime clock correction by ");
      Serial.print(speedAdj/10.0, 1);
      Serial.print(" s/day from ");
      Serial.print(settings.rtcCorrection/10.0, 1);
      Serial.print(" s/day to ");
      Serial.print((settings.rtcCorrection += speedAdj)/10.0, 1);
      Serial.println(" s/day.");
      writeEEPROM();
      speedAdj = 0;
    }
    Serial.println("Adjustments: off.");
    led.setColor(RGB_LED_BLACK);
  } else {
    Serial.println("Adjustments: on.");
    Serial.print("Realtime clock correction is ");
    Serial.print(settings.rtcCorrection/10.0, 1);
    Serial.println(" s/day.");
    led.setColor(stepix);
  }
  adjustable = !adjustable;
}

//  "A" button: Decrement adjustment step size
void fA() {
  if (adjustable) {                            // If it's adjustable
    if (--stepix > 5) {                        //  Decrement step index (mod 5)
      stepix = 5;
    }
    led.setColor(stepix);                      //  Set led color to match
    Serial.print("Decremented adjustment step size to ");
    Serial.print(stepSize[stepix]/10.0, 1);
    Serial.println(" seconds.");
  }
}

//  "B" button: Cancel adjustments
void fB() {
  if (adjustable) {                            // If it's adjustable
    Serial.println("Canceling adjustments.");
    c.cancelDrive();
    speedAdj = 0;
  }
}

//  "C" button: Increment adjustment step size
void fC() {
  if (adjustable) {                            // If it's adjustable
    if (++stepix > 5) {                       //  Increment step size (mod 5)
      stepix = 0;
    }
    led.setColor(stepix);                      //  Set led color to match
    Serial.print("Incremented adjustment step size to ");
    Serial.print(stepSize[stepix]/10.0, 1);
    Serial.println(" seconds.");
  }
}

// "Up" button: Increase pending realtime clock correction by adjustment step size seconds per day
void fUp() {
  if (adjustable) {
    Serial.print("Increasing pending realtime clock correction by ");
    Serial.print(stepSize[stepix]/10.0, 1);
    Serial.print(" s/day to ");
    Serial.print((speedAdj += stepSize[stepix])/10.0, 1);
    Serial.println(" s/day.");
  }
}

// "Down" button: Decrease pending realtime clock correction by adjustment step size seconds per day
void fDown() {
  if (adjustable) {
    Serial.print("Decreasing pending realtime clock correction by ");
    Serial.print(stepSize[stepix]/10.0, 1);
    Serial.print(" s/day to ");
    Serial.print((speedAdj -= stepSize[stepix])/10.0, 1);
    Serial.println(" s/day.");
  }
}

// "Left" button: Pause clock for the adjustment step size seconds
void fLeft() {
  if (adjustable) {
    Serial.print("Pausing clock for ");
    Serial.print(stepSize[stepix]/10);
    Serial.println(" seconds.");
    if (stepSize[stepix] <= 21474) {                    // Min value for a long is -2,147,483,648. Don't want to underflow
      c.driveMicros(-stepSize[stepix]*100000);
    } else {
      c.driveSec(-stepSize[stepix]/10);
    }
  }
}

// "Right" button: Fast-forward clock by adjustment step size seconds
void fRight() {
  if (adjustable) {
    Serial.print("Advancing clock by ");
    Serial.print(stepSize[stepix]/10);
    Serial.println(" seconds.");
    if (stepSize[stepix] <= 21474) {                    // Max value for a long is 2,147,483,647. Don't want to overflow
      c.driveMicros(stepSize[stepix]*100000);
    } else {
      c.driveSec(stepSize[stepix]/10);
    }
  }
}

//  Select: No function
void fSelect () {
}

/****
 *
 *   Main loop: Called over and over until power goes off or the Arduino is reset.
 *
 ****/

void loop() {
  static void (*fPointer[])() = {fPower, fA, fB, fC, fUp, fDown, fLeft, fRight, fSelect}; // Array of button function pointers
  static unsigned long lastTime = micros();            // Time on rtc the last time we ticked the clock.
  static unsigned long topTime = 0;                    // Time on rtc at top of loop
  static unsigned long deltaT = 0;                     // Microseconds since last ticked clock

  topTime = micros();                                  // What timeis it now?
  deltaT = topTime - lastTime;                         // How much (uncorrected) time has passed?
  if (deltaT >= 500000) {                              // If more than a half an (uncorrected) second has passed
    deltaT += ((settings.rtcCorrection * deltaT) + 432000) / 864000;
                                                       //   Make deltaT into corrected time
    c.driveMicros(deltaT);                             //   Drive clock by the amount of (corrected) time that has passed
    lastTime = topTime;                                //   Remember what rtc said when we did all this
  }
  onButton(fPointer);                                  // Deal with any IR remote buttons that may have been pushed
}
