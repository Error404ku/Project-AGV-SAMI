void loopRfid() {
  // Allow RFID processing in RFID settings menu OR when AGV mode is active
  if (!(currentMenu == MENU_RFID_SETTINGS || isAgvMode || MENU_RFID_UJUNG || MENU_RFID_WAREHOUSE || MENU_AUTO_INPUT_STATION || MENU_TERMINAL_DROP || MENU_TERMINAL_PICKUP)) {
    return;
  }

  // Feed watchdog to prevent reset
  // esp_task_wdt_reset();
  
  noInterrupts();
  wiegand.flush();
  interrupts();
  
  // Process any pending RFID data immediately
  if (newRfidScanned && strlen(lastScannedRfidOptimized) > 0) {
    // Data RFID sudah tersimpan di lastScannedRfidOptimized oleh receivedData()
    // Flag newRfidScanned sudah di-set true
    // Pemrosesan akan dilakukan oleh logika AGV atau menu yang memanggil
    // Tidak perlu reset flag di sini karena akan di-reset oleh pemroses
  }
}

void pinStateChanged() {
  // Keep interrupt handler as minimal as possible
  static unsigned long lastInterruptTime = 0;
  unsigned long currentTime = micros();
  
  if (currentTime - lastInterruptTime > 100) { // 100 microseconds debounce
    wiegand.setPin0State(digitalRead(PIN_D0));
    wiegand.setPin1State(digitalRead(PIN_D1));
    lastInterruptTime = currentTime;
  }
}

// Notifies when a reader has been connected or disconnected.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onStateChange()`
void stateChanged(bool plugged, const char* message) {
  // Serial.print() - removed for production
  // Serial.println() - removed for production
}

// Notifies when a card was read.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onReceive()`
void receivedData(uint8_t* data, uint8_t bits, const char* message) {
  static unsigned long lastRfidTime = 0;
  unsigned long currentTime = millis();
  
  // Debounce: ignore RFID reads within 500ms
  if (currentTime - lastRfidTime < 300) {
    return;
  }
  lastRfidTime = currentTime;
  
  // Minimize serial prints in interrupt context
  // Move heavy processing to main loop
  
  // Only essential processing here
  char rfidBuffer[32] = "";
  uint8_t bytes = (bits + 7) / 8;
  int bufferIndex = 0;

  for (int i = 0; i < bytes && bufferIndex < 30; i++) {
    char hexChar1 = (data[i] >> 4) < 10 ? '0' + (data[i] >> 4) : 'A' + (data[i] >> 4) - 10;
    char hexChar2 = (data[i] & 0xF) < 10 ? '0' + (data[i] & 0xF) : 'A' + (data[i] & 0xF) - 10;
    rfidBuffer[bufferIndex++] = hexChar1;
    rfidBuffer[bufferIndex++] = hexChar2;
  }
  rfidBuffer[bufferIndex] = '\0';

  strncpy(lastScannedRfidOptimized, rfidBuffer, sizeof(lastScannedRfidOptimized) - 1);
  lastScannedRfidOptimized[sizeof(lastScannedRfidOptimized) - 1] = '\0';
  newRfidScanned = true;
}

// Notifies when an invalid transmission is detected
void receivedDataError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message) {
  // Count RFID errors silently
  static int rfidErrorCount = 0;
  rfidErrorCount++;

  // If too many errors, log it
  if (rfidErrorCount >= 10) {
    logError(ERROR_RFID_COMMUNICATION, "RFID error 10x berturut");
    rfidErrorCount = 0;  // Reset counter
  }
}

// RFID Station Management Functions
void loadRfidStations() {
  // Load from first namespace (stations 0-39)
  preferences.begin("rfid-stations", false);
  rfidStationCount = preferences.getInt("stationCount", 0);

  // Limit to 40 for first namespace
  int firstBatchCount = (rfidStationCount > 40) ? 40 : rfidStationCount;
  
  for (int i = 0; i < firstBatchCount; i++) {
    char stationKey[20], rfidKey[20];
    sprintf(stationKey, "station%d", i);
    sprintf(rfidKey, "rfid%d", i);

    rfidStations[i].stationId = preferences.getInt(stationKey, 0);
    rfidStations[i].rfidId = preferences.getString(rfidKey, "");
    rfidStations[i].isActive = (rfidStations[i].stationId > 0 && rfidStations[i].rfidId.length() > 0);
  }
  preferences.end();

  // Load from second namespace (stations 40-49)
  preferences.begin("rfid-stations-2", false);
  int secondBatchCount = preferences.getInt("stationCount", 0);
  
  for (int i = 0; i < secondBatchCount && (40 + i) < MAX_RFID_STATIONS; i++) {
    char stationKey[20], rfidKey[20];
    sprintf(stationKey, "station%d", i);
    sprintf(rfidKey, "rfid%d", i);

    int arrayIndex = 40 + i;
    rfidStations[arrayIndex].stationId = preferences.getInt(stationKey, 0);
    rfidStations[arrayIndex].rfidId = preferences.getString(rfidKey, "");
    rfidStations[arrayIndex].isActive = (rfidStations[arrayIndex].stationId > 0 && rfidStations[arrayIndex].rfidId.length() > 0);
  }
  preferences.end();

  // Update total count
  if (rfidStationCount < 40 && secondBatchCount > 0) {
    rfidStationCount = 40 + secondBatchCount;
  } else if (rfidStationCount >= 40) {
    rfidStationCount = firstBatchCount + secondBatchCount;
  }
}

void saveRfidStations() {
  // Save to first namespace (stations 0-39)
  preferences.begin("rfid-stations", false);
  
  int firstBatchCount = (rfidStationCount > 40) ? 40 : rfidStationCount;
  preferences.putInt("stationCount", firstBatchCount);

  for (int i = 0; i < firstBatchCount; i++) {
    char stationKey[20], rfidKey[20];
    sprintf(stationKey, "station%d", i);
    sprintf(rfidKey, "rfid%d", i);

    preferences.putInt(stationKey, rfidStations[i].stationId);
    preferences.putString(rfidKey, rfidStations[i].rfidId);
  }
  preferences.end();

  // Save to second namespace (stations 40-49)
  if (rfidStationCount > 40) {
    preferences.begin("rfid-stations-2", false);
    
    int secondBatchCount = rfidStationCount - 40;
    preferences.putInt("stationCount", secondBatchCount);

    for (int i = 0; i < secondBatchCount && (40 + i) < MAX_RFID_STATIONS; i++) {
      char stationKey[20], rfidKey[20];
      sprintf(stationKey, "station%d", i);
      sprintf(rfidKey, "rfid%d", i);

      int arrayIndex = 40 + i;
      preferences.putInt(stationKey, rfidStations[arrayIndex].stationId);
      preferences.putString(rfidKey, rfidStations[arrayIndex].rfidId);
    }
    preferences.end();
  }
}

int findRfidStation(int stationId) {
  for (int i = 0; i < rfidStationCount; i++) {
    if (rfidStations[i].stationId == stationId && rfidStations[i].isActive) {
      return i;
    }
  }
  return -1;
}

// Function to find RFID station by RFID ID string
int findRfidStationByRfidId(String rfidId) {
  for (int i = 0; i < rfidStationCount; i++) {
    if (rfidStations[i].rfidId == rfidId && rfidStations[i].isActive) {
      return i;
    }
  }
  return -1;
}

bool addRfidStation(int stationId, String rfidId) {
  // Check if station already exists
  int existingIndex = findRfidStation(stationId);
  if (existingIndex >= 0) {
    // Update existing station
    rfidStations[existingIndex].rfidId = rfidId;
    rfidStations[existingIndex].isActive = true;
    saveRfidStations();
    return true;
  }

  // Add new station if we have space
  if (rfidStationCount < MAX_RFID_STATIONS) {
    rfidStations[rfidStationCount].stationId = stationId;
    rfidStations[rfidStationCount].rfidId = rfidId;
    rfidStations[rfidStationCount].isActive = true;
    rfidStationCount++;
    saveRfidStations();
    return true;
  }

  return false;  // No space available
}

bool deleteRfidStation(int stationId) {
  int index = findRfidStation(stationId);
  if (index >= 0) {
    // Shift remaining stations
    for (int i = index; i < rfidStationCount - 1; i++) {
      rfidStations[i] = rfidStations[i + 1];
    }
    rfidStationCount--;

    // Clear the last station
    rfidStations[rfidStationCount].stationId = 0;
    rfidStations[rfidStationCount].rfidId = "";
    rfidStations[rfidStationCount].isActive = false;

    saveRfidStations();
    
    // Clear second namespace if count drops to 40 or below
    if (rfidStationCount <= 40) {
      preferences.begin("rfid-stations-2", false);
      preferences.clear();
      preferences.end();
    }
    
    return true;
  }
  return false;
}

String getRfidForStation(int stationId) {
  int index = findRfidStation(stationId);
  if (index >= 0) {
    return rfidStations[index].rfidId;
  }
  return "";
}

void clearAllRfidStations() {
  preferences.begin("rfid-stations", false);
  preferences.clear();
  preferences.end();

  for (int i = 0; i < MAX_RFID_STATIONS; i++) {
    rfidStations[i].stationId = 0;
    rfidStations[i].rfidId = "";
    rfidStations[i].isActive = false;
  }
  rfidStationCount = 0;

  // Serial.println() - removed for production
}

// Function to check if current RFID matches a station and return station ID (optimized)
int getStationFromLastRfid(){
  if (strlen(lastScannedRfidOptimized) == 0 || !newRfidScanned) {
    return -1;  // No RFID scanned
  }

  for (int i = 0; i < rfidStationCount; i++) {
    if (rfidStations[i].isActive && rfidStations[i].rfidId.equals(lastScannedRfidOptimized)) {
      newRfidScanned = false;  // Reset flag when station match found
      return rfidStations[i].stationId;
    }
  }

  // Reset flag jika tidak ada station yang cocok untuk mencegah
  // RFID yang tidak dikenal mempengaruhi scan berikutnya
  newRfidScanned = false;
  return 0;                // RFID scanned but no matching station found
}
