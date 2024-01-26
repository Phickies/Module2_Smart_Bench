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
#define LED_PIN 12

#define MOTION_SENSOR_PIN 2
#define PHOTO_RESISTOR_PIN A0
#define CAPACITIVE_PIN_IN 4
#define CAPACITIVE_PIN_OUT 10

#define LIGHT_COLOR CRGB(255, 255, 255)  // Light color (R,G,B)
#define DIM_RATE 2                       // factor to full brightness

#define STABILITY_WEIGHT 20
#define LIGHT_THRESHOLD 250
#define CAPACITIVE_THRESHOLD 10000
#define LIGHT_ON_DURATION 30000
#define INACTIVITY_TIMEOUT 60000

CapacitiveSensor cs = CapacitiveSensor(
  CAPACITIVE_PIN_IN,
  CAPACITIVE_PIN_OUT);
CRGB leds[LED_COUNT];

unsigned long motionTimeLastDetected;
unsigned long lastActivityTime;
CRGB* color = &LIGHT_COLOR;
bool isSeated = false;

void setup() {
#if DEBUG
  Serial.begin(9600);
#endif
  PRINTS("\n[START PROGRAM]");

  // Initilized LED object and set up pin.
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_COUNT);

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
  return (average < LIGHT_THRESHOLD) ? 0 : map(average, 0, 1023, 0, 255);
}

void handleMotionSensor() {
  if (digitalRead(MOTION_SENSOR_PIN) == HIGH) {
    motionTimeLastDetected = millis();
    PRINTS("\n[MOTION DETECTED]");
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
  if (!isSeated) {
    turnOnLight(0, color);
  } else if (!isSeated
             && millis() - motionTimeLastDetected < LIGHT_ON_DURATION) {
    PRINT("Time wait till the last motion detected: ",
          millis() - motionTimeLastDetected);
    turnOnLight(lightData / DIM_RATE, color);
  } else {
    turnOnLight(lightData, color);
  }
}

void checkAndSleepIfInactive() {
  if (millis() - lastActivityTime
      > INACTIVITY_TIMEOUT) {
  }
}

void turnOnLight(uint8_t inputValue, CRGB* light_color) {
  if (inputValue == 0) {
    return;
  }
  fadeAnimation(inputValue);
  FastLED.showColor(light_color);
  lastActivityTime = millis();
}

void fadeAnimation(uint8_t inputValue) {
  uint8_t curVal = FastLED.getBrightness();
#if DEBUG
  PRINT("Light brightness value: ", curVal);
#endif
  while (curVal < inputValue) {
    FastLED.setBrightness(curVal++);
    delay(5);
  }
  while (curVal > inputValue) {
    FastLED.setBrightness(curVal--);
    delay(5);
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
