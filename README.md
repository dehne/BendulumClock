# Arduino Sketches for Bendulum Clocks

Sketches for Bendulum Clocks Copyright 2014 by D. L. Ehnebuske  
License terms: [Creative Commons Attribution-ShareAlike 3.0 United States (CC BY-SA 3.0 US)]
(http://creativecommons.org/licenses/by-sa/3.0/us/ "CC BY-SA 3.0 US")

This is a collection of sketches for the bendulum clock project.

## What's a bendulum clock?

A pendulum clock is one whose timing is determined by the period of a pendulum’s swing. A 
bendulum clock is one whose timing is determined by the period of a bendulum’s swing. A 
“bendulum” is a sort of inverted pendulum, made from a long, thin springy vertical stalk, 
usually made of metal, that’s fixed at the bottom and free to move at the top. Typically, 
the stalk is rectangular in cross section so that it bends easily left and right but not 
forward and back.

A bendulum is topped with a weight. If you give the top a push, it sways back and forth 
periodically. The motion is much like a pendulum, except that instead of swinging rigidly 
from a pivot and moving the most at the bottom, the bendulum’s stem flexes across its 
narrow dimension and the motion is most pronounced at the top.

Like a pendulum a bendulum is approximately isochronous, meaning that its period is not 
too dependent on the amplitude of its swing or on the ambient temperature. Also like a 
pendulum, a bendulum is more accurately isochronous if the swing amplitude is small and 
the temperature is constant.

Since a bendulum is close to isochronous, its periodicity can be used to drive a clock. To 
do so it needs to be equipped with a mechanism that does two things. It needs to detect 
when the bendulum passes through the center of its swing and it needs to give the bendulum 
precisely the same little push each time it passes to keep it going. In a pendulum clock 
such a mechanism is called an “escapement,” and that’s what we call it here, too.

The bendulum escapement discussed here uses a strong button magnet for the bendulum weight 
and swings it past a many-turn coil of fine copper wire located at the center of the 
bendulum’s swing. The coil is attached, via a custom Arduino shield, to an analog input 
and a digital output of an Arduino Uno microcontroller. When the magnet swings past the 
coil, it induces a small pulse of electricity that the sketch running in the Arduino detects 
on the analog input. Very quickly thereafter, the sketch uses the Arduino’s digital output 
to send a short pulse through the coil. The pulse induces a magnetic field in the coil which 
gives the magnet the little push it needs.

The sketch also uses its detection of the passing magnet as timing information to drive a 
clock movement that moves the clock's hands.

## What's in the collection of sketches and what do they do?

### BendulumClock

The BendulumClock sketch implements a complete, automatically calibrating, temperature
compensated bendulum clock. It drives the bendulum so that it keeps going and uses the 
calibrated length of each beat to to drive the clock movement to advance the hands 
of the clock. The sketch supports an IR remote control that can be used to regulate and 
adjust the clock. Finally, it implements automatic temperature-compensated calibration to 
determine the length of the bendulum's beat using the Arduino's built in realtime clock. The 
realtime clock isn't accurate enough to do the full job out of the box, but it can itself be 
easily calibrated using the RealTimeClockCal sketch, below.

### Escapement

The Escapement sketch was the first main sketch for the project. It is largely the same as the 
BendulumClock sketch, above, but is not temperature compensated. In normal operation, bendulum 
clocks can run this sketch but they will drift if the temperature changes by a few degrees C. 

### DriveBendulum

This sketch tests to see that the bendulum can be driven and lets you adjust a trim pot on 
the shield to set the strength of the pulses that drive the bendulum.

### DriveClock

This sketch tests to see that the clock movement can be driven to move the hands of the clock 
and lets you adjust a trim pot on the shield to set the strength of the pulses that drive the 
clock movement.

### RealtimeClockCal

As mentioned above, the real-time clock in the Arduino Uno is not accurate enough for the 
automatic calibration to work perfectly. This sketch gets around that problem by calibrating 
the Arduino's real-time clock. It works very much like the BendulumClock sketch except that it 
drives the clock movement directly from the Arduino's real-time clock instead of from the 
bendulum. When you use the IR remote to regulate the clock to make it run at the correct 
speed, the sketch calculates the adjustment factor needed to make the real-time clock run at 
the correct speed. It stores this information in the Arduino's EEPROM where it will be picked 
up by the BendulumClock sketch, making the automatic calibration much more accurate. An accurate 
automatic calibration means it will take fewer steps to regulate the clock when it runs using 
the bendulum as its isochronous element.