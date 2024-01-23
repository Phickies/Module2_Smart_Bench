#include <avr/sleep.h>
#include <CapacitiveSensor.h>
#include <FastLED.h>

#define LED_B_PIN 3
#define LED_G_PIN 5
#define LED_R_PIN 6

#define LED_COUNT 98
#define LED_PIN 12

#define MOTION_SENSOR_PIN 2
#define PHOTO_RESISTOR_PIN A0
#define CAPACITIVE_PIN_IN 4
#define CAPACITIVE_PIN_OUT 10

#define DIM_RATE 10
#define STABILITY_WEIGHT 20
#define LIGHT_THRESHOLD 250
#define CAPACITIVE_THRESHOLD 10000
#define LIGHT_ON_DURATION 30000
#define INACTIVITY_TIMEOUT 60000

CapacitiveSensor cs = CapacitiveSensor(
  CAPACITIVE_PIN_IN,
  CAPACITIVE_PIN_OUT);
CRGB leds[LED_COUNT];

unsigned long inputLastDetectedAt = 0;
unsigned long lastActivityTime = 0;
boolean isSeated = false;

void setup() {
  Serial.begin(9600);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_COUNT);

  pinMode(LED_B_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(MOTION_SENSOR_PIN, INPUT);

  // Setup interrupt pin for sleep mode
  attachInterrupt(
    digitalPinToInterrupt(MOTION_SENSOR_PIN),
    wakeUp,
    RISING);

  // Turnon auto calibration and time out
  // for capacitive sensor.
  cs.set_CS_AutocaL_Millis(0xFFFFFFFF);
  cs.set_CS_Timeout_Millis(500);
}

void loop() {
  unsigned int lightData = getLightData();

  handleMotionSensor();
  handleCapaSensor();
  controlLightBasedOnSensor(lightData);
  checkAndSleepIfInactive();

  delay(100);
}

unsigned int getLightData() {
  long sum = 0;
  for (int j = 0; j < STABILITY_WEIGHT; j++) {
    sum += analogRead(PHOTO_RESISTOR_PIN);
    delay(10);
  }
  int average = sum / STABILITY_WEIGHT;
  Serial.println(average);
  return (average < LIGHT_THRESHOLD) ? 0 : map(average, 0, 1023, 0, 255);
}

void handleMotionSensor() {
  if (digitalRead(MOTION_SENSOR_PIN) == HIGH) {
    inputLastDetectedAt = millis();
    Serial.println("isMotioned");
  }
}

void handleCapaSensor() {
  long cs_value = cs.capacitiveSensor(30);
  Serial.print("Capacitive value: ");
  Serial.println(cs_value);
  isSeated = (cs_value >= CAPACITIVE_THRESHOLD)
               ? true
               : false;
}

void controlLightBasedOnSensor(unsigned int lightData) {
  if (lightData > 0) {
    if (isSeated) {
      turnOnLight(lightData);
    } else if (millis() - inputLastDetectedAt
               < LIGHT_ON_DURATION) {
      Serial.print("Motion Inactive Time Left: ");
      Serial.println(millis() - inputLastDetectedAt);
      turnOnLight(DIM_RATE);
    } else {
      turnOnLight(0);
    }
  } else {
    turnOnLight(0);
    inputLastDetectedAt = 0;
  }
}

void checkAndSleepIfInactive() {
  if (millis() - lastActivityTime
      > INACTIVITY_TIMEOUT) {
    goToSleep();
  }
}

void turnOnLight(int inputValue) {
  for (int i = 0; i < LED_COUNT; i++){
    leds[i] = CRGB(255, 255, 255);
    leds[i].maximizeBrightness(inputValue);
  }
  FastLED.show();  
  lastActivityTime = millis();
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
