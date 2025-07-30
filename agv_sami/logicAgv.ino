void agvMode(AgvState state) {
  if (state == AGV_STATE_MOVE_FORWARD) {
    agvMoveForward();
  } else if (state == AGV_STATE_MOVE_BACKWARD) {
    agvMoveBackward();
  } else if (state == AGV_STATE_STOP) {
    agvStop();
  } else if (state == AGV_STATE_TERMINAL_DROP) {
    agvTerminalDrop();
  } else if (state == AGV_STATE_TERMINAL_PICKUP) {
    agvTerminalPickup();
  } else if (state == AGV_STATE_WAREHOUSE) {
    agvWarehouse();
  } else if (state == AGV_STATE_STATION) {
    agvStation();
  } else {
    agvStop();
  }
}

void agvWarehouse() {
  saveCurrentStateAGVToPreferences(AGV_STATE_WAREHOUSE);
  agvStop();
  if (START()) {
    agvMode(AGV_STATE_MOVE_FORWARD);
  }
}

void agvStation() {
  saveCurrentStateAGVToPreferences(AGV_STATE_STATION);
  if (START()) {
    if (lastStateAgv == AGV_STATE_MOVE_FORWARD) {
      agvMode(AGV_STATE_MOVE_FORWARD);
    }else if (lastStateAgv == AGV_STATE_MOVE_BACKWARD) {
      agvMode(AGV_STATE_MOVE_BACKWARD);
    }
  }
}

void agvTerminalPickup(); {
  saveCurrentStateAGVToPreferences(AGV_STATE_TERMINAL);
  agvStop();
  hook("turun");
  delay(2000);
  pidLinefollower(2, PID_MODE_MAJU);
  String currentRfid = String(lastScannedRfidOptimized);
  if (currentRfid.length() > 0) {
    agvMode(AGV_STATE_WAREHOUSE);
    lastStateAGV(AGV_STATE_TERMINAL);
    return;
  }
}

void agvStop() {
  saveCurrentStateAGVToPreferences(AGV_STATE_STOP);
  pwmMotor(0, 0);
}

void agvMoveForward() {
  saveCurrentStateAGVToPreferences(AGV_STATE_MOVE_FORWARD);
  // Cek apakah ada RFID yang terbaca
  int currentStation = getStationFromLastRfid();

  // Jika ada stasiun yang terdeteksi, cek apakah ada di target list
  if (currentStation != -1) {
    // Cari apakah stasiun ini ada di targetStationsList
    for (int i = 0; i < targetStationsList.size(); i++) {
      if (targetStationsList[i] == currentStation) {
        agvMode(AGV_STATE_STATION);
        lastStateAGV(AGV_STATE_MOVE_FORWARD);
        return;
      }
    }
  }

  // Cek apakah RFID ujung terdeteksi
  String currentRfid = String(lastScannedRfidOptimized);
  if (currentRfid.length() > 0 && currentRfid.equals(ujungRfidId) && ujungRfidId.length() > 0) {
    agvMode(AGV_STATE_MOVE_BACKWARD);
    lastStateAGV(AGV_STATE_MOVE_FORWARD);
    return;
  }

  // Jika tidak ada hambatan dan bukan stasiun target, lanjutkan bergerak
  if (!obstacleDetected) {
    pidLinefollower(2, PID_MODE_MAJU);
  }
}

void agvMoveBackward() {
  saveCurrentStateAGVToPreferences(AGV_STATE_MOVE_BACKWARD);
  if (targetStationsList.size() != 0) {
    // Cek apakah ada RFID yang terbaca
    int currentStation = getStationFromLastRfid();

    // Jika ada stasiun yang terdeteksi, cek apakah ada di target list
    if (currentStation != -1) {
      // Cari apakah stasiun ini ada di targetStationsList
      for (int i = 0; i < targetStationsList.size(); i++) {
        if (targetStationsList[i] == currentStation) {
          agvMode(AGV_STATE_STATION);
          lastStateAGV(AGV_STATE_MOVE_BACKWARD);
          return;
        }
      }
    }
  }

  // Cek apakah RFID terminal terdeteksi
  String currentRfid = String(lastScannedRfidOptimized);
  if (currentRfid.length() > 0) {
    agvMode(AGV_STATE_TERMINAL);
    lastStateAGV(AGV_STATE_MOVE_BACKWARD);
    return;
  }
  // Jika tidak ada hambatan dan bukan stasiun target, lanjutkan bergerak
  if (!obstacleDetected) {
    pidLinefollower(2, PID_MODE_MUNDUR);
  }
}


void lastStateAGV(AgvState lastState){
    if (lastState == AGV_STATE_MOVE_FORWARD) {
        lastStateAgv = AGV_STATE_MOVE_FORWARD;
    } else if (lastState == AGV_STATE_MOVE_BACKWARD) {
        lastStateAgv = AGV_STATE_MOVE_BACKWARD;
    } else if (lastState == AGV_STATE_TERMINAL) {
        lastStateAgv = AGV_STATE_TERMINAL;
    } else if (lastState == AGV_STATE_WAREHOUSE) {
        lastStateAgv = AGV_STATE_WAREHOUSE;
    } else if (lastState == AGV_STATE_STATION) {
        lastStateAgv = AGV_STATE_STATION;
    } else if (lastState == AGV_STATE_STOP) {
        lastStateAgv = AGV_STATE_STOP;
    }
}

// Fungsi untuk mengkonversi AgvState ke string
String agvStateToString(AgvState state) {
    switch (state) {
        case AGV_STATE_MOVE_FORWARD:
            return "MOVE_FORWARD";
        case AGV_STATE_MOVE_BACKWARD:
            return "MOVE_BACKWARD";
        case AGV_STATE_STOP:
            return "STOP";
        case AGV_STATE_TERMINAL:
            return "TERMINAL";
        case AGV_STATE_WAREHOUSE:
            return "WAREHOUSE";
        case AGV_STATE_STATION:
            return "STATION";
        default:
            return "UNKNOWN";
    }
}

// Fungsi untuk menyimpan state AGV ke Preferences dalam bentuk string
void saveCurrentStateAGVToPreferences(AgvState currentState) {
    currentStateAgv = currentState;
    
    // Konversi state ke string
    String stateString = agvStateToString(currentState);
    
    // Simpan ke Preferences
    preferences.begin("agv-state", false);
    preferences.putString("current_state", stateString);
    preferences.end();
    
    Serial.println("AGV State saved to Preferences: " + stateString);
}

// Fungsi untuk mengkonversi string ke AgvState
AgvState stringToAgvState(String stateString) {
    if (stateString == "MOVE_FORWARD") {
        return AGV_STATE_MOVE_FORWARD;
    } else if (stateString == "MOVE_BACKWARD") {
        return AGV_STATE_MOVE_BACKWARD;
    } else if (stateString == "STOP") {
        return AGV_STATE_STOP;
    } else if (stateString == "TERMINAL") {
        return AGV_STATE_TERMINAL;
    } else if (stateString == "WAREHOUSE") {
        return AGV_STATE_WAREHOUSE;
    } else if (stateString == "STATION") {
        return AGV_STATE_STATION;
    } else {
        return AGV_STATE_STOP; // Default state
    }
}

// Fungsi untuk memuat state AGV dari Preferences
AgvState loadCurrentStateAGVFromPreferences() {
    preferences.begin("agv-state", true);
    String stateString = preferences.getString("current_state", "STOP");
    preferences.end();
    
    AgvState loadedState = stringToAgvState(stateString);
    currentStateAgv = loadedState;
    
    Serial.println("AGV State loaded from Preferences: " + stateString);
    return loadedState;
}

// Fungsi untuk mengkonversi AgvState ke string
String agvStateToString(AgvState state) {
    switch (state) {
        case AGV_STATE_MOVE_FORWARD:
            return "MOVE_FORWARD";
        case AGV_STATE_MOVE_BACKWARD:
            return "MOVE_BACKWARD";
        case AGV_STATE_STOP:
            return "STOP";
        case AGV_STATE_TERMINAL:
            return "TERMINAL";
        case AGV_STATE_WAREHOUSE:
            return "WAREHOUSE";
        case AGV_STATE_STATION:
            return "STATION";
        default:
            return "UNKNOWN";
    }
}

// Fungsi untuk menyimpan state AGV ke Preferences dalam bentuk string
void saveCurrentStateAGVToPreferences(AgvState currentState) {
    currentStateAgv = currentState;
    
    // Konversi state ke string
    String stateString = agvStateToString(currentState);
    
    // Simpan ke Preferences
    preferences.begin("agv-state", false);
    preferences.putString("current_state", stateString);
    preferences.end();
    
    Serial.println("AGV State saved to Preferences: " + stateString);
}

// Fungsi untuk mengkonversi string ke AgvState
AgvState stringToAgvState(String stateString) {
    if (stateString == "MOVE_FORWARD") {
        return AGV_STATE_MOVE_FORWARD;
    } else if (stateString == "MOVE_BACKWARD") {
        return AGV_STATE_MOVE_BACKWARD;
    } else if (stateString == "STOP") {
        return AGV_STATE_STOP;
    } else if (stateString == "TERMINAL") {
        return AGV_STATE_TERMINAL;
    } else if (stateString == "WAREHOUSE") {
        return AGV_STATE_WAREHOUSE;
    } else if (stateString == "STATION") {
        return AGV_STATE_STATION;
    } else {
        return AGV_STATE_STOP; // Default state
    }
}

// Fungsi untuk memuat state AGV dari Preferences
AgvState loadCurrentStateAGVFromPreferences() {
    preferences.begin("agv-state", true);
    String stateString = preferences.getString("current_state", "STOP");
    preferences.end();
    
    AgvState loadedState = stringToAgvState(stateString);
    currentStateAgv = loadedState;
    
    Serial.println("AGV State loaded from Preferences: " + stateString);
    return loadedState;
}