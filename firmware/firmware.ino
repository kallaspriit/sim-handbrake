/*
 * 1. Choose correct board (Tools > Board > "Teensy 3.2/3.1")
 * 2. Choose Serial + HID USB type (USB Type > "Serial + Keyboard + Mouse + Joystick")
 * 3. Edit arduino/hardware/teensy/avr/cores/teensy3/usb_desc.h and under "#elif defined(USB_SERIAL_HID)"
 *    change "#define JOYSTICK_SIZE 12" to "#define JOYSTICK_SIZE 64" which enables 16 bit extreme joystick
 */

// configuration
const int LOOP_FREQUENCY = 100;
const int LOOP_DELAY_MS = 1000 / LOOP_FREQUENCY;
const int OUTPUT_RESOLUTION_BITS = 16;
const int OUTPUT_RANGE = pow(2, OUTPUT_RESOLUTION_BITS); // 16 bit output resolution

// runtime info
int value = 0;
int speed = (OUTPUT_RANGE / LOOP_FREQUENCY / 10); // goes from zero to max in 10 seconds


// setup app
void setup() {
  Serial.begin(115200);
  Serial.print("Starting joystick at ");
  Serial.print(OUTPUT_RESOLUTION_BITS);
  Serial.print(" bits resolution, running at ");
  Serial.print(LOOP_FREQUENCY);
  Serial.println("Hz");

  Joystick.useManualSend(true);
}

// main loop
void loop() {
  // set joystick value
  Joystick.Z(value);

  // calculate new value 
  value = max(min(value + speed, OUTPUT_RANGE - 1), 0);

  // flip the value once either of the range endpoints are reached
  if (value == OUTPUT_RANGE - 1 || value == 0) {
    speed *= -1;
  }

  // send all updates
  Joystick.send_now();

  // delay to run at requested LOOP_FREQUENCY
  delay(LOOP_DELAY_MS);
}
