#include <avr/sleep.h>
#include <CapacitiveSensor.h>
#include <FastLED.h>

// Turn on debug statements to the serial output
#define DEBUG 1

#if DEBUG
#define PRINT(s, x) \
  { \
    Serial.print(F(s)); \
    Serial.println(x); \
  }
#define PRINTS(x) Serial.print(F(x));
#define PRINTD(x) Serial.println(x, DEC);
#else
#define PRINT(s, x)
#define PRINTS(x)
#define PRINTD(x)
#endif

#define LED_COUNT 98
#define LED_PIN1 12
#define LED_PIN2 13

#define MOTION_SENSOR_PIN 2
#define PHOTO_RESISTOR_PIN A0
#define CAPACITIVE_PIN_IN 4
#define CAPACITIVE_PIN_OUT 10

// #define LIGHT_COLOR CRGB(255, 255, 0)  // Light color (R,G,B)
#define DIM_RATE 4  // factor to full brightness

#define STABILITY_WEIGHT 30
#define LIGHT_THRESHOLD 250
#define CAPACITIVE_THRESHOLD 5000
#define LIGHT_ON_DURATION 5000
#define INACTIVITY_TIMEOUT 60000

CapacitiveSensor cs = CapacitiveSensor(
  CAPACITIVE_PIN_IN,
  CAPACITIVE_PIN_OUT);
CRGB leds[LED_COUNT];

unsigned long motionTimeLastDetected;
unsigned long lastActivityTime;
uint8_t preValue;
// CRGB* color = &LIGHT_COLOR;
bool isSeated = false;
bool isMotioned = false;

void setup() {
#if DEBUG
  Serial.begin(9600);
#endif
  PRINTS("\n[START PROGRAM]");

  // Initilized LED object and set up pin.
  FastLED.addLeds<WS2812, LED_PIN1, GRB>(leds, LED_COUNT);
  FastLED.addLeds<WS2812, LED_PIN2, GRB>(leds, LED_COUNT);
  FastLED.clear();
  FastLED.setBrightness(250);

  // Setup sensor pins.
  pinMode(MOTION_SENSOR_PIN, INPUT);
  pinMode(PHOTO_RESISTOR_PIN, INPUT);

  // Turnon auto calibration and time out
  // for capacitive sensor.
  cs.set_CS_AutocaL_Millis(0xFFFFFFFF);
  cs.set_CS_Timeout_Millis(500);

  // Setup interrupt pin for sleep mode
  attachInterrupt(
    digitalPinToInterrupt(MOTION_SENSOR_PIN),
    wakeUp,
    RISING);
}

void loop() {
#if DEBUG
  delay(100);  // Avoid overfloating serial monitor
#endif
  handleMotionSensor();
  handleCapaSensor();
  controlLightBasedOnSensor(getLightData());
  checkAndSleepIfInactive();
  delay(10);
}

uint8_t getLightData() {
  long sum = 0;
  for (int j = 0; j < STABILITY_WEIGHT; j++) {
    sum += analogRead(PHOTO_RESISTOR_PIN);
    delay(10);
  }
  int average = sum / STABILITY_WEIGHT;
  PRINT("Average Light Ambient: ", average);
  return (average < LIGHT_THRESHOLD) ? 0 : map(average, 0, 1023, 0, 250);
}

void handleMotionSensor() {
  if (digitalRead(MOTION_SENSOR_PIN) == HIGH) {
    motionTimeLastDetected = millis();
    isMotioned = true;
    PRINTS("\n[MOTION DETECTED]");
  } else if (millis() - motionTimeLastDetected < LIGHT_ON_DURATION) {
    PRINT("Time wait till the last motion detected: ",
          millis() - motionTimeLastDetected);
  } else if (millis() - motionTimeLastDetected < LIGHT_ON_DURATION) {
    isMotioned = false;
  }
}

void handleCapaSensor() {
  long cs_value = cs.capacitiveSensor(30);
  PRINT("\nCapacitiveSensor value: ", cs_value);
  isSeated = (cs_value >= CAPACITIVE_THRESHOLD)
               ? true
               : false;
}

void controlLightBasedOnSensor(unsigned int lightData) {
  if (!isSeated && !isMotioned) {
    turnOnLight(0);
  } else if (!isSeated && isMotioned) {
    turnOnLight(lightData / DIM_RATE);
  } else {
    turnOnLight(lightData);
  }
}

void checkAndSleepIfInactive() {
  if (millis() - lastActivityTime
      > INACTIVITY_TIMEOUT) {
  }
}

void turnOnLight(uint8_t inputValue) {
  if (preValue != inputValue) {
    fadeAnimation(preValue, inputValue);
    lastActivityTime = millis();
  } else {
    for (int i = 0; i < LED_COUNT; i++) {
      leds[i].setRGB(inputValue, inputValue, inputValue);
    }
    FastLED.show();
  }

  preValue = leds[0].getLuma();
  if (inputValue != 0) {
    lastActivityTime = millis();
  }
}

void fadeAnimation(uint8_t preValue, uint8_t inputValue) {
#if DEBUG
  PRINT("Light brightness value: ", preValue);
#endif
  if (preValue < inputValue) {
    for (int i = preValue; i <= inputValue; i++) {
      for (int j = 0; j < LED_COUNT; j++) {
        leds[j].setRGB(i, i, i);
      }
      delay(5);
      FastLED.show();
    }
  } else {
    for (int i = preValue; i >= inputValue; i--) {
      for (int j = 0; j < LED_COUNT; j++) {
        leds[j].setRGB(i, i, i);
      }
      delay(5);
      FastLED.show();
    }
  }
}

void goToSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();

  sleep_disable();
  lastActivityTime = millis();
}

void wakeUp() {
  // ISR for waking up,
  // nothing needed here
}
