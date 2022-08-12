#include <Arduino.h>
#include <PinChangeInterrupt.h>

const int pin_JoystickLX = A0;
const int pin_JoystickLY = A1;
const int pin_JoystickRX = A2;
const int pin_JoystickRY = A3;

#define ROTARY_ANTICLOCKWISE -1
#define ROTARY_NOCHANGE 0
#define ROTARY_CLOCKWISE 1

/**
 * @brief flags from interrupt (change detected)
 */
volatile bool rotary_encoder_changed[4] = {false, false, false, false};
// Interrupt routines just sets a flag when rotation is detected
inline void rotary0()
{
  rotary_encoder_changed[0] = true;
};
inline void rotary1()
{
  rotary_encoder_changed[1] = true;
};
inline void rotary2()
{
  rotary_encoder_changed[2] = true;
};
inline void rotary3()
{
  rotary_encoder_changed[3] = true;
};
typedef void (*rotaryCallback)();
/**
 * @brief Callbacks to flag that an interrupt happened
*/
rotaryCallback rotary_callbacks[4] = {rotary0, rotary1, rotary2, rotary3};
// The pins that support PCINTs on a Mega2560 board are 0, 10-15, 50-53 and A8-A15.
int rotary_encoders[4][2] = {{50, 51}, {52, 53}, {A12, A13}, {A14, A15}};
// The pins that support INTs on the Mega2560 board are 2, 3, 18, 19, 20, 21

// D-PAD + ABXY + START & SELECT + L1 & R1 + L2 & R2 + 18 TriggerHappies
// !! Must be 2 * 16 !!
// Reading the buttons in bunches of 16 later
int buttons[32] = {
   2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 22, 23, 24, 25,
  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41
};

// TODO: add me some rotary encoders too

int axes[4] = {
  pin_JoystickLX,
  pin_JoystickLY,
  pin_JoystickRX,
  pin_JoystickRY
};


void setup()
{
  Serial.begin(115200);

  pinMode(pin_JoystickLX, INPUT);
  pinMode(pin_JoystickLY, INPUT);
  pinMode(pin_JoystickRX, INPUT);
  pinMode(pin_JoystickRY, INPUT);

  for (int i = 0; i < 32; i++)
  {
    pinMode(buttons[i], INPUT_PULLUP);
  }

  /**
   * Use PinChangeInterrupts over PinInterrupts - I may have use for
   * precious interruptible pins that can do RX/TX or so in the
   * future. Also I'll probably want more than 4 rotary encoders.
   */
  for (int i = 0; i < 4; i++)
  {
    pinMode(rotary_encoders[i][0], INPUT);
    pinMode(rotary_encoders[i][1], INPUT);
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(rotary_encoders[i][0]), rotary_callbacks[i], CHANGE);
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(rotary_encoders[i][1]), rotary_callbacks[i], CHANGE);
  }
}

void loop()
{
  uint16_t payload[8] = {UINT16_MAX, 0, 0, 0, 0, 0, 0, 0};

  for (uint8_t i = 0; i < 2; i++)
  { // 4 bytes buttons
    payload[(i + 1)] = readButtonStateBatch(i);
  }

  // 2 bytes rotary encoders (as buttons)
  payload[3] = readRotaryEncoderStates();

  for (uint8_t i = 0; i < 4; i++)
  { // 8 bytes axes
    payload[(i + 4)] = analogRead(axes[i]);
  }

  for (uint8_t i = 0; i < 8; i++)
  {
    writeShort(payload[i]);
  }

  Serial.flush();
}

/**
 * @brief Check in which direction the rotary encoder moved.
 * 
 * @see https://www.pinteric.com/rotary.html
 * 
 * @param rotary Rotary encoder index
 * @return int8_t ROTARY_NOCHANGE | ROTARY_CLOCKWISE | ROTARY_ANTICLOCKWISE
 */
int8_t checkRotaryEncoder(uint8_t rotary)
{
  // reset flag from interrupt
  rotary_encoder_changed[rotary] = false;

  // retrieve pins in question
  uint8_t l_pin = rotary_encoders[rotary][0];
  uint8_t r_pin = rotary_encoders[rotary][1];

  int8_t lr_result = ROTARY_NOCHANGE;

  static uint8_t lr_mem = 3;
  static int lr_sum = 0;
  static int8_t TRANS[] = {0, -1, 1, 14, 1, 0, 14, -1, -1, 14, 0, 1, 14, 1, -1, 0};

  // Read BOTH pin states to deterimine validity of rotation (ie not just switch bounce)
  int8_t l = digitalRead(l_pin);
  int8_t r = digitalRead(r_pin);

  // Move previous value 2 bits to the left and add in our new values
  lr_mem = ((lr_mem & 0x03) << 2) + (2 * l) + r;

  // Convert the bit pattern to a movement indicator (14 = impossible, ie switch bounce)
  lr_sum += TRANS[lr_mem];

  /* Encoder not in the neutral (detent) state */
  if (lr_sum % 4 != 0)
  {
    return lr_result;
  }

  switch(lr_sum)
  {
    case 4:
      /* Encoder in the neutral state - clockwise rotation */
      lr_result = ROTARY_CLOCKWISE;
      break;
    case -4:
      /* Encoder in the neutral state - anti-clockwise rotation */
      lr_result = ROTARY_ANTICLOCKWISE;
      break;
    default:
      /* An impossible rotation has been detected - ignore */
      break;
  }

  lr_sum = 0;

  return lr_result;
}

/**
 * @brief Sets and debounces bits for each rotary encoder depending
 * on the direction of the rotary encoder check.
 * 
 * FIXME: This uses currently only half of the available uint16_t
 * 
 * @return uint16_t 
 */
uint16_t readRotaryEncoderStates() {
  /**
 * @brief milliseconds since last change for debounce
 */
  static unsigned long last_change_millis[4] = {0, 0, 0, 0};
  static uint16_t encoder_state = 0;

  for (uint8_t i = 0; i < 4; i++)
  {

    if (rotary_encoder_changed[i])
    {
      int8_t rotationValue = checkRotaryEncoder(i);

      switch(rotationValue) {
        case ROTARY_CLOCKWISE:
          encoder_state &= ~(1 << (i*2));
          encoder_state |= (1 << (i*2)+1);
          last_change_millis[i] = millis();
          break;
        case ROTARY_ANTICLOCKWISE:
          encoder_state |= (1 << (i*2));
          encoder_state &= ~(1 << (i*2)+1);
          last_change_millis[i] = millis();
          break;
        default:
          break;
      }
    }

    // Debounce encoder
    if (last_change_millis[i] != 0 && millis() - last_change_millis[i] > 50)
    {
      last_change_millis[i] = 0;
      encoder_state &= ~(1 << (i*2));
      encoder_state &= ~(1 << (i*2)+1);
    }
  }

  return encoder_state;
}

/**
 * @brief Reads the buttons in bunches of 16 starting with a multiply of x
 */
uint16_t readButtonStateBatch(uint8_t x)
{
  uint16_t button_state = 0;
  uint8_t c = 0;

  for (uint8_t i = (x * 16); i < (x * 16 + 16); i++)
  {
    if (digitalRead(buttons[i]) == LOW)
    {
      button_state |= (1 << c);
    }
    c++;
  }

  return button_state;
}

// Remember for comfy checking of the data streams:
// xxd -b -g 2 -c 16 /dev/ttyUSB0
// xxd -g 2 -c 16 /dev/ttyUSB0
void writeShort(uint16_t value)
{
  Serial.write((uint8_t)value);
  Serial.write((uint8_t)(value >> 8) & 255);
}
