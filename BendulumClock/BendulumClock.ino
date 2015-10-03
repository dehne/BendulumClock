/****
 *
 *   BendulumClock v1.15
 *
 *   Copyright 2014-2015 by D. L. Ehnebuske 
 *   License terms: Creative Commons Attribution-ShareAlike 3.0 United States (CC BY-SA 3.0 US) 
 *                  See http://creativecommons.org/licenses/by-sa/3.0/us/ for specifics. 
 *
 *   This sketch implements an electromechanical pendulum or bendulum clock. (A "bendulum" is a long, slender, springy 
 *   piece of metal, held vertically, whose lower end is fixed and whose upper end has a weight that is free to move back 
 *   and forth -- a sort of upside-down pendulum.) The pendulum or bendulum terminates in a magnet that moves across a 
 *   many-turn coil of fine wire positioned at the center of the swing of the pendulum or bendulum. The motion of the 
 *   magnet is detected as it passes over the coil by the current the magnet induces. Just after the magnet passes, the
 *   software delivers a pulse of current through the coil, inducing a magnetic field in it that gives the magnet a small
 *   push, keeping the pendulum or bendulum going.
 *
 *   Detection of the magnet's passage is also used as a timing signal to drive a Lavet-motor clock (one of those
 *   one-battery quartz analog clock movements, repurposed so its timing mechanism isn't used). These Lavet motor clock
 *   movements advance by one second for each step of the motor.
 *
 *   Traditionally, pendulum clocks are designed with pendulums whose period is related in a simple way to the length of 
 *   a second. Doing it this way makes the gear train that converts pendulum swings into clock-hand movement relatively 
 *   straightforward to design. For example, the really tall "grandfather" clocks have pendulums with a period of two 
 *   seconds. This means the pendulum passes the center of its swing once each second (a speed known in the trade as 60 
 *   beats per minute or bpm). Since a 60 bpm pendulum causes the escapement to advance the escape wheel by one tooth each 
 *   second, the the gear train to which it is attached is quite straightforward to design.
 *
 *   With an electronic escapement the "digital gear train" can be much more flexible, making it possible to utilize just
 *   about whatever pendulum or bendulum we happen to have so long as we can accurately measure how long a beat takes. So, 
 *   instead of holding the gear train constant and adjusting the period of the pendulum/bendulum to make the clock run at 
 *   the correct speed, we let the period be what it is and adjust the "gear train" to advance the clock so that, it 
 *   advances at the correct rate, even though ratio of beats to seconds is not a simple one.
 *
 *   But how do you measure the period of your pendulum or bendulum accurately? One way you could do it is to use an 
 *   accurate clock to measure how long it takes for, say, 1000 beats and then work out the math. A more obvious way is to 
 *   use the Arduino's built-in clock to do the measuring. This, it turns out, is not as straightforward at it might be
 *   because the clock in an Arduino, while pretty stable, isn't very accurate. Fortunately, we provide a simple way to 
 *   calibrate your Arduino's real-time clock and then, using the calibrated real-time clock, an automatic method for 
 *   calibrating the period of the bendulum or pendulum. (See calibration details, below.)
 *
 *   The BendulumClock relies on an Escapement object (called e) to interface with the physical bendulum or pendulum and 
 *   to derive the timing information needed to drive the clock. The main operational method of e is e.beat(). Each 
 *   invocation of e.beat() drives the physical bendulum or pendulum through one beat and returns the number of microseconds 
 *   the beat took to complete. In this sketch e.beat() is invoked once in each pass through the sketch's loop() function. 
 *   Adding up the values returned by successive invocations of e.beat() marks the passage of time. The sketch does this by 
 *   passing the duration of each beat to the sketch's other main object, the Clock object, c, which interfaces to the Lavet 
 *   motor clock mechanism used to display the time of day, as described above. The main operational method of c is 
 *   c.driveMicros(). Like e.beat(), c.driveMicros() is invoked during each pass theough the sketch's loop() function. The 
 *   c.driveMicros() method adds the number of microseconds it is passed to the number it had been passed previously and 
 *   "works off" the accumulated time by moving the clock mechanism forward by one second when it has accumulated at least
 *   a half second.
 *
 *   As the loop() function is invoked repeatedly, the process continues -- e.beat() returns the number of microseconds 
 *   that passed since it was last invoked and c.driveMicros() adds this to its running total and then subtracts one second 
 *   from whenever it moves the clock forward.
 *
 *   Additional detail on how the Bendulum and Clock objects work may be found in their respective libraries.
 *
 *   Regulation and adjustment of the clock is controlled using one of the little red SparkFun IR remotes. The remote has 
 *   the following buttons and purposes:
 *
 *           Button        Function
 *           ======        ========================================================================================
 *           Power         Turn on/off clock adjustments. Make and store any pending speed adjustment in EEPROM
 *           A             Decrement adjustment step size
 *           B             Cancel any pending speed adjustment or on-going time-of day adjustment
 *           C             Increment adjustment step size
 *           Up            Add step interval to pending speed adjustment
 *           Down          Sobtract step interval from pending speed adjustment
 *           Left          Pause time display for the amount of the adjustment step
 *           Right         Fast-forward the time display by the amount of the adjustment step
 *           Select        Start calibration mode (details below)
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
 *   To regulate the clock, do something like the following. Set the hands of the clock to display approximately the 
 *   correct time of day, start the bendulum or pendulum by giving it a push, and apply power to the Arduino. The clock 
 *   will start running. 
 *
 *   If this is the first time you've used this Arduino to run a clock, the EEPROM won't contain calibration information, 
 *   so the sketch will do a "cold start." It begins by characterizing how the magnet on the bendulum or pendulum interacts 
 *   with the coil. During this process the LED will blink yellow. Once magnet-coil interaction characterization is complete, 
 *   the clock will enter real-time clock (RTC) calibration mode, indicated by the LED flashing magenta. At this point you 
 *   should adjust the time-of-day display to show the same time as the accurate clock you'll use as the standard. 
 *
 *   To adjust the time-of-day display, turn on adjustments with the Power button. Then use the Left and Right buttons
 *   together with the adjustment step size buttons.
 *   
 *   For example, if the clock is 2 minutes, 48 seconds ahead of the correct time, press the Power button to turn on 
 *   adjustments. Then press the A and/or C button until the LED turns cyan indicating a 1 minute step size. Then press 
 *   the Left button twice to indicate you'd like to stop the clock display from advancing for two minutes. Then press 
 *   the A button to select the next smaller step size. The LED turns green to indicate a step size of 10 seconds. Press 
 *   the Left button four times to indicate you'd like to stop the clock display for an additional 40 seconds. Then press 
 *   the A button to select the next smaller step size -- 1 second. The LED turns yellow. Press the Left buton eight times 
 *   to indicate you'd like to stop the clock display for an additional eight seconds, for a total pause of 2 minutes 48 
 *   seconds. Press the Power button to turn off adjustments. The LED goes back to flashing to indicate the ability to 
 *   make adjustments is turned off. When a total pause of 2 minutes and 48 seconds has passed, the clock display will 
 *   start advancing once again.
 *
 *   If you like, you can reduce the number of button pushes by doing both additions and subtractions to get to the right 
 *   adjustment. The -2:48 adjustment could be done as -3 min followed by +10 sec followed by -2 sec, for instance.
 *
 *   Let clock run, driven by the RTC, for an extended period of time -- a day or two. Until it has accumulated an 
 *   appreciable error. Then calculate the clock error in seconds per day. Using the IR remote, turn on adjustments. Then 
 *   use the speed adjustment buttons (the Up and Down buttons) together with the step-size buttone to set the pending 
 *   speed adjustment to the number you calculated. Then turn adjustments off with the Power button. This causes the 
 *   speed adjustment to be stored in EEPROM.
 *
 *   Next, change the time displayed on the clock so that it shows the correct time. Let the clock run to see how it does. 
 *   If further adjustment is needed, just repeat the process. After a few passes, you should have the Arduino's RTC clock 
 *   running as accurately as possible. Because the RTC for a given Arduino is pretty stable, you should only need to 
 *   calibrate it once.
 *
 *   Once the RTC is calibrated, it's time to exit RTC calibration and let the sketch characterize the operaqtion of the 
 *   pendulum or bendulum. To do this, turn on adjustments with the Power button and then press the B button to switch to 
 *   normal operation.
 *
 *   During normal operation, the clock will automatically calibrate its operation to conform with the beat rate of the
 *   pendulum or bendulum it is using. Since the beat rate will vary slightly as the temperature changes, it will
 *   calibrate its operation at each new temperature it encounters. When the LED is flashing green, the clock is being 
 *   driven by the pendulum or the bendulum. When it is flashing blue, the clock is using the RTC to determine the beat 
 *   rate at the current temperature. As the clock runs and the temperature varies, the rate at a wide range of 
 *   temperatures will be characterized and the flashing will be all green. At that point the clock is fully calibrated 
 *   and running accurately.
 *
 *   Depending on the construction of the bendulum or pendulum "running accurately" can vary quite a bit. If the bendulum 
 *   or pendulum is well made and isolated from air currents, the clock should be at least as accurate as a traditional 
 *   household pendulum clock -- to within a minutes a week.
 *
 *   You can tweak the results of the automatic calibration manually, if it's a little off. To do so, turn on adjustments
 *   when the clock is running normally (the LED is flashing green or blue) and use the Up and Down buttons together with 
 *   the step-size buttons to enter a correction factor in seconds per day. When you save it by turning adjustments off,
 *   the correction factor will be saved and applied to the clock speed. Making a clock speed adjustment in this way only
 *   changes the clock speed, not the RTC calibration.
 *
 *   Normally when the Arduino is powered up, the sketch loads its calibration information from EEPROM. This means that 
 *   once the clock has been properly regulated, operation can pick up where it left off if the power fails or the sketch 
 *   is restarted for some other reason.
 * 
 *   Hot starting can save a lot of calibration work, but if you should ever want to "cold start" the whole process, simply 
 *   install the cold start jumper on the bendulum shield. This causes the sketch to ignore all the stored information when 
 *   it starts. With no stored information to go on, it goes through the same auto-calibration process it did the first time 
 *   the sketch ran. Once the calibration run is done, remove the jumper so that the sketch won't cold start every time it's
 *   started.
 *
 *   If your clock has been running a while but you make a change to the benuluum or pendulum so that its beat duration
 *   changes, you'll need to recalibrate to determine the new rate. This is most easily done by hot starting the clock and 
 *   then pressing the Select button. This will put the clock into RTC calibration mode and turn adjustments off. Next turn
 *   adjustments on again and press Select a second time. This puts the clock into "warm start" mode. It will begin by 
 *   determining  how the magnet and coil interact (yellow flashing) and then move into normal operation, (blue and green 
 *   flashing) as it does the calibration over at each temperature it encounters. Also see http://xkcd.com/1421/ .
 *
 ****/

/****
 *
 *    Library header includes
 *
 ****/
#include <Wire.h>                                // Needed to fool the IDE into including this; it's needed by Escapement
#include <avr/eeprom.h>                          // Ditto
#include <Escapement.h>                          // Escapement library
#include <Clock.h>                               // Lavet motor clock library
#include <RgbLed.h>                              // Simple RGB LED library
#include <LRremote.h>                            // IR remote control library (http://github.com/shirriff/Arduino-IRremote)

/**** 
 *
 *    Hardware pin assignment constants -- what's hooked up to which Arduino pins
 *
 ****/
#define CS_PIN       (2)                         // The jumper that tells whether we should do a cold start is on pin 2
#define IR_PIN       (3)                         // The IR receiver is attached to pin 3
#define LED_R_PIN    (9)                         // The pin to which the red part of the LED is hooked
#define LED_G_PIN    (6)                         // The pin to which the green part of the LED is hooked
#define LED_B_PIN    (5)                         // The pin to which the blue part of the LED is hooked

/****
 *
 * Constants for the SparkFun Little Red Remote Control
 *
 ****/
#define SFIRR_POWER  0x10EFD827                  // "Power" button IR code
#define SFIRR_A      0x10EFF807                  // "A" button IR code
#define SFIRR_B      0x10EF7887                  // "B" button IR code
#define SFIRR_C      0x10EF58A7                  // "C" button IR code
#define SFIRR_UP     0x10EFA05F                  // "Up" button IR code
#define SFIRR_DOWN   0x10EF00FF                  // "Down" button IR code
#define SFIRR_LEFT   0x10EF10EF                  // "Left" button IR code
#define SFIRR_RIGHT  0x10EF807F                  // "Right" button IR code
#define SFIRR_SELECT 0x10EF20DF                  // "Select" button IR code

/****
 *
 * Other constants
 *
 ****/
#define VERSION_STRING F("BendulumClock v1.15.") // Name and version of this sketch
#define COLD_BEAT_COLOR   RGB_LED_RED            // The cold start flash color is red
#define WARM_BEAT_COLOR   RGB_LED_YELLOW         // The warm start flash color is yellow
#define CAL_BEAT_COLOR    RGB_LED_BLUE           // The calibration flash color is blue
#define NORMAL_BEAT_COLOR RGB_LED_GREEN          // The normal beat flash color is green
#define RTC_BEAT_COLOR    RGB_LED_MAGENTA        // The RTC calibration flash color is magenta

/****
 *
 *    Global variables
 *
 ****/
Escapement e;                                    // Instantiate the escapement object on the default hardware pins
Clock c;                                         // Instantiate a clock object on digital pins 10 and 11
LRremote remote(IR_PIN);                         // Instantiate an LRremote object for our little red IR remote control
RgbLed led(LED_R_PIN, LED_G_PIN, LED_B_PIN);     // Instantiate a simple RGB LED
static byte stepix = 3;                          // The adjustment stepsize index (3 = 1 minute)
const long stepSize[6] = {                       // The adjustment stepsize in tenths of a second
  1, 10, 100, 600, 6000, 36000                   // 0.1s, 1s, 10s, 1m, 10m, 1h
};
boolean adjustable = false;                      // Whether the adjustments are enabled (by Power button)
boolean adjRTC = false;                          // Whether speedAdj is to be applied to the real-time clock
long speedAdj = 0;                               // The amount of the pending speed adjustment in tenths of a second; 0 if none
byte ledBeatColor = RGB_LED_BLACK;

/****
 *
 *  Initialization routine: Called once when power is applied or when the Arduino is reset
 *
 ****/

void setup() {
  pinMode(CS_PIN, INPUT);                        // Set the cold start pin to INPUT.
  digitalWrite(CS_PIN, HIGH);                    // Turn on pullup resistor. Installing cold start jumper takes this to Gnd
  Serial.begin(9600);                            // Initialize serial communications
  Serial.println(VERSION_STRING);                // Announce ourselves
  e.enable();                                    // Initialize the Escapement
  if (digitalRead(CS_PIN) == LOW) {              // If it's a forced cold start
    e.setRunMode(COLDSTART);
  }
  
  Serial.print(F("Arduino RTC: "));
  Serial.print(e.getBias()/10.0, 1);             // Display Arduino RTC clock correction
  Serial.println(F(" s/day"));
  switch (e.getRunMode()) {                      // Which runmode are we starting with?
    case RUN:                                    //   Case RUN
      ledBeatColor = NORMAL_BEAT_COLOR;
      Serial.print(F("Hot starting with beatDelta: "));
      Serial.print(e.getBeatDelta()/10.0, 1);
      Serial.println(F(" s/day"));
      break;
      
    case WARMSTART:                              // Case SETTLE
      ledBeatColor = WARM_BEAT_COLOR;
      Serial.println(F("Warm starting with automatic calibration."));
      break;

    case COLDSTART:                              // Case COLDSTART
      ledBeatColor = COLD_BEAT_COLOR;
      Serial.println(F("Cold Starting."));
      break;
  }
  remote.enable();                               // Enable IR remote
}

/****
 *
 *   Button functions -- one for each button on the remote. Called (by remote.onButton()) when the user presses the
 *   corresponding button on the remote.
 *
 ****/

//  Power: Flip adjustable; If going off, post any adjustment(s)
void fPower() {
  if (adjustable) {
    if (speedAdj != 0) {
      if (adjRTC) {
        Serial.print(F("Changing real-time clock speed by: "));
        Serial.print(speedAdj/10.0, 1);
        Serial.print(F(" s/day from "));
        Serial.print(e.getBias()/10.0, 1);
        Serial.print(F(" s/day to "));
        Serial.print(e.incrBias(speedAdj)/10.0, 1);
        Serial.println(F(" s/day."));
      } else {
        Serial.print(F("Changing clock speed by: "));
        Serial.print(speedAdj/10.0, 1);
        Serial.print(F(" s/day from "));
        Serial.print(e.getBeatDelta()/10.0, 1);
        Serial.print(F(" s/day to "));
        Serial.print(e.incrBeatDelta(speedAdj)/10.0, 1);
        Serial.println(F(" s/day."));
      }
    speedAdj = 0;
    }
  Serial.println(F("Adjustments: off."));
  led.setColor(RGB_LED_BLACK);
  } else {
    Serial.println(F("Adjustments: on."));
    led.setColor(stepix);
  }
  adjustable = !adjustable;
}

//  "A" button: Decrement adjustment step size
void fA() {
  if (adjustable) {                              // If it's adjustable
    if (--stepix > 5) {                          //  Decrement step index (mod 5)
      stepix = 5;
    }
    led.setColor(stepix);                        //  Set led color to match
    Serial.print(F("Decremented adjustment step size to "));
    Serial.print(stepSize[stepix]/10.0, 1);
    Serial.println(F(" seconds."));
  }
}

//  "B" button: Cancel adjustments
void fB() {
  if (adjustable) {                              // If it's adjustable
    Serial.println(F("Canceling adjustments."));
    c.cancelDrive();
    if (adjRTC && speedAdj == 0) {               // If doing RTC calibration and no speed adjustment, cancel RTC calibration
      adjRTC = false;
      e.setRunMode(RUN);
      ledBeatColor = NORMAL_BEAT_COLOR;
    } else {                                     // Otherwise just cancl adjustment
      speedAdj = 0;
    }
  }
}

//  "C" button: Increment adjustment step size
void fC() {
  if (adjustable) {                              // If it's adjustable
    if (++stepix > 5) {                          //  Increment step size (mod 5)
      stepix = 0;
    }
    led.setColor(stepix);                        //  Set led color to match
    Serial.print(F("Incremented adjustment step size to "));
    Serial.print(stepSize[stepix]/10.0, 1);
    Serial.println(F(" seconds."));
  }
}

// "Up" button: Increase beat duration by adjustment stepSize[stepIx] tenths of a seconds per day
void fUp() {
  if (adjustable) {
    if (adjRTC) {
      Serial.print(F("Increasing RTC clock speed adjustment by: "));
    } else {
      Serial.print(F("Increasing clock speed adjustment by: "));
    }
    Serial.print(stepSize[stepix]/10.0, 1);
    Serial.print(F(" s/day to "));
    Serial.print((speedAdj += stepSize[stepix])/10.0, 1);
    Serial.println(F(" s/day."));
  }
}

// "Down" button: Decrease beat duration by adjustment stepSize[stepIx] tenths of a seconds per day
void fDown() {
  if (adjustable) {
    if (adjRTC) {
      Serial.print(F("Decreasing RTC clock speed adjustment by: "));
    } else {
      Serial.print(F("Decreasing clock speed adjustment by: "));
    }
    Serial.print(stepSize[stepix]/10.0, 1);
    Serial.print(F(" s/day to "));
    Serial.print((speedAdj -= stepSize[stepix])/10.0, 1);
    Serial.println(F(" s/day."));
  }
}

// "Left" button: Pause clock for the adjustment step size seconds
void fLeft() {
  if (adjustable) {
    Serial.print(F("Pausing clock for "));
    Serial.print(stepSize[stepix]/10);
    Serial.println(F(" seconds."));
    if (stepSize[stepix] <= 21474) {             // Min value for a long is -2,147,483,648. Don't want to underflow
      c.driveMicros(-stepSize[stepix]*100000);
    } else {
      c.driveSec(-stepSize[stepix]/10);
    }
  }
}

// "Right" button: Fast-forward clock by adjustment step size seconds
void fRight() {
  if (adjustable) {
    Serial.print(F("Advancing clock by "));
    Serial.print(stepSize[stepix]/10);
    Serial.println(F(" seconds."));
    if (stepSize[stepix] <= 21474) {             // Max value for a long is 2,147,483,647. Don't want to overflow
      c.driveMicros(stepSize[stepix]*100000);
    } else {
      c.driveSec(stepSize[stepix]/10);
    }
  }
}

//  Select: Start a calibration run
void fSelect () {
  if (adjustable) {
    if (adjRTC) {
      adjRTC = false;                              //   If in RTC calibration switch to warm start
      e.setRunMode(WARMSTART);
      ledBeatColor = WARM_BEAT_COLOR;
      Serial.println(F("Redoing beat calibration."));
    } else {                                       //   Otherwise switch to RTC calibration mode
      adjRTC = true;
      e.setRunMode(CALRTC);
      ledBeatColor = RTC_BEAT_COLOR;
      Serial.println(F("Starting real-time clock calibration."));
    }
    Serial.println(F("Adjustments: off."));        //   In either case, turn adjustments off and let things run
    led.setColor(RGB_LED_BLACK);
    speedAdj = 0;
    adjustable = false;
  }
}

/****
 *
 *   Main loop: Called over and over until power goes off or the Arduino is reset.
 *
 ****/

void loop() {
  // Parallel arrays of IR remote control button codes and button function pointers
  static long code[]          = {SFIRR_POWER, SFIRR_A, SFIRR_B, SFIRR_C, SFIRR_UP, SFIRR_DOWN, SFIRR_LEFT, SFIRR_RIGHT, SFIRR_SELECT};
  static void (*fPointer[])() = {fPower,      fA,      fB,      fC,      fUp,      fDown,      fLeft,      fRight,      fSelect};
  static int codeCount = sizeof(code)/sizeof(code[0]);
  byte cix;                                            // Current led color

  c.driveMicros(e.beat());                             // Let the bendulum do its thing, move the clock forward however long it took
  if (remote.onButton(code, fPointer, codeCount)) {    // If an IR remote buttonwas pushed
    cix = led.getColor();
    led.setColor(RGB_LED_BLACK);                       //   White flash led then return it to its former color to let
    delay(40);                                         //   user know we got a function request
    led.setColor(RGB_LED_WHITE);
    delay(60);
    led.setColor(cix);
  } else if (led.getColor() == RGB_LED_BLACK) {        // Else (i.e., no IR remote button press) if the light's out
    led.setColor(ledBeatColor);                        //   Flash LED whatever the current flash color is
    delay(40);
    led.setColor(RGB_LED_BLACK);
  }

  switch (e.getRunMode()) {                          //   Do things based on the current "run mode"
    case COLDSTART:                                  //   When cold starting
      Serial.print(F("Cold started."));              //    Just say that's what we're doing
      break;
    case WARMSTART:                                  //   When settling in
      Serial.print(F("Warming up. Count "));         //    Say that we're warming up and how far along we've gotten
      Serial.print(e.getBeatCounter());
      Serial.print(F(", delta "));                   //    Display the ratio of tick duration to tock duration
      Serial.print(e.getDelta(), 4);
      Serial.print(F(", current bpm "));             //      the measured beats per minute
      Serial.print(e.getCurBpm(), 4);
      break;
    case CALIBRATE:                                  //   When calibrating
      Serial.print(F("Calibrating. Count "));        //     Say we're calibrating, how much smoothing We've been
      Serial.print(e.getSmoothing());                //       able to do so far,
      Serial.print(F(", delta "));                   //       how symmetrical "ticks" and"tocks" are currently,
      Serial.print(e.getDelta(), 4);                 //       what the current beat's bpm is as measured by the RTC, 
      Serial.print(F(", current bpm "));             //       what the average bpm so far is,
      Serial.print(e.getCurBpm(), 4);
      Serial.print(F(", average bpm "));
      Serial.print(e.getAvgBpm(), 4);
      if (e.isTempComp()) {
        Serial.print(F(", temp "));                  //       and the temperature, if running temp compensated
        Serial.print(e.getTemp(), 4);
        Serial.print(F(" C"));
      }
      ledBeatColor = RGB_LED_BLUE;
      break;
    case CALFINISH:                                  //   When finished calibrating
      Serial.print(F("Finished calibrating. "));
      if (e.isTempComp()) {
        Serial.print(F("Temp: "));
        Serial.print(e.getTemp(), 4);
        Serial.print(F(" C, "));
      }
      Serial.print(F("average bpm "));
      Serial.print(e.getAvgBpm(), 4);
      break;
    case RUN:                                        //   When running
      Serial.print(F("Running. Cal bpm "));          //     Say that we're running along normally, 
      Serial.print(e.getBeatBpm(), 4);               //       what the calibrated beats per minute is,
      Serial.print(F(", current bpm "));             //       what the currentl real-time clock measured bpm is,
      Serial.print(e.getCurBpm(), 4);
      if (e.isTempComp()) {                          //       and, if running temp compensated, 
        Serial.print(F(", temp "));
        Serial.print(e.getTemp(), 4);                //       the temperature reading.
        Serial.print(F(" C"));
      }
      ledBeatColor = NORMAL_BEAT_COLOR;              //     Change beat flash color to normal -- green
      break;
    case CALRTC:                                     //   When calibrating the real-time clock
      Serial.print(F("RTC Calibration. Corr: "));    //     Say that we're calibrating the RTC,
      Serial.print(e.getBias()/10.0, 1);             //       what the current bias is,
      Serial.print(F(" sec, delta "));               //       how symmetrical "ticks" and "tocks" are currently,
      Serial.print(e.getDelta(), 4);                 //       what the current beat's bpm is as measured by the RTC, 
      Serial.print(F(", current Bpm: "));
      Serial.print(e.getCurBpm(), 4);                //       and what the current bpm is
      break;
  }
  Serial.println(F("."));
}

