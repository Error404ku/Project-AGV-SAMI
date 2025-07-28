/*
  RFID.INO - RFID Management and Station Control
  
  This file combines all RFID-related functions:
  - RFID reading and processing
  - Station management
  - Wiegand protocol handling
  - RFID-station association
*/

#include <algorithm>

// ==================== RFID READING FUNCTIONS ====================

void loopRfid() {
  // Only process RFID when in RFID setup menu or station mode
  if (currentMenuState == MENU_RFID_SETUP || systemState.currentMode == MODE_STATION) {
    // RFID processing is handled by Wiegand interrupts
    // This function is kept for compatibility
  }
}

// Wiegand callback functions
void pinStateChanged() {
  // Update Wiegand pin states
  wiegand.setPin0State(digitalRead(WIEGAND_D0));
  wiegand.setPin1State(digitalRead(WIEGAND_D1));
}

void stateChanged(bool plugged, const char* message) {
  // Handle Wiegand reader connection/disconnection
  DEBUG_PRINTF("Wiegand reader %s: %s\n", plugged ? "connected" : "disconnected", message);
  
  if (plugged) {
    systemState.rfidReaderConnected = true;
    music("connect");
  } else {
    systemState.rfidReaderConnected = false;
    music("disconnect");
  }
}

void receivedData(uint8_t* data, uint8_t bits, const char* type) {
  // Process received RFID data
  DEBUG_PRINTF("Wiegand received %d bits of type %s\n", bits, type);
  
  // Convert data to string
  String rfidCode = "";
  for (int i = 0; i < (bits / 8); i++) {
    if (data[i] < 16) rfidCode += "0";
    rfidCode += String(data[i], HEX);
  }
  rfidCode.toUpperCase();
  
  DEBUG_PRINTF("RFID Code: %s\n", rfidCode.c_str());
  
  // Store the last scanned RFID
  lastScannedRfid = rfidCode;
  lastRfidScanTime = millis();
  
  // If in station mode, try to match with configured stations
  if (systemState.currentMode == MODE_STATION) {
    processStationRfid(rfidCode);
  }
  
  // Play confirmation sound
  music("rfid_scan");
}

void receivedDataError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* type) {
  // Handle RFID transmission errors
  DEBUG_PRINTF("Wiegand error %d for %d bits of type %s\n", (int)error, rawBits, type);
  
  // Count errors for diagnostics
  static int errorCount = 0;
  errorCount++;
  
  if (errorCount > 10) {
    logError(ERROR_RFID_COMMUNICATION, "Too many RFID errors: " + String(errorCount));
    errorCount = 0;
  }
  
  music("error");
}

void processStationRfid(String rfidCode) {
  // Find station associated with this RFID
  int stationId = findRfidStation(rfidCode);
  
  if (stationId > 0) {
    DEBUG_PRINTF("RFID matched station %d\n", stationId);
    
    // Update current station
    currentStationId = stationId;
    
    // Execute station-specific actions
    executeStationAction(stationId);
    
    // Update LCD display
    displayMessage("Station Found", "ID: " + String(stationId), 2000);
    
    music("station_found");
  } else {
    DEBUG_PRINTLN("RFID not associated with any station");
    displayMessage("Unknown RFID", rfidCode.substring(0, 16), 2000);
    music("error");
  }
}

void executeStationAction(int stationId) {
  // Execute actions based on station ID
  switch (stationId) {
    case 1:
      // Station 1: Stop and lower hook
      stopMovement();
      setHookDown();
      DEBUG_PRINTLN("Station 1: Stop and lower hook");
      break;
      
    case 2:
      // Station 2: Raise hook and continue
      setHookUp();
      DEBUG_PRINTLN("Station 2: Raise hook and continue");
      break;
      
    case 3:
      // Station 3: Turn around
      stopMovement();
      delay(1000);
      turnRight(systemConfig.baseSpeed);
      delay(2000);
      stopMovement();
      DEBUG_PRINTLN("Station 3: Turn around");
      break;
      
    default:
      // Default action: stop for 2 seconds
      stopMovement();
      delay(2000);
      DEBUG_PRINTF("Station %d: Default stop action\n", stationId);
      break;
  }
}

// ==================== STATION MANAGEMENT ====================

void loadRfidStations() {
  // Load RFID stations from Preferences
  preferences.begin(PREF_NAMESPACE_RFID, true);
  
  size_t dataSize = preferences.getBytesLength(PREF_KEY_RFID_STATIONS);
  if (dataSize > 0 && dataSize <= sizeof(rfidStations)) {
    preferences.getBytes(PREF_KEY_RFID_STATIONS, rfidStations, dataSize);
    DEBUG_PRINTF("Loaded %d bytes of RFID station data\n", dataSize);
  } else {
    DEBUG_PRINTLN("No RFID station data found, initializing empty");
    memset(rfidStations, 0, sizeof(rfidStations));
  }
  
  preferences.end();
  
  // Count loaded stations
  int count = 0;
  for (int i = 0; i < MAX_RFID_STATIONS; i++) {
    if (rfidStations[i].stationId != 0) {
      count++;
    }
  }
  
  DEBUG_PRINTF("Loaded %d RFID stations\n", count);
}

void saveRfidStations() {
  // Save RFID stations to Preferences
  preferences.begin(PREF_NAMESPACE_RFID, false);
  
  size_t written = preferences.putBytes(PREF_KEY_RFID_STATIONS, rfidStations, sizeof(rfidStations));
  
  if (written == sizeof(rfidStations)) {
    DEBUG_PRINTLN("RFID stations saved successfully");
  } else {
    logError(ERROR_STORAGE, "Failed to save RFID stations");
  }
  
  preferences.end();
}

int findRfidStation(String rfidCode) {
  // Find station ID for given RFID code
  for (int i = 0; i < MAX_RFID_STATIONS; i++) {
    if (rfidStations[i].stationId != 0 && rfidStations[i].rfidCode == rfidCode) {
      return rfidStations[i].stationId;
    }
  }
  return 0; // Not found
}

bool addRfidStation(int stationId, String rfidCode) {
  // Validate input
  if (stationId <= 0 || stationId > MAX_STATIONS || rfidCode.length() == 0) {
    DEBUG_PRINTLN("Invalid station ID or RFID code");
    return false;
  }
  
  // Check if RFID is already associated with another station
  int existingStation = findRfidStation(rfidCode);
  if (existingStation > 0 && existingStation != stationId) {
    DEBUG_PRINTF("RFID already associated with station %d\n", existingStation);
    return false;
  }
  
  // Find existing entry or empty slot
  int targetIndex = -1;
  
  // First, look for existing entry with same station ID
  for (int i = 0; i < MAX_RFID_STATIONS; i++) {
    if (rfidStations[i].stationId == stationId) {
      targetIndex = i;
      break;
    }
  }
  
  // If not found, look for empty slot
  if (targetIndex == -1) {
    for (int i = 0; i < MAX_RFID_STATIONS; i++) {
      if (rfidStations[i].stationId == 0) {
        targetIndex = i;
        break;
      }
    }
  }
  
  if (targetIndex == -1) {
    DEBUG_PRINTLN("No space for new RFID station");
    return false;
  }
  
  // Add/update the station
  rfidStations[targetIndex].stationId = stationId;
  rfidStations[targetIndex].rfidCode = rfidCode;
  
  // Save to storage
  saveRfidStations();
  
  DEBUG_PRINTF("Added RFID station: ID=%d, RFID=%s\n", stationId, rfidCode.c_str());
  return true;
}

bool deleteRfidStation(int stationId) {
  // Find and delete station
  bool found = false;
  
  for (int i = 0; i < MAX_RFID_STATIONS; i++) {
    if (rfidStations[i].stationId == stationId) {
      rfidStations[i].stationId = 0;
      rfidStations[i].rfidCode = "";
      found = true;
      DEBUG_PRINTF("Deleted RFID station %d\n", stationId);
    }
  }
  
  if (found) {
    saveRfidStations();
  }
  
  return found;
}

String getRfidForStation(int stationId) {
  // Get RFID code for given station ID
  for (int i = 0; i < MAX_RFID_STATIONS; i++) {
    if (rfidStations[i].stationId == stationId) {
      return rfidStations[i].rfidCode;
    }
  }
  return ""; // Not found
}

void clearAllRfidStations() {
  // Clear all RFID stations
  memset(rfidStations, 0, sizeof(rfidStations));
  saveRfidStations();
  DEBUG_PRINTLN("All RFID stations cleared");
}

void loadStationsListFromPreferences() {
  // Load stations list from preferences
  preferences.begin(PREF_NAMESPACE_STATIONS, false);
  
  size_t dataSize = preferences.getBytesLength("stationsList");
  if (dataSize > 0) {
    uint8_t* buffer = new uint8_t[dataSize];
    preferences.getBytes("stationsList", buffer, dataSize);
    
    // Parse buffer to stationsList
    stationsList.clear();
    size_t numStations = dataSize / sizeof(int);
    int* stationData = (int*)buffer;
    
    for (size_t i = 0; i < numStations; i++) {
      stationsList.push_back(stationData[i]);
    }
    
    delete[] buffer;
    DEBUG_PRINTF("Loaded %d stations from preferences\n", stationsList.size());
  } else {
    stationsList.clear();
    DEBUG_PRINTLN("No stations list found in preferences");
  }
  
  preferences.end();
}

void clearStationsData() {
  // Clear all station data
  stationsList.clear();
  jumlahStasiun = 0;
  currentStationId = 1;
  
  // Clear from preferences
  preferences.begin(PREF_NAMESPACE_STATIONS, false);
  preferences.clear();
  preferences.end();
  
  DEBUG_PRINTLN("All station data cleared");
}

void sortStationsList() {
  // Sort stations list in ascending order
  if (stationsList.size() > 1) {
    std::sort(stationsList.begin(), stationsList.end());
    DEBUG_PRINTLN("Stations list sorted");
  }
}

void listRfidStations() {
  // List all configured RFID stations
  DEBUG_PRINTLN("=== RFID STATIONS ===");
  
  int count = 0;
  for (int i = 0; i < MAX_RFID_STATIONS; i++) {
    if (rfidStations[i].stationId != 0) {
      DEBUG_PRINTF("Station %d: %s\n", rfidStations[i].stationId, rfidStations[i].rfidCode.c_str());
      count++;
    }
  }
  
  if (count == 0) {
    DEBUG_PRINTLN("No RFID stations configured");
  } else {
    DEBUG_PRINTF("Total: %d stations\n", count);
  }
  
  DEBUG_PRINTLN("=== END RFID STATIONS ===");
}

// ==================== HOOK CONTROL ====================

void setHookUp() {
  if (systemState.hookStatus != HOOK_UP) {
    DEBUG_PRINTLN("Setting hook UP");
    systemState.hookStatus = HOOK_MOVING;
    
    digitalWrite(HOOK_RELAY, HIGH);
    
    // Wait for limit switch or timeout
    unsigned long startTime = millis();
    while (millis() - startTime < HOOK_TIMEOUT_MS) {
      if (digitalRead(HOOK_LIMIT_UP) == HIGH) {
        systemState.hookStatus = HOOK_UP;
        DEBUG_PRINTLN("Hook UP position reached");
        break;
      }
      delay(10);
    }
    
    digitalWrite(HOOK_RELAY, LOW);
    
    if (systemState.hookStatus != HOOK_UP) {
      systemState.hookStatus = HOOK_UP; // Assume success even if limit switch not triggered
      DEBUG_PRINTLN("Hook UP timeout - assuming position reached");
    }
    
    music("hook_up");
  }
}

void setHookDown() {
  if (systemState.hookStatus != HOOK_DOWN) {
    DEBUG_PRINTLN("Setting hook DOWN");
    systemState.hookStatus = HOOK_MOVING;
    
    digitalWrite(HOOK_RELAY, LOW);
    
    // Wait for limit switch or timeout
    unsigned long startTime = millis();
    while (millis() - startTime < HOOK_TIMEOUT_MS) {
      if (digitalRead(HOOK_LIMIT_DOWN) == HIGH) {
        systemState.hookStatus = HOOK_DOWN;
        DEBUG_PRINTLN("Hook DOWN position reached");
        break;
      }
      delay(10);
    }
    
    if (systemState.hookStatus != HOOK_DOWN) {
      systemState.hookStatus = HOOK_DOWN; // Assume success even if limit switch not triggered
      DEBUG_PRINTLN("Hook DOWN timeout - assuming position reached");
    }
    
    music("hook_down");
  }
}

void toggleHook() {
  switch (systemState.hookStatus) {
    case HOOK_UP:
      setHookDown();
      break;
    case HOOK_DOWN:
      setHookUp();
      break;
    case HOOK_MOVING:
      DEBUG_PRINTLN("Hook is moving, cannot toggle");
      break;
    default:
      // Unknown state, try to go up
      setHookUp();
      break;
  }
}

void updateHookStatus() {
  // Update hook status based on limit switches
  static unsigned long lastCheck = 0;
  
  if (millis() - lastCheck < 100) {
    return; // Check every 100ms
  }
  lastCheck = millis();
  
  if (systemState.hookStatus != HOOK_MOVING) {
    if (digitalRead(HOOK_LIMIT_UP) == HIGH) {
      systemState.hookStatus = HOOK_UP;
    } else if (digitalRead(HOOK_LIMIT_DOWN) == HIGH) {
      systemState.hookStatus = HOOK_DOWN;
    }
  }
}

// ==================== MUSIC AND FEEDBACK ====================

void music(String type) {
  if (!systemConfig.musicEnabled) {
    return;
  }
  
  // Simple tone generation for different events
  if (type == "rfid_scan") {
    tone(MUSIC_PIN, 1000, 200);
  } else if (type == "station_found") {
    tone(MUSIC_PIN, 1500, 100);
    delay(150);
    tone(MUSIC_PIN, 2000, 100);
  } else if (type == "hook_up") {
    tone(MUSIC_PIN, 800, 300);
  } else if (type == "hook_down") {
    tone(MUSIC_PIN, 600, 300);
  } else if (type == "connect") {
    tone(MUSIC_PIN, 1200, 100);
    delay(120);
    tone(MUSIC_PIN, 1500, 100);
  } else if (type == "disconnect") {
    tone(MUSIC_PIN, 1500, 100);
    delay(120);
    tone(MUSIC_PIN, 1200, 100);
  } else if (type == "error") {
    tone(MUSIC_PIN, 400, 500);
  } else if (type == "startup") {
    tone(MUSIC_PIN, 1000, 100);
    delay(120);
    tone(MUSIC_PIN, 1200, 100);
    delay(120);
    tone(MUSIC_PIN, 1500, 200);
  }
  
  // LED feedback
  if (type == "error") {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
  }
}

void stopMusic() {
  // Stop all music/sound output
  digitalWrite(systemConfig.musicErrorPin, LOW);
  digitalWrite(systemConfig.musicDetectPin, LOW);
  digitalWrite(systemConfig.musicKomputerPin, LOW);
  digitalWrite(systemConfig.musicStationPin, LOW);
  
  statusMusic = false;
  DEBUG_PRINTLN("All music stopped");
}

// ==================== DIAGNOSTIC FUNCTIONS ====================

void testRfidSystem() {
  DEBUG_PRINTLN("=== RFID SYSTEM TEST ===");
  
  DEBUG_PRINTF("RFID Reader Connected: %s\n", systemState.rfidReaderConnected ? "YES" : "NO");
  DEBUG_PRINTF("Last Scanned RFID: %s\n", lastScannedRfid.c_str());
  DEBUG_PRINTF("Last Scan Time: %lu ms ago\n", millis() - lastRfidScanTime);
  
  listRfidStations();
  
  DEBUG_PRINTF("Current Station ID: %d\n", currentStationId);
  DEBUG_PRINTF("Total Stations: %d\n", jumlahStasiun);
  
  DEBUG_PRINTLN("=== END RFID TEST ===");
}

void simulateRfidScan(String rfidCode) {
  // Simulate RFID scan for testing
  DEBUG_PRINTF("Simulating RFID scan: %s\n", rfidCode.c_str());
  
  lastScannedRfid = rfidCode;
  lastRfidScanTime = millis();
  
  if (systemState.currentMode == MODE_STATION) {
    processStationRfid(rfidCode);
  }
  
  music("rfid_scan");
}

// ==================== COMPATIBILITY FUNCTIONS ====================

// These functions maintain compatibility with existing code
void setupRfid() {
  // RFID setup is now handled in setup.ino
  DEBUG_PRINTLN("RFID setup called (compatibility function)");
}

void bacaRfid() {
  // RFID reading is now handled by interrupts
  // This function is kept for compatibility
}

void prosesRfid() {
  // RFID processing is now handled in receivedData callback
  // This function is kept for compatibility
}