// To give your project a unique name, this code must be
// placed into a .c file (its own tab).  It can not be in
// a .cpp file or your main sketch (the .ino file).

#include "usb_names.h"

// Edit these lines to create your own name.  The length must
// match the number of characters in your custom name.

#define JOYSTICK_NAME                                          \
  {                                                            \
    'S', 'L', ' ', 'H', 'a', 'n', 'd', 'b', 'r', 'a', 'k', 'e' \
  }
#define JOYSTICK_NAME_LEN 12

// Do not change this part. This exact format is required by USB.

struct usb_string_descriptor_struct usb_string_product_name = {
    2 + JOYSTICK_NAME_LEN * 2,
    3,
    JOYSTICK_NAME};