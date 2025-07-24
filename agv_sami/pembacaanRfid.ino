void loopRfid() {
  // Only process RFID when in RFID Settings menu or in Station mode
  if (!(currentMenu == MENU_RFID_SETTINGS || modeStation)) {
    return;
  }
  
  noInterrupts();
  wiegand.flush();
  interrupts();
  //Sleep a little -- this doesn't have to run very often.
  // delay(100);
}
void pinStateChanged() {
  wiegand.setPin0State(digitalRead(PIN_D0));
  wiegand.setPin1State(digitalRead(PIN_D1));
}

// Notifies when a reader has been connected or disconnected.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onStateChange()`
void stateChanged(bool plugged, const char* message) {
  // Serial.print(message);
  // Serial.println(plugged ? "CONNECTED" : "DISCONNECTED");
}

// Notifies when a card was read.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onReceive()`
void receivedData(uint8_t* data, uint8_t bits, const char* message) {
  // Serial.print(message);
  // Serial.print(bits);
  // Serial.print("bits / ");
  
  // Convert RFID data to string for storage
  String rfidString = "";
  uint8_t bytes = (bits + 7) / 8;
  for (int i = 0; i < bytes; i++) {
    if (data[i] >> 4 < 10) rfidString += "0";
    rfidString += String(data[i] >> 4, 16);
    if ((data[i] & 0xF) < 10) rfidString += "0";
    rfidString += String(data[i] & 0xF, 16);
  }
  rfidString.toUpperCase();
  
  // Store the scanned RFID for menu use
  lastScannedRfid = rfidString;
  newRfidScanned = true;
  
  //Print value in HEX
  for (int i = 0; i < bytes; i++) {
    // Serial.print(data[i] >> 4, 16);
    // Serial.print(data[i] & 0xF, 16);
  }
  // Serial.println();
  
  // Different feedback based on current mode
  if (currentMenu == MENU_RFID_SETTINGS) {
    // Serial.println("RFID Scanned for Settings: " + rfidString);
  } else if (modeStation) {
    // Serial.println("RFID Scanned at Station: " + rfidString);
    // Check if this RFID matches any configured station
    for (int i = 0; i < rfidStationCount; i++) {
      if (rfidStations[i].isActive && rfidStations[i].rfidId == rfidString) {
        // Serial.print("Matched Station ID: ");
      // Serial.println(rfidStations[i].stationId);
        break;
      }
    }
  }
}

// Notifies when an invalid transmission is detected
void receivedDataError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message) {
  // Serial.print(message);
  // Serial.print(Wiegand::DataErrorStr(error));
  // Serial.print(" - Raw data: ");
  // Serial.print(rawBits);
  // Serial.print("bits / ");

  //Print value in HEX
  uint8_t bytes = (rawBits + 7) / 8;
  for (int i = 0; i < bytes; i++) {
    // Serial.print(rawData[i] >> 4, 16);
    // Serial.print(rawData[i] & 0xF, 16);
  }
  // Serial.println();
  
  // Count RFID errors
  static int rfidErrorCount = 0;
  rfidErrorCount++;
  
  // If too many errors, log it
  if (rfidErrorCount >= 10) {
    logError(ERROR_RFID_COMMUNICATION, "RFID error 10x berturut");
    rfidErrorCount = 0; // Reset counter
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
  
  // Serial.println("Loaded RFID stations:");
  for (int i = 0; i < rfidStationCount; i++) {
    if (rfidStations[i].isActive) {
      // Serial.print("Station ");
      // Serial.print(rfidStations[i].stationId);
      // Serial.print(": ");
      // Serial.println(rfidStations[i].rfidId);
    }
  }
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
  // Serial.println("RFID stations saved successfully");
}

int findRfidStation(int stationId) {
  for (int i = 0; i < rfidStationCount; i++) {
    if (rfidStations[i].stationId == stationId && rfidStations[i].isActive) {
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
  
  return false; // No space available
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
  
  // Serial.println("All RFID stations cleared");
}

// Function to check if current RFID matches a station and return station ID
int getStationFromLastRfid() {
  if (lastScannedRfid.length() == 0 || !newRfidScanned) {
    return -1; // No RFID scanned
  }
  
  for (int i = 0; i < rfidStationCount; i++) {
    if (rfidStations[i].isActive && rfidStations[i].rfidId == lastScannedRfid) {
      newRfidScanned = false; // Reset flag to prevent repeated processing
      return rfidStations[i].stationId;
    }
  }
  
  newRfidScanned = false; // Reset flag even if no match found
  return 0; // RFID scanned but no matching station found
}