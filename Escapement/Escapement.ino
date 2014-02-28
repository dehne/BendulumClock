/****
 *
 *   Escapement v1.26
 *
 *   Copyright 2014 by D. L. Ehnebuske 
 *   License terms: Creative Commons Attribution-ShareAlike 3.0 United States (CC BY-SA 3.0 US) 
 *                  See http://creativecommons.org/licenses/by-sa/3.0/us/ for specifics. 
 *
 *   This sketch implements an electronic escapement for pendulum or bendulum clocks. (A "bendulum" is a long, slender,
 *   springy piece of metal, held vertically, whose lower end is fixed and whose upper end has a weight that is free to
 *   move back and forth -- a sort of upside-down pendulum.) The pendulum or bendulum terminates in a magnet that moves
 *   across a many-turn coil of fine wire positioned at the center of the swing of the pendulum or bendulum. The motion of
 *   the magnet is detected as it passes over the coil by the current the magnet induces. Just after the magnet passes, the
 *   escapement delivers a pulse of current through the coil, inducing a magnetic field in it that gives the magnet a small
 *   push, keeping the pendulum or bendulum going.
 *
 *   Detection of the magnet's passage is also used as a timing signal to drive a Lavet-motor clock (one of those
 *   one-battery quartz analog clock movements, repurposed so its timing mechanism isn't used). These Lavet motor clock
 *   movements advance by one second for each step of the motor. 
 *
 *   Traditionally, pendulum clocks are designed with pendulums whose period is related in a simple way to the length of 
 *   a second. Doing so makes the gear train that converts pendulum swings into clock hand movement relatively 
 *   straightforward to design. For example, the really tall "grandfather" clocks have pendulums with a period of two 
 *   seconds. This means the pendulum passes the center of its swing once each second (a speed known in the trade as 60 
 *   beats per minute or bpm). Since a 60 bpm pendulum causes the escapement to advance the escape wheel by one tooth each 
 *   second, the the gear train to which it is attached is quite straightforward to design.
 *
 *   With an electronic escapement the "digital gear train" can be much more flexible, making it possible to utilize just
 *   about whatever pendulum or bendulum we happen to have so long as we can accurately measure how long a beat takes. So, 
 *   instead of holding the gear train constant and adjusting the period of the pendulum/bendulum to make the clock run at 
 *   the correct speed, we let the period be what it is and adjust the "gear train" to advance the clock so that, on 
 *   average, it advances at the correct rate, even though ratio of beats to seconds is not a simple one.
 *
 *   But how do you measure the period of your pendulum or bendulum accurately? One way you could do it is to use an 
 *   accurate clock to measure how long it takes for, say, 100 beats and then work out the math. A more practical way is to 
 *   use the Arduino's built-in clock to do the measuring. This, it turns out, is not as straightforward at it might be
 *   because the clock in an Arduino, while stable enough, isn't very accurate. It's close enough to get in the ballpark, but 
 *   you'll almost certainly need to fine tune it. That takes a while, but it's certainly no more dificult than regulating a 
 *   traditional pendulum clock. As the clock is regulated, the escapement sketch stores the successive approximations in 
 *   EEPROM so that is won't be lost in the event of a power failure. (See below, for more on calibration.)
 *
 *   The escapement relies on a Bendulum object (called b) to interface with the physical bendulum or pendulum and 
 *   to derive the timing information needed to drive the clock. The main operational method of b is b.beat(). Each 
 *   invocation of b.beat() drives the physical bendulum or pendulum through one beat and returns the number of microseconds 
 *   the beat took to complete. In this sketch b.beat() is invoked once in each pass through the sketch's loop() function. 
 *   Adding up the values returned by successive invocations of b.beat() marks the passage of time. The sketch does this by 
 *   passing the duration of each beat to the sketch's other main object, the Clock object, c, which interfaces to the Lavet 
 *   motor clock mechanism used to display the time of day, as described above. The main operational method of c is 
 *   c.driveMicros(). Like b.beat(), c.driveMicros() is invoked during each pass theough the sketch's loop() function. The 
 *   c.driveMicros() method adds the number of microseconds it is passed to the number it had been passed previously and 
 *   "works off" the accumulated time by moving the clock mechanism forward by one second when it has accumulated at least
 *   a half second. 
 *
 *   As the loop() function is invoked repeatedly, the process continues -- b.beat() returns the number of microseconds 
 *   that passed since the last trip through loop() and c.driveMicros() adds this to its running total and then subtracts 
 *   one second from whenever it moves the clock forward.
 *
 *   Additional detail on how the Bendulum and Clock objects work may be found in their respective libraries.
 *
 *   Regulation and adjustment of the escapement is controlled by using one of the little red SparkFun IR remotes. The 
 *   remote has the following buttons and purposes:
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
 *           Select        Start calibration mode
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
 *   When adjustment is turned off the LED is dark except for a flash marking each beat. Press Power to turn it back on. 
 *   The LED flashes white each time a command is received from the remote control.
 *
 *   To regulate the escapement, do something like the following. Set the hands of the clock to display the correct 
 *   time of day, start the bendulum or pendulum by giving it a push, and apply power to the Arduino. The clock will 
 *   start running. If this is the first time you've used the Arduino to run the clock, the EEPROM won't contain 
 *   calibration information, so the sketch will calibrate the period of the bendulum or pendulum as best it can with 
 *   the Arduino real-time clock. Calibration takes about 2200 complete bendulum/pendulum cycles. (A cycle is two "beats,"
 *   i.e., a tick and a tock.) While it's calibrating, the LED will flash yellow with each beat. Once calibration is 
 *   complete, the LED flashes green with each beat. Let the calibrarion complete and then let the clock run normally for 
 *   enough time to accumulate a few minutes of error when compared with an accurate clock. Figure out how many minutes 
 *   and seconds per day the error represents, and use the remote to adjust the running speed. 
 *
 *   For example, if the initial error works out to 2 minutes, 48 seconds fast per day, you could press the Power button 
 *   to turn on adjustments. The press the A and/or C button until the LED turns cyan indicating a 1 minute step size. 
 *   Then press the Down button twice to indicate you'd like to slow the clock by two minutes. Then press the A button 
 *   to select the next smaller step size. The LED turns green to indicate a step size of 10 seconds. Press the Down 
 *   button four times to indicate you'd like to slow the clock by an additional 40 seconds per day. Then press the A 
 *   button to select the next smaller step size -- 1 second. The LED turns green. Press the Down buton eight times to 
 *   indicate you'd like to slow the clock by and additional eight seconds per day, for a total slow-down of 2 minutes 
 *   48 seconds. Press the Power button to make the change to the speed of the clock and to turn off adjustments. The
 *   LED goes out to indicate the ability to make adjustments is turned off.
 *
 *   If you like, you can reduce the number of button pushes by doing both aditions and subtractions to get to the right 
 *   adjustment. The -2:48 adjustment could be done as - 3 min followed by +10 sec followed by -2 sec, for instance.
 *
 *   If you want to change the time displayed on the clock, turn on adjustments with the Power button (or leave it on 
 *   after setting the pending speed adjustment) then use the remote's Left, and Right butons togther with the A and 
 *   C step-size buttons to adjust the clock hands to the correct time. When everything is all set, turn off adjustments 
 *   by pressing the Power button. Let the clock run to see how it does. If further adjustment is needed, just repeat the 
 *   process. After a few passes, you should have the clock running as accurately as possible. Depending on the 
 *   construction of the bendulum or pendulum "running accurately" can vary quite a bit. If the bendulum or pendulum is 
 *   well made and isolated from air currents, the clock should be as accurate as a traditional household pendulum clock 
 *   -- to a few minutes a week. The main residual errors are most likely due to thermal expansion and, for a pendulum, 
 *   changes in the Young's modulus of the material it is made of as the ambient temperature changes. Correcting for them 
 *   we'll leave for version 1.3.
 *
 *   Normally when the Arduino is powered up, the sketch loads three pieces of calibration information from EEPROM: The 
 *   Arduino real-time clock correction (used, but not changed in this sketch), the beat duration (set by auto-calibration 
 *   and changed by adjusting the clock speed) and a beat detection sensitivity factor called peakScale (set by
 *   auto-calibration). This means that once the clock has been properly regulated, operation can pick up where it left 
 *   off if the power fails or the sketch is restarted for some other reason. The sketch "hot starts" by setting b's 
 *   runMode to RUNNING and the setting the Arduino clock correction, beat duration and peakScaling to the values stored 
 *   in EEPROM.
 * 
 *   Hot starting can save a lot of calibration work, but if you should ever want to "cold start" the whole process, simply 
 *   install the cold start jumper on the bendulum shield. This causes the sketch to ignore the stored information when it 
 *   starts. With no stored information to go on, it goes through the same auto-calibration process it did the first time 
 *   the sketch ran. Once the calibration run is done, remove the jumper to return the sketch's operation to normal.
 *
 *   If your clock has been running a while but you make a change to the benuluum or pendulum so that its beat duration
 *   changes, you'll need to recalibrate to determine the new rate. This is most easily done by hot starting the clock and 
 *   then pressing the Select button. This will start a clalibration run to calculate the beat duration and peakScaling. 
 *   Once the calibration run is complete, the calibration values will be stored in EEPROM. You'll then need to fine tune
 *   the running speed with using the remote.
 *
 ****/

/****
 *
 *    Library header includes
 *
 ****/

#include <avr/eeprom.h>                        // EEPROM read write library
#include <Bendulum.h>                          // Bendulum library
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
#define VERSION_STRING "Escapement v1.26."     // Name and version of this sketch
#define NORMAL_BEAT_COLOR RGB_LED_GREEN        // The normal beat flash color is green
#define CAL_BEAT_COLOR RGB_LED_YELLOW          // The calibration flash color is yellow

/****
 *
 *    EEPROM data structure definition
 *
 ****/
struct settings_t {                            // Structure of data stored in EEPROM
  int id;                                      // ID tag to know whether data (probably) belongs to this sketch
  int rtcCorrection;                           // Emprically determined correction factor for the Arduino real-time clock
  int peakScale;                               // Empirically determined scaling factor for peak induced voltage
  long uspb;                                   // Emprically determined duration of a beat in μs
};

/****
 *
 *    Global variables
 *
 ****/
Bendulum b;                                    // Instantiate the bendulum object on the default hardware pins
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
static byte ledBeatColor = RGB_LED_YELLOW;     // Color of the LED flash when not hot starting
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
  settings.rtcCorrection = b.getBias();          // Fill in the current rtc correction
  settings.peakScale = b.getPeakScale();         // Fill in the peak scaling factor
  settings.uspb = b.getBeatDuration();           // Fill in the current beat duration, and write EEPROM
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
      b.setBias(settings.rtcCorrection);       //      Set the rtc correction
      Serial.print("Arduino real-time clock correction set to ");
      Serial.print(b.getBias()/10.0, 1);       //      and say we did it.
      Serial.println(" seconds per day");
      if (settings.uspb > 0) {                 //      if we also have a stored beat duration
        b.setRunMode(RUNNING);                 //        Go right to RUNNING mode
        b.setBeatDuration(settings.uspb);      //        Using the stored beat duration
        b.setPeakScale(settings.peakScale);    //        and peak scaling value
        ledBeatColor = NORMAL_BEAT_COLOR;      //        Set beat flash color to normal
        Serial.print("Hot starting using ");   //        Say that's what we're doing
        Serial.print(b.getBeatDuration()/1000000.0, 5);
        Serial.print(" seconds per beat and a peak scaling of ");
        Serial.println(b.getPeakScale());
      } else {                                 //      else (Arduino clock correction but no uspb)
        Serial.println("Warm starting with automatic calibration.");
      }
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
  } else if (cix == RGB_LED_BLACK) {           // Else (no keypress) if the light's out
    led.setColor(ledBeatColor);               //   Flash LED whatever the current flash color is
    delay(40);
    led.setColor(cix);
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
    Serial.print("Changing beat duration by: ");
    Serial.print(speedAdj/10.0, 1);
    Serial.print(" s/day from ");
    Serial.print(b.getBeatDuration());
    Serial.print(" us to ");
    Serial.print(b.incrBeatDuration(speedAdj));
    Serial.println(" us.");
    writeEEPROM();
    speedAdj = 0;
    }
  Serial.println("Adjustments: off.");
  led.setColor(RGB_LED_BLACK);
  } else {
    Serial.println("Adjustments: on.");
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

// "Up" button: Increase beat duration by adjustment step size seconds per day
void fUp() {
  if (adjustable) {
    Serial.print("Increasing beat duration adjustment by: ");
    Serial.print(stepSize[stepix]/10.0, 1);
    Serial.print(" s/day to ");
    Serial.print((speedAdj += stepSize[stepix])/10.0, 1);
    Serial.println(" s/day.");
  }
}

// "Down" button: Decrease beat duration by adjustment step size seconds per day
void fDown() {
  if (adjustable) {
    Serial.print("Decreasing beat duration adjustment by: ");
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

//  Select: Start a calibration run
void fSelect () {
  if (adjustable) {
    b.setRunMode(SCALING);                 //   Do a scaling followed by a calibration
    ledBeatColor = CAL_BEAT_COLOR;         //   Set flash color to match mode
    Serial.println("Starting calibration run.");
    Serial.println("Adjustments: off.");
    led.setColor(RGB_LED_BLACK);
    adjustable = false;
  }
}

/****
 *
 *   Main loop: Called over and over until power goes off or the Arduino is reset.
 *
 ****/

void loop() {
  static void (*fPointer[])() = {fPower, fA, fB, fC, fUp, fDown, fLeft, fRight, fSelect}; // Array of button function pointers

  c.driveMicros(b.beat());                             // Let bendulum do one beat; move the clock forward however long it took
  onButton(fPointer);                                  // Deal with any IR remote buttons that may have been pushed
  if (b.isTick()) {                                    // If bendulum passed in the "tick" direction (as oppoed to "tock")
    switch (b.getRunMode()) {                          //   Do things based on the current "run mode"
      case SETTLING:                                   //   When settling in
        Serial.print("Settling: ");                    //     Say that we're settling in and how far along we've gotten
        Serial.print(b.getCycleCounter());
        Serial.print(", delta: ");                     //     Display the ratio of tick duration to tock duration
        Serial.print(b.getDelta(), 4);
        Serial.print(", Current BPM: ");               //       and the measured beats per minute
        Serial.print(b.getCurBpm(), 4);
        break;
      case SCALING:                                    //   When scaling for peak value
        Serial.print("Scaling: ");                     //    Say that we're scaling and how far along we've gotten
        Serial.print(b.getCycleCounter());
        Serial.print(", delta: ");                     //    Display the ratio of tick duration to tock duration
        Serial.print(b.getDelta(), 4);
        Serial.print(", Current BPM: ");               //      the measured beats per minute
        Serial.print(b.getCurBpm(), 4);
        Serial.print(", peak scaling: ");              //      and the current peak scaling
        Serial.print(b.getPeakScale());
        break;
      case CALIBRATING:                                //   When calibrating
        Serial.print("Calibrating: ");                 //     Say we're calibrating, how much smoothing We've been
        Serial.print(b.getCycleCounter());             //       able to do so far, how symmetrical the "ticks" and
        Serial.print(", delta: ");                     //       "tocks" are currently, and how many beats per minute
        Serial.print(b.getDelta(), 4);                 //       on average we're seeing so far
        Serial.print(", Average BPM: ");
        Serial.print(b.getAvgBpm(), 4);
        break;
      case CALFINISH:                                  //   When finished calibrating
        Serial.print("Finished calibrating; updating EEPROM.");
        Serial.print("  Arduino real-time clock correction: ");
        Serial.print(b.getBias()/10.0, 1);
        Serial.print(" seconds per day, beat duration: ");
        Serial.print(b.getBeatDuration()/1000000.0, 5);
        Serial.print(" seconds and a peak scaling of ");
        Serial.print(b.getPeakScale());
        writeEEPROM();                                 //     Say so and save the settings in EEPROM
        ledBeatColor = NORMAL_BEAT_COLOR;              //     Change beat flash color to normal -- green
        break;
      case RUNNING:                                    //   When running
        Serial.print("Running. Calibrated: ");         //     Say that we're running along normally and what the
        Serial.print(b.getAvgBpm(), 4);                //       calibrated beats per minute is and what the currently
        Serial.print("(bpm), Current: ");              //       measured bpm is
        Serial.print(b.getCurBpm(), 4);
        Serial.print("(bpm)");
        break;
    }
    Serial.println(".");
  }
}
