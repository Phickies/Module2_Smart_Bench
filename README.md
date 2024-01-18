# Smart_Bench
Arduino Code for Smart Bench

## GENERAL
This code is optimised for energy saving.

Requrire libary:
  CapacitiveSensor

1. **Capacitive Sensor Setup**: initialized the capacitive sensor and configured its calibration and timeout settings.

2. **Motion Sensor and Interrupt Setup**: The motion sensor is set up with an external interrupt to wake the Arduino from sleep mode.

3. **Main Loop Functions**:
   - `getLightData` averages the readings from the photoresistor.
   - `handleMotionSensor` updates the timestamp when motion is detected.
   - `handleCapaSensor` checks the capacitive sensor reading and updates the `isSeated` flag.
   - `controlLightBasedOnSensor` controls the light based on sensor inputs and light data.
   - `checkAndSleepIfInactive` puts the Arduino into sleep mode after a period of inactivity.

4. **Light Control Logic**:
   - The light turns on if someone is seated or if there's recent motion, with a dimming effect applied in the latter case.
   - `DIM_RATE` weigth the dim effect for the light, ranging from *0-1* with *1* is open full and *0* is off.
   - `STABILITY_WEIGHT` control the responsiviness and stability of the LDR sensor, max value is *50*, the higher value the more the sensor gonna stablizied more but trade off with responsiveness.

   - The light turns off if no motion is detected within the specified duration or if the ambient light is above the threshold.
   - `LIGHT_THRESHOLD` set the minimums ambient light environment to activate the light.
   - `CAPACITIVE_THRESHOLD` set the minimus value for detecting contact with human, use for callilbrating sensor.
   - `LIGHT_ON_DURATIOIN` duration the light will turn on after milisecond. The default value is 30000 which is 30 seconds.

5. **Sleep Mode Logic**:
   - The Arduino enters sleep mode after 1 minute of inactivity and wakes up upon motion detection.
   - `INACTIVITY_TIMEOUT` set the inactivity timeout in milisecond.

6. **Use of `Serial.println`**:
   - The `Serial.println` inside `controlLightBasedOnSensor` is useful for debugging but should be commented out or removed in the final deployment to conserve power and reduce serial traffic.

### Suggested Improvements:

- **Debouncing Motion Sensor**: If your motion sensor is sensitive to small movements or prone to false triggering, you might want to implement a debouncing mechanism.

