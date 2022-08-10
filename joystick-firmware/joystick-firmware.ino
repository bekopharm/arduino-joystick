const int pin_JoystickLX = A0;
const int pin_JoystickLY = A1;
const int pin_JoystickRX = A2;
const int pin_JoystickRY = A3;

// D-PAD + ABXY + START & SELECT + L1 & R1 + L2 & R2 + 34 TriggerHappies
// !! Must be 3 * 16 !!
// Reading the buttons in bunches of 16 later
int buttons[48] = {
   2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 22, 23,
  24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
  38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
  52, 53, A4, A5, A6, A7,
};

// TODO: add me some rotary encoders too

int axes[4] = {
  pin_JoystickLX,
  pin_JoystickLY,
  pin_JoystickRX,
  pin_JoystickRY
};

void setup() {
  Serial.begin(115200);

  pinMode(pin_JoystickLX, INPUT);
  pinMode(pin_JoystickLY, INPUT);
  pinMode(pin_JoystickRX, INPUT);
  pinMode(pin_JoystickRY, INPUT);

  for (int i = 0; i < 48; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
  }
}

void loop() {
  uint16_t payload[8];

  payload[0] = UINT16_MAX; // header: 16 1s (2 byte)

  for (uint8_t i = 0; i < 3; i++) { // 6 bytes buttons
    payload[(i+1)] = readButtonStateBatch(i);
  }

  for (uint8_t i = 0; i < 4; i++) { // 8 bytes axes
    payload[(i+4)] = analogRead(axes[i]);
  }

  for (uint8_t i = 0; i < 8; i++) {
    writeShort(payload[i]);
  }

  Serial.flush();

  delay(10);
}

/**
* Reads the buttons in bunches of 16 starting with a multiply of x
*/
uint16_t readButtonStateBatch(uint8_t x) {
  uint16_t buttonState = 0;
  uint8_t c = 0;

  for (uint8_t i = (x * 16); i < (x * 16 + 16); i++) {
    if (digitalRead(buttons[i]) == LOW) {
      buttonState |= (1 << c);
    }
    c++;
  }
  
  return buttonState;
}

// Remember for comfy checking of the data streams:
// xxd -b -g 2 -c 16 /dev/ttyUSB0
// xxd -g 2 -c 16 /dev/ttyUSB0
void writeShort(uint16_t value) {
  Serial.write((uint8_t) value);
  Serial.write((uint8_t) (value >> 8) & 255);
}
