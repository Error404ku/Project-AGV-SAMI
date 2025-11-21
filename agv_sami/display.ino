// Global flag untuk mereset display
bool resetDisplayRequested = false;

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
    lcd.print("AGV Mode:           ");
    lcd.setCursor(0, 1);
    
    // ✅ OPTIMIZED: Direct const char* - already optimized, no need .c_str()
    const char* stateString = agvStateToString(currentStateAgv);
    lcd.print(stateString);
    lcd.print("                ");  // Clear remaining chars

    // Update state tracking
    lastDisplayedState = currentStateAgv;
    displayInitialized = true;
  }

  // Reset global flag setelah digunakan
  if (resetDisplayRequested) {
    resetDisplayRequested = false;
  }
}

struct ScrollState {
  char currentMessage[64];  // ✅ Fixed buffer instead of String
  char paddedMessage[96];   // ✅ Fixed buffer for padded text
  int scrollPos = 0;
  unsigned long lastScrollTime = 0;
};
ScrollState scrollLine0;
ScrollState scrollLine1;

// ✅ OPTIMIZED: Zero heap allocation scrolling
void scrollText(int row, const char* message, int delayTime) {
  ScrollState* state = (row == 0) ? &scrollLine0 : &scrollLine1;
  
  // Check if message changed
  if (strcmp(state->currentMessage, message) != 0) {
    strncpy(state->currentMessage, message, sizeof(state->currentMessage) - 1);
    state->currentMessage[sizeof(state->currentMessage) - 1] = '\0';
    
    // Build padded message with fixed buffer - NO HEAP
    int msgLen = strlen(message);
    int padLen = 16;
    
    // Add leading spaces
    memset(state->paddedMessage, ' ', padLen);
    // Copy message
    strncpy(state->paddedMessage + padLen, message, sizeof(state->paddedMessage) - padLen - 2);
    // Add trailing space
    state->paddedMessage[padLen + msgLen] = ' ';
    state->paddedMessage[padLen + msgLen + 1] = '\0';
    
    state->scrollPos = 0;
    state->lastScrollTime = millis();
  }

  // Scroll if delay elapsed
  if (millis() - state->lastScrollTime > delayTime) {
    state->lastScrollTime = millis();
    lcd.setCursor(0, row);
    
    // ✅ Direct print without substring - NO HEAP
    int totalLen = strlen(state->paddedMessage);
    for (int i = 0; i < 16 && (state->scrollPos + i) < totalLen; i++) {
      lcd.write(state->paddedMessage[state->scrollPos + i]);
    }
    
    state->scrollPos++;
    if (state->scrollPos > totalLen - 16) {
      state->scrollPos = 0;
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

void modeDisplayMoveBackward() {
  scrollText(0, "Mode : Move Backward", 500);
  displaySensorData();
}

/*
 * Hapus fungsi scrollText() Anda yang lama.
 * Ganti fungsi modeDisplayTerminalPickup() dengan yang ini.
 * Fungsi ini menerima parameter 'hookIsUp' untuk menentukan teks yang ditampilkan.
*/
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
