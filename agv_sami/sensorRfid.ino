void loopRfid() {
  // Allow RFID processing in RFID settings menu OR when AGV mode is active
  if (!(currentMenu == MENU_RFID_SETTINGS || isAgvMode || MENU_RFID_UJUNG || MENU_RFID_WAREHOUSE || MENU_AUTO_INPUT_STATION || MENU_TERMINAL_DROP || MENU_TERMINAL_PICKUP)) {
    return;
  }
  noInterrupts();
  wiegand.flush();
  interrupts();
}

void IRAM_ATTR pinStateChanged() {
  // Optimized interrupt handler - minimal operations only
  static unsigned long lastInterruptTime = 0;
  unsigned long currentTime = micros();
  
  // Increased debounce time to reduce interrupt frequency
  if (currentTime - lastInterruptTime > 100) { // 200 microseconds debounce
    wiegand.setPin0State(digitalRead(PIN_D0));
    wiegand.setPin1State(digitalRead(PIN_D1));
    lastInterruptTime = currentTime;
  }
}

// Notifies when a reader has been connected or disconnected.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onStateChange()`
void stateChanged(bool plugged, const char* message) {
  Serial.print(message);
  Serial.println(plugged ? "CONNECTED" : "DISCONNECTED");
}

// Notifies when a card was read.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onReceive()`
void receivedData(uint8_t* data, uint8_t bits, const char* message) {
  static unsigned long lastRfidTime = 0;
  unsigned long currentTime = millis();
  
  // Increased debounce time to reduce processing load
  if (currentTime - lastRfidTime < 200) {
    return;
  }
  lastRfidTime = currentTime;
  
  // Optimized: Minimal processing in interrupt context
  // Store raw data for processing in main loop
  uint8_t bytes = (bits + 7) / 8;
  
  // Quick hex conversion with lookup table for better performance
  static const char hexLookup[] = "0123456789ABCDEF";
  int bufferIndex = 0;
  
  // Limit processing to prevent interrupt conflicts
  for (int i = 0; i < bytes && i < 15 && bufferIndex < 30; i++) {
    lastScannedRfidOptimized[bufferIndex++] = hexLookup[data[i] >> 4];
    lastScannedRfidOptimized[bufferIndex++] = hexLookup[data[i] & 0xF];
  }
  lastScannedRfidOptimized[bufferIndex] = '\0';
  
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
  preferences.begin("rfid-stations", false);

  rfidStationCount = preferences.getInt("stationCount", 0);

  for (int i = 0; i < rfidStationCount && i < MAX_RFID_STATIONS; i++) {
    char stationKey[20], rfidKey[20];
    sprintf(stationKey, "station%d", i);
    sprintf(rfidKey, "rfid%d", i);

    rfidStations[i].stationId = preferences.getInt(stationKey, 0);
    rfidStations[i].rfidId = preferences.getString(rfidKey, "");
    rfidStations[i].isActive = (rfidStations[i].stationId > 0 && rfidStations[i].rfidId.length() > 0);
  }

  preferences.end();
}

void saveRfidStations() {
  preferences.begin("rfid-stations", false);

  preferences.putInt("stationCount", rfidStationCount);

  for (int i = 0; i < rfidStationCount && i < MAX_RFID_STATIONS; i++) {
    char stationKey[20], rfidKey[20];
    sprintf(stationKey, "station%d", i);
    sprintf(rfidKey, "rfid%d", i);

    preferences.putInt(stationKey, rfidStations[i].stationId);
    preferences.putString(rfidKey, rfidStations[i].rfidId);
  }

  preferences.end();
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

  Serial.println("All RFID stations cleared");
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
