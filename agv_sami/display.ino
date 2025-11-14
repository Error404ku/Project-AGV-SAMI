// Fungsi untuk mereset semua flag display
void resetDisplayFlags() {
  resetDisplayRequested = true;
}

void displayPrint() {
  static AgvState lastDisplayedState = AGV_STATE_STOP;
  static bool displayInitialized = false;

  // Reset flag jika diminta
  if (resetDisplayRequested) {
    displayInitialized = false;
  }

  // Hanya update display jika state berubah atau belum diinisialisasi
  if (!displayInitialized || currentStateAgv != lastDisplayedState) {
    lcd.setCursor(0, 0);
    lcd.print("AGV Mode:           ");  // Tambah spasi untuk clear sisa karakter
    lcd.setCursor(0, 1);
    String stateString = agvStateToString(currentStateAgv);
    stateString += "                ";  // Tambah spasi untuk clear sisa karakter
    lcd.print(stateString);

    // Update state tracking
    lastDisplayedState = currentStateAgv;
    displayInitialized = true;
  }

  // Reset global flag setelah digunakan
  if (resetDisplayRequested) {
    resetDisplayRequested = false;
  }
}

void scrollText(int row, const char* message, int delayTime) {
  static unsigned long lastScrollTime = 0;
  static int scrollPos = 0;
  static char currentMessage[64] = "";
  
  // Check if message changed
  if (strcmp(currentMessage, message) != 0) {
    strncpy(currentMessage, message, sizeof(currentMessage) - 1);
    currentMessage[sizeof(currentMessage) - 1] = '\0';
    scrollPos = 0;
    lastScrollTime = millis();
  }

  int msgLen = strlen(currentMessage);
  
  // If message fits on screen, just display with padding
  if (msgLen <= 16) {
    if (millis() - lastScrollTime > delayTime) {
      lastScrollTime = millis();
      lcd.setCursor(0, row);
      char displayBuffer[17];
      snprintf(displayBuffer, sizeof(displayBuffer), "%-16s", currentMessage);
      lcd.print(displayBuffer);
    }
    return;
  }

  // Add padding for scroll effect
  char paddedMessage[128];
  int paddedLen = 0;
  
  // Add 16 spaces at start
  for (int i = 0; i < 16 && paddedLen < 127; i++) {
    paddedMessage[paddedLen++] = ' ';
  }
  
  // Add message
  for (int i = 0; i < msgLen && paddedLen < 127; i++) {
    paddedMessage[paddedLen++] = currentMessage[i];
  }
  
  // Add space at end
  if (paddedLen < 127) {
    paddedMessage[paddedLen++] = ' ';
  }
  paddedMessage[paddedLen] = '\0';

  // Scroll logic
  if (millis() - lastScrollTime > delayTime) {
    lastScrollTime = millis();
    lcd.setCursor(0, row);
    
    char displayBuffer[17];
    int copyLen = (scrollPos + 16 <= paddedLen) ? 16 : paddedLen - scrollPos;
    memcpy(displayBuffer, paddedMessage + scrollPos, copyLen);
    
    // Pad with spaces if needed
    for (int i = copyLen; i < 16; i++) {
      displayBuffer[i] = ' ';
    }
    displayBuffer[16] = '\0';
    
    lcd.print(displayBuffer);
    scrollPos++;
    if (scrollPos > paddedLen - 16) {
      scrollPos = 0;
    }
  }
}

void modeDisplayWarehouse() {
  static bool displayInitialized = false;

  // Reset flag jika diminta
  if (resetDisplayRequested) {
    displayInitialized = false;
  }

  // Hanya update baris pertama sekali saja
  if (!displayInitialized) {
    lcd.setCursor(0, 0);
    lcd.print("Mode: Warehouse     ");  // Tambah spasi untuk clear sisa karakter
    displayInitialized = true;
  }
  scrollText(1, "Tekan Start untuk jalan", 500);
}

void modeDisplayMoveForward() {
  scrollText(0, "Mode : Move Forward", 500);
  displaySensorData();
}

void modeDisplayTerminalPickup(bool hookIsUp) {
  static bool lastHookIsUp = false;
  static AgvState lastCurrentStateAgv = AGV_STATE_STOP;
  static bool displayInitialized = false;

  // Reset flag jika diminta
  if (resetDisplayRequested) {
    displayInitialized = false;
  }

  // Hanya update display jika ada perubahan atau belum diinisialisasi
  if (!displayInitialized || hookIsUp != lastHookIsUp || currentStateAgv != lastCurrentStateAgv) {
    lcd.setCursor(0, 0);
    lcd.print("Mode: Pickup        ");  // Tambah spasi untuk clear sisa karakter
    lcd.setCursor(0, 1);
    if (!hookIsUp) {
      if (currentStateAgv == AGV_STATE_NULL) {
        lcd.print("Start u/ Naikkan    ");
      } else {
        lcd.print("Hook naik auto..    ");
      }
    } else {
      lcd.print("START untuk jln     ");
    }

    // Update state tracking
    lastHookIsUp = hookIsUp;
    lastCurrentStateAgv = currentStateAgv;
    displayInitialized = true;
  }
}

void modeDisplayTerminalDrop() {
  lcd.setCursor(0, 0);
  scrollText(0, "Mode: Terminal Drop", 500);
  scrollText(1, "Tekan Start untuk jalan", 500);
}

void modeDisplayStation() {
  lcd.setCursor(0, 0);
  scrollText(0, "Mode: Station", 500);
  scrollText(1, "Tekan Start untuk jalan", 500);
}

void displaySensorData() {
  // Rate limiting - update LCD max 10 Hz (every 100ms)
  static unsigned long lastDisplayUpdate = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastDisplayUpdate < 100) {
    return;  // Skip update if less than 100ms since last update
  }
  lastDisplayUpdate = currentTime;
  
  // Display sensor data on LCD (16 sensors in 2 rows)
  lcd.setCursor(0, 1);
  lcd.print("Sensor Magnet ");
  if (getCurrentMagnetSlaveId() == SLAVEID_MAGNET_DEPAN) {
    lcd.print("F");
  } else {
    lcd.print("B");
  }

  // Display magnet sensor status: 1 = detected, 0 = not detected
  lcd.setCursor(0, 2);
  // Menampilkan dari kanan ke kiri (sensor 15, 14, 13, ... 0)
  for (int i = 15; i >= 0; i--) {
    if (jumlahMagnet[i] == 1) {
      lcd.print("1");
    } else {  
      lcd.print("0");
    } 
  }

  // Show error value
  lcd.setCursor(0, 3);
  lcd.print("Error: ");
  lcd.print(errorValue);
  lcd.print("   ");
}
