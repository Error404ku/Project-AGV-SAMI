// Variables for button handling
bool tombolBoot = false;
unsigned long bootHoldStart = 0;
int lastPressed;

// Button pin variables
int upPin = 10;
int downPin = 40;
int rightPin = 39;
int leftPin = 42;
int startPin = 9;
int stopPin = 41;

// Debounce variables
unsigned long lastUpPress = 0;
unsigned long lastDownPress = 0;
unsigned long lastLeftPress = 0;
unsigned long lastRightPress = 0;
unsigned long lastStartPress = 0;
unsigned long lastStopPress = 0;
void setupTombol() {
  pinMode(BOOT_PIN, INPUT_PULLUP);
  
  // Setup manual assigned button pins for ACTIVE HIGH with internal pull-down
  pinMode(upPin, INPUT_PULLDOWN);     // UP - active HIGH
  pinMode(downPin, INPUT_PULLDOWN);   // DOWN - active HIGH
  pinMode(leftPin, INPUT_PULLDOWN);   // LEFT - active HIGH
  pinMode(rightPin, INPUT_PULLDOWN);  // RIGHT - active HIGH
  pinMode(startPin, INPUT_PULLDOWN);  // START - active HIGH
  pinMode(stopPin, INPUT_PULLDOWN);   // STOP - active HIGH
  
  // Serial.println("SETUP TOMBOL MANUAL (ACTIVE HIGH + PULLDOWN):");
  // Serial.println("UP=39, DOWN=40, LEFT=41, RIGHT=42, START=2, STOP=1");
  // Serial.println("Tekan = HIGH, Tidak tekan = LOW (pulldown)");
  
  // Display setup complete message
  lcd.setCursor(0, 0);
  lcd.print("TOMBOL PULLDOWN");
  delay(1500);
  lcd.clear();
}

// Non-blocking debounce variables
static unsigned long lastDebounceTime[6] = {0}; // UP, DOWN, LEFT, RIGHT, START, STOP
static bool lastButtonState[6] = {false};
static bool buttonState[6] = {false};
const unsigned long debounceDelay = 50; // Optimized debounce delay

// Optimized non-blocking button reading function
bool readButtonWithDebounce(int pin, int buttonIndex) {
  bool reading = digitalRead(pin) == HIGH;
  
  if (reading != lastButtonState[buttonIndex]) {
    lastDebounceTime[buttonIndex] = millis();
  }
  
  if ((millis() - lastDebounceTime[buttonIndex]) > debounceDelay) {
    if (reading != buttonState[buttonIndex]) {
      buttonState[buttonIndex] = reading;
      lastButtonState[buttonIndex] = reading;
      return reading; // Return true only on state change to HIGH
    }
  }
  
  lastButtonState[buttonIndex] = reading;
  return false;
}

// Button functions using digitalRead == HIGH format with 50ms debounce
bool UP() {
  return readButtonWithDebounce(upPin, 0);
}

bool LEFT() {
  return readButtonWithDebounce(leftPin, 2);
}

bool RIGHT() {
  return readButtonWithDebounce(rightPin, 3);
}

bool DOWN() {
  return readButtonWithDebounce(downPin, 1);
}

bool START() {
  return readButtonWithDebounce(startPin, 4);
}

bool STOP() {
  return readButtonWithDebounce(stopPin, 5);
}

void uji_tombol() {
  // Show button press on LCD at position (13,2) - right side
  // NOTE: No debounce in test function for real-time feedback
  lcd.setCursor(13, 2);
  
  if (digitalRead(upPin) == HIGH) {
    lcd.print("UP ");
    // Serial.println("Tombol UP ditekan");
  }
  else if (digitalRead(leftPin) == HIGH) {
    lcd.print("LF ");
    // Serial.println("Tombol LEFT ditekan");
  }
  else if (digitalRead(rightPin) == HIGH) {
    lcd.print("RT ");
    // Serial.println("Tombol RIGHT ditekan");
  }
  else if (digitalRead(downPin) == HIGH) {
    lcd.print("DN ");
    // Serial.println("Tombol DOWN ditekan");
  }
  else if (digitalRead(startPin) == HIGH) {
    lcd.print("Start  ");
    // Serial.println("Tombol START ditekan");
  }
  else if (digitalRead(stopPin) == HIGH) {
    lcd.print("Stop  ");
    // Serial.println("Tombol STOP ditekan");
  }
  else {
    lcd.print("       "); // Clear if no button pressed
  }
}