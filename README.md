# Foobler Arduino Sketch

This is an Arduino sketch to allow an Arduino to replace the otherwise unreprogrammable microcontroller in the Foobler

## Rough Circuit Diagram

|IO|uC pin|Arduino pin|Function      |Polarity       |
|--|------|-----------|--------------|---------------|
|O |1     |-          |Ground        |-              |
|I |2     |-          |Battery +     |-              |
|I |3     |-          |Battery +     |-              |
|O |4     |11         |LED "sense"   |-              |
|? |5     |10         |Green LED     |Low = on       |
|I |6     |9          |Power switch  |High = pressed |
|I |7     |8          |30 minute     |High = selected|
|I |8     |7          |60 minute     |High = selected|
|I |9     |6          |90 minute     |High = selected|
|I |10    |-          |Battery +     |-              |
|O |11    |5          |Red LED       |Low = on       |
|O |12    |4          |Motor         |High = on      |
|I |13    |3          |Battery health|High = good    |
|I |14    |2          |Index switch  |High = closed  |

### LED control

The LEDs are a four pin chip which _looks_ like it would just be two LEDs packaged together, however driving it based on that assumption means that the green LED will not light. The assumption is that it's some weird arrangement where there is some form of feedback to the microcontroller about current usage or equivalent.

Therefore in my version of this, I've removed the LED chip and replaced it with two discrete LEDs.

### Timing

The timing switch is connected to the microcontroller using three IO lines. These go high to select 30, 60 and 90 minutes respectively. 15 minutes is selected when none of the others are high.

## Compiling

This is an Arduino sketch built with PlatformIO.

You should be able to run the sketch through the Arduino IDE with no modifications, however it was written with PlatformIO as that has / integrates with better IDE features.

Note that this uses standard AVR GCC / libc features to do the power saving bits in the "off" state, so the full set of AVR GCC headers are required.

## Physical description

1. *Orange end caps:* The one at the "food" end contains a locking screw piece in newer models. The one at the "motor" end contains the entirety of the electronics including the batteries
2. *The blue body:* contains six food hoppers accessible from the "food" end. The openings for the end caps are keyed so they cannot be put in the wrong opening. This also contains the "gate". Food flows from the hoppers, through the hole in the gate, into a roughly torus shaped "inner" area between the gate and the outer surface of the ball. There are then 4 holes in the outside of the ball for food to flow through.
3. *The gate:* a large orange cylinder driven by the motor which contains a single hole through which food can flow through. The motor end contains a large funnel to guide the motor shaft into the area between the hoppers. There is also a small nub on the motor end to operate the index switch. The position of the hole is such that when the gate is rotated _just_ beyond the index switch, the hole is centred over the opening in the hopper.
4. *Motor end cap:* This consists of two main parts: the battery cover, which is on the outside of the ball, and the motor assembly. A captive screw holds these together. The battery cover has a hole in it to allow access to the power switch.
5. *Motor assembly:* Two buttons, the recessed power button on the underside which is clear so the LEDs can shine through and the black index switch on the inside near the edge. There is also the white motor shaft in the centre and the timer selection switch near the index switch. Instructions are embossed around the timer switch.
6. *Inside:* Almost all the electronics are on a single board, with only the motor and battery terminals external to that. The motor is a standard commutated DC motor which drives a set of gears which drive the external shaft and a wheel which operates the bell. The battery holders are moulded into the plastic.
7. *PCB:* On the PCB there are these main assemblies:
   * The battery management pieces consisting of a 3 pin health monitor chip and a large capacitor to protect against momentary battery disconnections. The health monitor is connected directly across the battery with the output pin tied directly to the microcontroller.
   * The motor controller, a couple of resistors and a transistor or MOSFET to drive the motor. The motor has a capacitor on-board and a small ceramic capacitor soldered directly to it's terminals. It's arranged so that the transistor or MOSFET grounds one side of the motor with the other connected to the battery.
   * The LED circuit, two "red" and "green" IO pins with resistors, the LED chip itself, who's "sense" pin is tied directly to the microcontroller with a positive pin tied directly to the battery.
   * Timing switch, a complicated switching arrangement which essentially connects three of the microcontroller's pins to the battery depending on the position of the switch. All three pins have pull-down resistors.
   * Both external switches simply connect one of the microcontroller's pins to the battery. The index switch has a pull-down resistor.
   * The microcontroller has a decoupling capacitor.

## Stock Operation

1. When powered, the microcontroller goes into a low-power "off" state.
2. Pressing the power button turns the device on and the green LED flashes. It will continue flashing until the entire cycle is completed or the microcontroller detects an error.
3. The motor runs until the index switch is pressed
4. If the index switch is not pressed within a certain time limit, it's assumed that the gate cylinder inside is jammed and the microcontroller switches to a "error" state with the red LED flashing. After a while it will return to the low power state.
5. The microcontroller then waits until the time is up, depending on the setting of the switches.
6. Stem 3-5 are repeated another 5 times so all 6 hoppers are emptied.
7. The microcontroller then returns to a low power state with both LEDs off.
8. If the battery health monitor reports a problem at any point, the microcontroller switches to a "bad battery" state with a sold red LED. After a while it will return to the low power state.
9. If the power button is held down for a couple of seconds while outside of the low-power state, the microcontroller will return to that state.

## Modifications

### Different timing
`#define`s are provided at the top of `src/foobler.ino` for setting different time periods for the four switch states, making it easy to reduce the time required for experienced dogs.

### Other operation modes
Other operation modes could include manually pushing the power button to advance the gate instead of having a fixed time limit. This could be useful for teaching dogs how to use it. 

### Futher hacking
Replacing the microcontroller with a different controller, e.g. an ESP8266 board, could enable things like remote control over WiFi, starting the cycle at a certain time, etc.

## Product review

Our dog (a very food motivated Dalmatian) quite enjoys it, however our back yard isn't entirely flat, so he frequently "loses" it if it rolls off the concrete slab and into any of the hollows he has dug. We find that he quickly empties each of the hoppers, meaning that the stock times are too long for him and he quickly gets bored with it or loses it after unsuccessfully trying to retrieve more food from the currently open hopper after it has emptied.

A friend, who this project is for, wants to use the toy to dispense her dog's dinner and consequently wants the timing to be of the order of several tens of seconds instead of the tens of minutes it is stock.
