/*
 * 1. Choose correct board (Tools > Board > "Teensy 3.2/3.1")
 * 2. Choose Serial + HID USB type (USB Type > "Serial + Keyboard + Mouse + Joystick")
 * 3. Edit arduino/hardware/teensy/avr/cores/teensy3/usb_desc.h and under "#elif defined(USB_SERIAL_HID)"
 *    change "#define JOYSTICK_SIZE 12" to "#define JOYSTICK_SIZE 64" which enables 16 bit extreme joystick
 *    (or C:\Users\kalla\.platformio\packages\framework-arduinoteensy\cores\teensy3 when using platformio)
 */

#include <Arduino.h>

// configuration
const int SENSOR_PIN = A7;         // pressure sensor or potentiometer
const int LOOP_FREQUENCY_HZ = 100; // Hz
const int HEADROOM_PERCENTAGE = 1;
const int INPUT_RESOLUTION_BITS = 16;
const int OUTPUT_RESOLUTION_BITS = 16;

// calculated constants
const int LOOP_DELAY_MS = 1000 / LOOP_FREQUENCY_HZ;
const long INPUT_RANGE = pow(2, INPUT_RESOLUTION_BITS);
const long OUTPUT_RANGE = pow(2, OUTPUT_RESOLUTION_BITS);
const long HEADROOM = floor((HEADROOM_PERCENTAGE / 100.0f) * INPUT_RANGE);

// auto-calibration info
long smallestRawValue = INPUT_RANGE * 0.25L;
long largestRawValue = INPUT_RANGE * 0.75L;

// setup app
void setup()
{
  // setup analog
  analogReadRes(INPUT_RESOLUTION_BITS);

  // setup pins
  pinMode(SENSOR_PIN, INPUT);

  // send startup info
  Serial.begin(115200);
  Serial.print("Starting joystick at ");
  Serial.print(OUTPUT_RESOLUTION_BITS);
  Serial.print(" bits resolution, running at ");
  Serial.print(LOOP_FREQUENCY_HZ);
  Serial.println("Hz");

  // setup manual send mode
  Joystick.useManualSend(true);
}

// main loop
void loop()
{
  // read current value
  long inputValue = analogRead(SENSOR_PIN);

  // auto-calibrate smallest value
  if (inputValue < smallestRawValue)
  {
    smallestRawValue = inputValue;
  }

  // auto-calibrate largest value
  if (inputValue > largestRawValue)
  {
    largestRawValue = inputValue;
  }

  // map raw value to output range (use calibration values)
  // long outputValue = map(inputValue, smallestRawValue, largestRawValue, 0L, OUTPUT_RANGE - 1L);
  // long outputValue = map(inputValue, 0L, INPUT_RANGE - 1L, 0L, OUTPUT_RANGE - 1L);
  long outputValue = inputValue;

  // cut of small proportion of the low and high end to be able to always hit the limits
  if (outputValue < HEADROOM)
  {
    outputValue = 0L;
  }
  else if (outputValue > OUTPUT_RANGE - 1L - HEADROOM)
  {
    outputValue = OUTPUT_RANGE - 1L;
  }

  // Serial.print("Input value: ");
  // Serial.println(inputValue);
  // Serial.print("Smallest raw value: ");
  // Serial.println(smallestRawValue);
  // Serial.print("Largest raw value: ");
  // Serial.println(largestRawValue);
  // Serial.print("Output value: ");
  // Serial.println(outputValue);
  // Serial.println();

  // set joystick value
  Joystick.Z(outputValue);

  // send all updates
  Joystick.send_now();

  // delay to run at requested LOOP_FREQUENCY_HZ
  delay(LOOP_DELAY_MS);
}
