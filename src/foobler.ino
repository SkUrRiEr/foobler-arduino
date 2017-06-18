#include "Arduino.h"
#include <avr/power.h>
#include <avr/sleep.h>

/*
 * Speed in seconds
 */
#define SPEED_FACTOR 60
#define SPEED_15M   (15 * SPEED_FACTOR)
#define SPEED_30M   (30 * SPEED_FACTOR)
#define SPEED_60M   (60 * SPEED_FACTOR)
#define SPEED_90M   (90 * SPEED_FACTOR)

/*
 * Defines for pin numbers
 */
#define PIN_LED_UNKNOWN  11
#define PIN_LED_GREEN    10
#define PIN_SWITCH_POWER  9
#define PIN_SPEED_30M     8
#define PIN_SPEED_60M     7
#define PIN_SPEED_90M     6
#define PIN_LED_RED       5
#define PIN_MOTOR         4
#define PIN_BAT_MONITOR   3
#define PIN_SWITCH_INDEX  2

#define PIN_LED_INTERNAL 13

/*
 * LED Mode
 *
 * Bitfield: 0 == red, 1 == green
 */
#define LED_MODE_OFF    0
#define LED_MODE_RED    1
#define LED_MODE_GREEN  2
#define LED_MODE_YELLOW 3

/*
 * Current delay in seconds
 */
int setDelay = SPEED_15M;

/*
 * Sets the delay variable to the selected speed.
 */
void readSpeed() {
  if (digitalRead(PIN_SPEED_30M) == HIGH) {
    setDelay = SPEED_30M;
  } else if (digitalRead(PIN_SPEED_60M) == HIGH) {
    setDelay = SPEED_60M;
  } else if (digitalRead(PIN_SPEED_90M) == HIGH) {
    setDelay = SPEED_90M;
  } else {
    setDelay = SPEED_15M;
  }
}

/*
 * Set mode of all used pins
 * 
 * Turn off the motor and the internal LED.
 * Turn on both external LEDs
 */
void setup() {
  pinMode(PIN_LED_UNKNOWN,  INPUT);
  pinMode(PIN_SWITCH_POWER, INPUT);
  pinMode(PIN_SWITCH_INDEX, INPUT);
  pinMode(PIN_SPEED_30M,    INPUT);
  pinMode(PIN_SPEED_60M,    INPUT);
  pinMode(PIN_SPEED_90M,    INPUT);
  pinMode(PIN_BAT_MONITOR,  INPUT);

  pinMode(PIN_LED_GREEN,    OUTPUT);
  pinMode(PIN_LED_RED,      OUTPUT);
  pinMode(PIN_LED_INTERNAL, OUTPUT);
  pinMode(PIN_MOTOR,        OUTPUT);
  
  digitalWrite(PIN_MOTOR,        LOW);
  setLed(LED_MODE_YELLOW);
  digitalWrite(PIN_LED_INTERNAL, LOW);
}

/*
 * Set the external LED colour
 *
 * TODO: Remove LED chip and replace with separate LEDs.
 */
void setLed(char mode) {
  digitalWrite(PIN_LED_GREEN, (mode & LED_MODE_GREEN) == LED_MODE_GREEN ? LOW : HIGH);
  digitalWrite(PIN_LED_RED,   (mode & LED_MODE_RED)   == LED_MODE_RED   ? LOW : HIGH);
}

/*
 * Sets the motor
 */
void setMotor(int state) {
  digitalWrite(PIN_MOTOR, state);
}

/*
 * State machine
 */
#define STATE_OFF        0
#define STATE_START      1
#define STATE_INDEX_WAIT 2
#define STATE_RUNNING    3
#define STATE_ERROR      4
#define STATE_WAIT       5
#define STATE_BAD        6
// TODO: Hold button to power off?

char state = STATE_OFF;

void power_on_isr() {
  sleep_disable();
  state = STATE_START;
}

/*
 * Main state machine
 */
void loop() {
  if (digitalRead(PIN_BAT_MONITOR) == LOW) {
    state = STATE_BAD;
  }
  
  switch (state) {
    case STATE_OFF: // Power down and wait for the power switch to be pressed
      setLed(LED_MODE_OFF);

      // Prepare for sleep
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      sleep_enable();

      // add ISR for receiving the power switch event
      attachInterrupt(0, power_on_isr, LOW); // TODO: check! - LOW is the only option here!

      // Disable timer
      power_timer0_disable();

      // power off
      sleep_mode();
      
      // powered off
      
      // wake-up happens here
      sleep_disable();

      // Remove power switch ISR
      detachInterrupt(0);

      // Re-enable timer
      power_timer0_enable();

      break;
    case STATE_START: // Blink green LED and choose where to go
      // Green blinking light: Timer?
      
      if (digitalRead(PIN_SWITCH_INDEX) == HIGH) { // Another state?
        state = STATE_INDEX_WAIT;
      } else {
        state = STATE_RUNNING;
      }
      
      break;
    case STATE_INDEX_WAIT: // Motor is running, wait until the index switch has been released
      // Arm timer for index wait => STATE_ERROR
      // Arm ISR for index switch => LOW => STATE_RUNNING

      setMotor(HIGH);

      // Sleep

      break;
    case STATE_RUNNING: // Motor is running, wait until the index switch is pressed
      // Arm timer for index wait => STATE_ERROR;
      // Arm ISR for index switch => HIGH => STATE_WAIT;

      setMotor(HIGH);

      // Sleep
      
      break;
    case STATE_WAIT: // Wait until time is up
      readSpeed();
      delay(setDelay); // Is this powered down enough?
      
      state = STATE_START;
      
      break;
    case STATE_ERROR: // Motor did not hit the index switch within time
      // Kill green blinking LED
      // Red blinking light: Timer?
 
      delay(65535);
 
      setLed(LED_MODE_OFF);
 
      state = STATE_OFF;     
      
      break;
    case STATE_BAD: // Low battery => solid red for a while then off.
      // Kill green blinking LED

      setLed(LED_MODE_RED);
      delay(65535);
      setLed(LED_MODE_OFF);
      
      state = STATE_OFF;

      break;      
  }
}
