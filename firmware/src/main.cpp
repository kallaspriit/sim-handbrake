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
const int SENSOR_PIN = A7;                               // pressure sensor or potentiometer
const int SHIFTER_UP_PIN = 14;                           // shifter gear up switch pin
const int SHIFTER_DOWN_PIN = 15;                         // shifter gear down switch pin
const int LOOP_FREQUENCY_HZ = 100;                       // Hz
const int UPDATE_INTERVAL_MS = 1000 / LOOP_FREQUENCY_HZ; // joystick update interval in milliseconds
const int DEADZONE_PERCENTAGE = 1;                       // how much deadzone to apply at either end
const int INPUT_RESOLUTION_BITS = 16;                    // analog input resolution in bits
const int CONSOLE_INTERMAL_MS = 1000;                    // log to console interval (set to zero to disable)
const int GEAR_SHIFT_MIN_INTERVAL_MS = 100;              // minimum duration between two gear shift events
const int OUTPUT_RESOLUTION_BITS = 10;                   // normal joystick
const bool SERIAL_ENABLED = false;
// const int OUTPUT_RESOLUTION_BITS = 16; // extreme joystick (requires usb_desc.h modification)

// calculated constants
const int LOOP_DELAY_MS = 1000 / LOOP_FREQUENCY_HZ;
const long INPUT_RANGE = pow(2, INPUT_RESOLUTION_BITS);
const long OUTPUT_RANGE = pow(2, OUTPUT_RESOLUTION_BITS);
const long DEADZONE = floor((DEADZONE_PERCENTAGE / 100.0f) * OUTPUT_RANGE);

// runtime info
unsigned long lastConsoleTime = 0;
unsigned long lastUpdateTime = 0;
unsigned long lastGearShiftTime = 0;
bool wasShifterUpPressed = false;
bool wasShifterDownPressed = false;

// setup app
void setup()
{
  // setup analog
  analogReadRes(INPUT_RESOLUTION_BITS);

  // setup pins
  pinMode(SENSOR_PIN, INPUT);
  pinMode(SHIFTER_UP_PIN, INPUT_PULLUP);
  pinMode(SHIFTER_DOWN_PIN, INPUT_PULLUP);

  // setup manual send mode
  Joystick.useManualSend(true);

  // send startup info after a small delay
  delay(3000);

  // note that serial does not work unless "build_flags =-D USB_SERIAL_HID" is used in platformio.ini
  if (SERIAL_ENABLED)
  {
    // send startup info
    Serial.begin(115200);

    Serial.print("Starting SL Handbrake v1.8.0 at ");
    Serial.print(OUTPUT_RESOLUTION_BITS);
    Serial.print(" bits resolution, running at ");
    Serial.print(LOOP_FREQUENCY_HZ);
    Serial.println("Hz");
  }
}

// main loop
void loop()
{
  // get current time
  unsigned long currentTime = millis();

  // avoid triggering gear shift events too often (for example when bouncing between states because of the spring)
  if (currentTime - lastGearShiftTime >= GEAR_SHIFT_MIN_INTERVAL_MS)
  {
    // read shifter buttons
    bool isShifterUpPressed = digitalRead(SHIFTER_UP_PIN) == LOW;
    bool isShifterDownPressed = digitalRead(SHIFTER_DOWN_PIN) == LOW;

    // handle shifter up button
    if (isShifterUpPressed && !wasShifterUpPressed)
    {

      if (SERIAL_ENABLED)
      {
        Serial.println("shifter up pressed");
      }

      Joystick.button(2, true);

      wasShifterUpPressed = true;
      lastGearShiftTime = currentTime;
    }
    else if (!isShifterUpPressed && wasShifterUpPressed)
    {
      if (SERIAL_ENABLED)
      {
        Serial.println("shifter up released");
      }

      Joystick.button(2, false);

      wasShifterUpPressed = false;
      lastGearShiftTime = currentTime;
    }

    // handle shifter down button
    if (isShifterDownPressed && !wasShifterDownPressed)
    {
      if (SERIAL_ENABLED)
      {
        Serial.println("shifter down pressed");
      }

      Joystick.button(3, true);

      wasShifterDownPressed = true;
      lastGearShiftTime = currentTime;
    }
    else if (!isShifterDownPressed && wasShifterDownPressed)
    {
      if (SERIAL_ENABLED)
      {
        Serial.println("shifter down released");
      }

      Joystick.button(3, false);

      wasShifterDownPressed = false;
      lastGearShiftTime = currentTime;
    }
  }

  if (currentTime - lastUpdateTime >= UPDATE_INTERVAL_MS)
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

    // set joystick value
    Joystick.Z(outputValue);

    // set a button value at mid range
    Joystick.button(1, outputValue > OUTPUT_RANGE / 2);

    // send all updates
    Joystick.send_now();

    // log to console at certain interval (set CONSOLE_INTERMAL_MS to zero to disable)
    if (SERIAL_ENABLED && CONSOLE_INTERMAL_MS > 0 && (lastConsoleTime == 0 || currentTime - lastConsoleTime > CONSOLE_INTERMAL_MS))
    {
      // detailed output
      Serial.print(inputValue);
      Serial.print("/");
      Serial.println(outputValue);

      lastConsoleTime = currentTime;
    }
  }
}
