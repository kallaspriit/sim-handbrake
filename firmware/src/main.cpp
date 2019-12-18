/*
 * 1. Choose correct board (Tools > Board > "Teensy 3.2/3.1")
 * 2. Choose Serial + HID USB type (USB Type > "Serial + Keyboard + Mouse + Joystick")
 * 3. [ONLY FOR EXTREME 16 bit joystick]
 *    Edit arduino/hardware/teensy/avr/cores/teensy3/usb_desc.h and under "#elif defined(USB_SERIAL_HID)"
 *    change "#define JOYSTICK_SIZE 12" to "#define JOYSTICK_SIZE 64" which enables 16 bit extreme joystick
 *    (or C:\Users\kalla\.platformio\packages\framework-arduinoteensy\cores\teensy3 when using platformio)
 */

#include <Arduino.h>

// configuration
const int SENSOR_PIN = A7;             // pressure sensor or potentiometer
const int LOOP_FREQUENCY_HZ = 100;     // Hz
const int DEADZONE_PERCENTAGE = 1;     // how much deadzone to apply at either end
const int INPUT_RESOLUTION_BITS = 16;  // analog input resolution in bits
const int OUTPUT_RESOLUTION_BITS = 10; // normal joystick
const int CONSOLE_INTERMAL_MS = 100;   // log to console interval (set to zero to disable)
// const int OUTPUT_RESOLUTION_BITS = 16; // extreme joystick (requires usb_desc.h modification)

// calculated constants
const int LOOP_DELAY_MS = 1000 / LOOP_FREQUENCY_HZ;
const long INPUT_RANGE = pow(2, INPUT_RESOLUTION_BITS);
const long OUTPUT_RANGE = pow(2, OUTPUT_RESOLUTION_BITS);
const long DEADZONE = floor((DEADZONE_PERCENTAGE / 100.0f) * OUTPUT_RANGE);

// console timer
long lastConsoleTime = 0;

// setup app
void setup()
{
  // setup analog
  analogReadRes(INPUT_RESOLUTION_BITS);

  // setup pins
  pinMode(SENSOR_PIN, INPUT);

  // setup manual send mode
  Joystick.useManualSend(true);

  // send startup info
  Serial.begin(115200);

  // send startup info after a small delay
  delay(3000);

  Serial.print("Starting Priit Sim Racing Handbrake v1.6.0 at ");
  Serial.print(OUTPUT_RESOLUTION_BITS);
  Serial.print(" bits resolution, running at ");
  Serial.print(LOOP_FREQUENCY_HZ);
  Serial.println("Hz");
}

// main loop
void loop()
{
  // read current value
  long inputValue = analogRead(SENSOR_PIN);

  // map input value to output range
  long outputValue = map(inputValue, 0L, INPUT_RANGE - 1L, 0L, OUTPUT_RANGE - 1L);

  // cut of small proportion of the low and high end to be able to always hit the limits
  if (outputValue < DEADZONE)
  {
    outputValue = 0L;
  }
  else if (outputValue > OUTPUT_RANGE - 1L - DEADZONE)
  {
    outputValue = OUTPUT_RANGE - 1L;
  }

  // get current time
  long currentTime = millis();

  // log to console at certain interval (set CONSOLE_INTERMAL_MS to zero to disable)
  if (CONSOLE_INTERMAL_MS > 0 && (lastConsoleTime == 0 || currentTime - lastConsoleTime > CONSOLE_INTERMAL_MS))
  {
    // detailed output
    Serial.print(inputValue);
    Serial.print("/");
    Serial.println(outputValue);

    lastConsoleTime = currentTime;
  }

  // set joystick value
  Joystick.Z(outputValue);

  // set a button value at mid range
  Joystick.button(1, outputValue > OUTPUT_RANGE / 2);

  // send all updates
  Joystick.send_now();

  // delay to run at requested LOOP_FREQUENCY_HZ
  delay(LOOP_DELAY_MS);
}
