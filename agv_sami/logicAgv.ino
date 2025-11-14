// This function handles the AGV mode based on the provided state.
void agvMode(AgvState state) {
  switch (state) {
    case AGV_STATE_MOVE_FORWARD:
      agvMoveForward();
      break;
    case AGV_STATE_STOP:
      static bool stopCalled = false;
        if (!stopCalled) {
          agvStop();
          stopCalled = true;
        }
      break;
    case AGV_STATE_TERMINAL_DROP:
      agvTerminalDrop();
      break;
    case AGV_STATE_TERMINAL_PICKUP:
      agvTerminalPickup();
      break;
    case AGV_STATE_WAREHOUSE:
      agvWarehouse();
      break;
    case AGV_STATE_STATION:
      agvStation();
      break;
    default:
      agvStop();
      break;
  }
}

// Flag for resetting warehouse state
static bool warehouseNeedReset = false;

// Function to reset warehouse state when loading from preferences
void resetWarehouseState() {
  // Set global flag so agvWarehouse() performs reset when called next time
  // This is needed to ensure AGV doesn't start moving immediately after restart
  warehouseNeedReset = true;
}

// Function to handle AGV logic when in warehouse
void agvWarehouse() {
  static bool trigger = false;
  static bool showingErrorMessage = false;
  static unsigned long errorMessageStartTime = 0;
  static bool needsDisplayRefresh = false;
  
  // Reset trigger when first called after restart/load state
  if (warehouseNeedReset) {
    trigger = false;
    showingErrorMessage = false;
    needsDisplayRefresh = false;
    warehouseNeedReset = false;  // Reset only once
  }
  
  saveCurrentStateAGVToPreferences(AGV_STATE_WAREHOUSE);

  // Handle error message display timing
  if (showingErrorMessage) {
    if (millis() - errorMessageStartTime >= 2000) {
      showingErrorMessage = false;
      needsDisplayRefresh = true; // Force display refresh after error
    } else {
      // Keep showing error message and block other actions
      return;
    }
  }

  // If START button is pressed and there are target stations in the list, trigger the AGV to move
  if (START()) {
    if (targetStationsList.size() == 0){
      // Show "no station" error message for 2 seconds
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tidak ada station");
      lcd.setCursor(0, 1);
      lcd.print("di daftar target"); // Shortened to fit 16 chars
      showingErrorMessage = true;
      errorMessageStartTime = millis();
      return; // Return without starting movement
    } else { // There are target stations to move
      trigger = true;
    }
  }
  if (!trigger) { // Handle movement trigger (trigger from START button on the top)
    agvStop();
    updatestations = true;
    // Force refresh display if needed
    if (needsDisplayRefresh) {
      lcd.clear(); // Clear screen first
      resetDisplayRequested = true; // Force reset display flags
      needsDisplayRefresh = false;
    }
    modeDisplayWarehouse();
  }else{
    stopCalledPickup = false; // Reset flag for next use
    updatestations = false;
    agvMode(AGV_STATE_MOVE_FORWARD);
    trigger = false;
    return;
  }
}

// Function to handle AGV logic when at a station
void agvStation() {
  bool trigger = false;
  saveCurrentStateAGVToPreferences(AGV_STATE_STATION);
  modeDisplayStation();
  static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();
  agvStop();

  if (lastReadTime == 0) {
    lastReadTime = currentTime;
  }
  if (currentTime - lastReadTime > 20000) {
    music(MUSIC_MODE_WARNING);
  } else {
    music(MUSIC_MODE_STATION);
  }
  // Check if START button is pressed
  if (START()) {
    lastReadTime = 0;
    trigger = true;
  }
  if (trigger) { // Handle movement trigger (trigger from START button on the top)
    // Check if there are still stations in the list
    if (targetStationsList.size() == 0) {
      // No more stations left
      agvMode(AGV_STATE_MOVE_FORWARD);
    } else {
      // Still stations left, continue moving forward
      agvMode(AGV_STATE_MOVE_FORWARD);
    }
    trigger = false; // Reset trigger
    return;
  }
}

// Fungsi ini menangani logika AGV saat melakukan penurunan di terminal.
void agvTerminalDrop() {
  static int dropProcessStep = 0; // This variable tracks the current step of the drop process
  saveCurrentStateAGVToPreferences(AGV_STATE_TERMINAL_DROP); // Save state to preferences
  modeDisplayTerminalDrop();
  music(MUSIC_MODE_ON);

  // Process drop steps
  switch (dropProcessStep) {
    case 0: 
      dropProcessStep = 1;
      agvStop();
      break;
    case 1: 
      hookPosition = hook(DOWN_HOOK);
      if (hookPosition == DOWN_POS) {
        delay(5000);
        dropProcessStep = 2; 
      }
      break;
    case 2: 
      dropProcessStep = 0; 
      agvMode(AGV_STATE_MOVE_FORWARD);
      break;
  }
}

// Function to handle AGV logic when performing terminal pickup
void agvTerminalPickup() {
  modeDisplayTerminalPickup(isHookUp);
  static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();
  
  if (currentStateAgv == AGV_STATE_NULL){
    isHookUp = false;
  } else {
    saveCurrentStateAGVToPreferences(AGV_STATE_TERMINAL_PICKUP);
  }
  if (!stopCalledPickup) {
    agvStop();
    delay(2000);
    stopCalledPickup = true;
  }
  if (!isHookUp) {
    currentRFID = AGV_STATE_TERMINAL_PICKUP;
    exceptErrorPosition = false;
    // Trigger hook up if: otomatis (ada state) ATAU manual (tekan START)
    bool shouldRaiseHook = (currentStateAgv != AGV_STATE_NULL) || 
                          (currentStateAgv == AGV_STATE_NULL && START());
    if (shouldRaiseHook) {
      saveCurrentStateAGVToPreferences(AGV_STATE_TERMINAL_PICKUP);
      hookPosition = hook(UP_HOOK);
      if (hookPosition == UP_POS) {
        isHookUp = true;
      }
    }
  } else {
    music(MUSIC_MODE_WARNING);
    if (START()) {
      lastReadTime = currentTime;
      stopCalledPickup = false;
      agvMode(AGV_STATE_MOVE_FORWARD);
    }
  }
}

// Function to stop AGV movement
void agvStop() {
  rpmMotor(0, 0);
  Serial.println("STOP");  // Use PWM stop command
  return;
}

// Function to handle AGV logic when moving forward
void agvMoveForward() {
  // Initialize soft start on first entry to MOVE_FORWARD state
  static bool needsSoftStart = true;
  if (needsSoftStart) {
    softStartTime = millis();
    softStartActive = true;
    pidSpeed = maxMotorRpm / 2;
    needsSoftStart = false;
  }
  
  saveCurrentStateAGVToPreferences(AGV_STATE_MOVE_FORWARD);
  checkObstacles();
  modeDisplayMoveForward();
  // Check RFID detected for mode switching (must be done before getStationFromLastRfid)
  const char* currentRfid = lastScannedRfidOptimized;
  unsigned long currentTime = millis();
  
  // Detect End RFID with 2-second Debounce
  if (strlen(currentRfid) > 0) {
    if (strcmp(currentRfid, ujungRfidId.c_str()) == 0) {
      // Check if 2 seconds have passed since last detection (debounce protection)
      if (currentTime - lastUjungDetectionTime >= UJUNG_IGNORE_DURATION) {
        lastUjungDetectionTime = currentTime;  // Update timer
        
        // Toggle mode
        isUjungSlowMode = !isUjungSlowMode;
        saveUjungSlowMode();  // Save to preferences
        
        // Set speed based on mode
        if (isUjungSlowMode) {
          pidSpeed = maxMotorRpm / 3;  // SLOW mode: 33% speed
        } else {
          pidSpeed = maxMotorRpm;       // FAST mode: 100% speed
        }
      }
      // If less than 2 seconds: ignore detection (debounce protection)
    }
  }
    
  if (strlen(currentRfid) > 0 && newRfidScanned) {
    // Check Terminal Pickup RFID and Current State isn't Terminal Pickup
    if (strcmp(currentRfid, terminalPickUpRfidId.c_str()) == 0 && currentRFID != AGV_STATE_TERMINAL_PICKUP) {
      stopMusic();
      newRfidScanned = false; // Reset flag
      isHookUp = false;
      exceptErrorPosition = false;
      saveExceptErrorFlag();
      currentRFID = AGV_STATE_TERMINAL_PICKUP;
      needsSoftStart = true;
      agvMode(AGV_STATE_TERMINAL_PICKUP);
      return;
    // Check Warehouse RFID and Current State isn't Warehouse
    } else if (strcmp(currentRfid, warehouseRfidId.c_str()) == 0 && currentRFID != AGV_STATE_WAREHOUSE) {
      stopMusic();
      newRfidScanned = false; // Reset flag
      exceptErrorPosition = true;
      saveExceptErrorFlag();
      currentRFID = AGV_STATE_WAREHOUSE;
      needsSoftStart = true;
      agvMode(AGV_STATE_WAREHOUSE);
      return;
    // Check Terminal Drop RFID and Current State isn't Terminal Drop
    } else  if (strcmp(currentRfid, terminalDropRfidId.c_str()) == 0 && currentRFID != AGV_STATE_TERMINAL_DROP) {
      newRfidScanned = false; // Reset flag
      currentRFID = AGV_STATE_TERMINAL_DROP;
      needsSoftStart = true;
      agvMode(AGV_STATE_TERMINAL_DROP);
      return;
    // Check RFID for station with exceptErrorPosition handling
    } else if (strlen(currentRfid) > 0 && newRfidScanned && strcmp(currentRfid, getRfidForStation(1).c_str()) == 0) {
      newRfidScanned = false; // Reset flag
      if (currentRFID == AGV_STATE_WAREHOUSE){ // If coming from warehouse, clear error position
        exceptErrorPosition = false;
      } else {
        exceptErrorPosition = true;
      }
      currentRFID = AGV_STATE_NULL;
      saveExceptErrorFlag();
    }
  }

  // Check for station RFID
  int currentStation = getStationFromLastRfid();

  // If a station is detected, check if it is in the target list
  if (currentStation != -1) {
    // Check if this station is in the targetStationsList
    for (int i = 0; i < targetStationsList.size(); i++) {
      if (targetStationsList[i] == currentStation) {
        removeTargetStationById(currentStation);
        stopMusic();
        needsSoftStart = true;
        agvMode(AGV_STATE_STATION);
        return;
      }
    }
  }

  // If there are no obstacles and it is not a target station, continue moving
  if (!obstacleDetected) {
    music(MUSIC_MODE_ON);
    if (exceptErrorPosition && totalSensorAktif > 7) {
      pidLinefollower(0, PID_MODE_MAJU);
    } else {
      if (targetStationsList.size() != 0 || currentRFID == AGV_STATE_TERMINAL_PICKUP) {
        pidLinefollower(errorValue, PID_MODE_MAJU);  // Error from magnet sensor
      } else {
        pidLinefollower(errorValue, PID_MODE_MAJU);  // Error from magnet sensor
      }
    }
  }
}

// Helper function to check RFID match
bool isRfidMatch(const char* currentRfid, const char* targetRfid) {
  // Null pointer check
  if (targetRfid == nullptr || currentRfid == nullptr) {
    return false;
  }
  
  // Check if target is not empty
  if (targetRfid[0] == '\0') {
    return false;
  }
  
  // Direct strcmp - faster than String.equals()
  return strcmp(currentRfid, targetRfid) == 0;
}

// Function to load all AGV states from Preferences
void loadAllAGVStatesFromPreferences() {
  preferences.begin("agv-state", true);

  // Load current state
  String currentStateString = preferences.getString("current_state", "NULL");
  currentStateAgv = stringToAgvState(currentStateString.c_str());

  preferences.end();
  
  // Reset warehouse state if AGV is loaded in WAREHOUSE state
  // This prevents the AGV from moving immediately after a restart
  if (currentStateAgv == AGV_STATE_WAREHOUSE) {
    resetWarehouseState();
  }
}

// Function to convert AgvState to string
const char* agvStateToString(AgvState state) {
  switch (state) {
    case AGV_STATE_MOVE_FORWARD:
      return "MOVE_FORWARD";
    case AGV_STATE_TERMINAL_PICKUP:
      return "TERMINAL_PICKUP";
    case AGV_STATE_TERMINAL_DROP:
      return "TERMINAL_DROP";
    case AGV_STATE_WAREHOUSE:
      return "WAREHOUSE";
    case AGV_STATE_STATION:
      return "STATION";
    case AGV_STATE_STOP:
      return "STOP";
    case AGV_STATE_NULL:
      return "NULL";
    default:
      return "NULL";
  }
}

// Function to save current AGV state to Preferences as string
// This function saves the current AGV state to Preferences.
void saveCurrentStateAGVToPreferences(AgvState currentState) {
  currentStateAgv = currentState;

  // Convert state to string
  const char* stateString = agvStateToString(currentState);

  // Save to Preferences
  preferences.begin("agv-state", false);
  preferences.putString("current_state", stateString);
  preferences.end();
}

// Function to convert string to AgvState
// This function converts a string representation of the AGV state back to the AgvState enum.
AgvState stringToAgvState(const char* stateString) {
  if (strcmp(stateString, "MOVE_FORWARD") == 0) {
    return AGV_STATE_MOVE_FORWARD;
  } else if (strcmp(stateString, "TERMINAL_PICKUP") == 0) {
    return AGV_STATE_TERMINAL_PICKUP;
  } else if (strcmp(stateString, "TERMINAL_DROP") == 0) {
    return AGV_STATE_TERMINAL_DROP;
  } else if (strcmp(stateString, "WAREHOUSE") == 0) {
    return AGV_STATE_WAREHOUSE;
  } else if (strcmp(stateString, "STATION") == 0) {
    return AGV_STATE_STATION;
  } else if (strcmp(stateString, "STOP") == 0) {
    return AGV_STATE_STOP;
  } else if (strcmp(stateString, "NULL") == 0) {
    return AGV_STATE_NULL;
  } else {
    return AGV_STATE_NULL;  // Default state
  }
}