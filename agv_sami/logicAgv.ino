// Fungsi ini mengatur mode operasi AGV berdasarkan status yang diberikan.
void agvMode(AgvState state) {
  switch (state) {
    case AGV_STATE_MOVE_FORWARD:
      agvMoveForward();
      break;
    case AGV_STATE_MOVE_BACKWARD:
      agvMoveBackward();
      break;
    case AGV_STATE_STOP:
      agvStop();
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

// Fungsi ini menangani logika AGV saat berada di gudang.
void agvWarehouse() {
  bool trigger = false;
  saveCurrentStateAGVToPreferences(AGV_STATE_WAREHOUSE);
  agvStop();
  modeDisplayWarehouse();
  if (START()) {
    trigger = true;
  }
  switch(trigger){
    case true:
      agvMode(AGV_STATE_MOVE_FORWARD);
      break;
  }
}

// Fungsi ini menangani logika AGV saat berada di stasiun.
void agvStation() {
  bool trigger = false;
  saveCurrentStateAGVToPreferences(AGV_STATE_STATION);
  
  if (START()) {
    trigger = true;
  }
  switch(trigger){
    case true:
      if (lastStateAgv == AGV_STATE_MOVE_FORWARD) {
        agvMode(AGV_STATE_MOVE_FORWARD);
      }else if (lastStateAgv == AGV_STATE_MOVE_BACKWARD) {
        agvMode(AGV_STATE_MOVE_BACKWARD);
      }
      break;
  }
}

// Fungsi ini menangani logika AGV saat melakukan penurunan di terminal.
void agvTerminalDrop() {
  bool trigger = false;
  //Save current state
  saveCurrentStateAGVToPreferences(AGV_STATE_TERMINAL_DROP);

  agvStop();
  hook("turun");
  delay(2000);

  if (START()){
    trigger = true;
  }
  switch(trigger){
    case true:
      pidLinefollower(2, PID_MODE_MAJU);
      //Scan terminal pickup rfid
      String currentRfid = String(lastScannedRfidOptimized);
      if (currentRfid.length() > 0 && currentRfid.equals(terminalPickUpRfidId) && terminalPickUpRfidId.length() > 0) {
        agvMode(AGV_STATE_TERMINAL_PICKUP);
      }
      break;
  }
}

// Fungsi ini menangani logika AGV saat melakukan pengambilan di terminal.
void agvTerminalPickup() {
  bool trigger = false;
  bool triggerHook = false;
  //Save current state
  saveCurrentStateAGVToPreferences(AGV_STATE_TERMINAL_PICKUP);

  agvStop();
  hook("turun");

  if (!currentStateAGV == AGV_STATE_NULL){
    hook("naik");
    delay(2000);
  }else{
    if (START()){
      triggerHook = true;
    }
    switch(triggerHook){
      case true:
        hook("naik");
        delay(2000);
        trigger = false;
        break;
    }
  }

  if (START()){
    trigger = true;
  }

  switch(trigger){
    case true:
      pidLinefollower(2, PID_MODE_MAJU);
      //Scan warehouse rfid
      String currentRfid = String(lastScannedRfidOptimized);
      if (currentRfid.length() > 0 && currentRfid.equals(terminalDropRfidId) && terminalDropRfidId.length() > 0) {
        agvMode(AGV_STATE_WAREHOUSE);
        break;
      }
      break;
    }
}

// Fungsi ini menghentikan pergerakan AGV.
void agvStop() {
  pwmMotor(0, 0);
  return;
}

// Fungsi ini menangani logika AGV saat bergerak maju.
void agvMoveForward() {
  bool trigger = false;
  saveCurrentStateAGVToPreferences(AGV_STATE_MOVE_FORWARD);
  
  if (START()){
    trigger = true;
  }
  switch(trigger){
    case true:
      // Cek apakah ada RFID yang terbaca
      int currentStation = getStationFromLastRfid();

      // Jika ada stasiun yang terdeteksi, cek apakah ada di target list
      if (currentStation != -1) {
        // Cari apakah stasiun ini ada di targetStationsList
        for (int i = 0; i < targetStationsList.size(); i++) {
          if (targetStationsList[i] == currentStation) {
            removeTargetStationById(currentStation);
            lastStateAGV(AGV_STATE_MOVE_FORWARD);
            agvMode(AGV_STATE_STATION);
            break;
          }
        }
      }

      // Cek apakah RFID ujung terdeteksi
      String currentRfid = String(lastScannedRfidOptimized);
      if (currentRfid.length() > 0 && currentRfid.equals(ujungRfidId) && ujungRfidId.length() > 0) {
        agvMode(AGV_STATE_MOVE_BACKWARD);
        break;
      }

      // Jika tidak ada hambatan dan bukan stasiun target, lanjutkan bergerak
      if (!obstacleDetected) {
        pidLinefollower(2, PID_MODE_MAJU);
      }
      break;
  }
}

// Fungsi ini menangani logika AGV saat bergerak mundur.
void agvMoveBackward() {
  bool trigger = false;
  saveCurrentStateAGVToPreferences(AGV_STATE_MOVE_BACKWARD);
  
  if (START()){
    trigger = true;
  }
  switch(trigger){
    case true:
      if (targetStationsList.size() != 0) {
        // Cek apakah ada RFID yang terbaca
        int currentStation = getStationFromLastRfid();

        // Jika ada stasiun yang terdeteksi, cek apakah ada di target list
        if (currentStation != -1) {
          // Cari apakah stasiun ini ada di targetStationsList
          for (int i = 0; i < targetStationsList.size(); i++) {
            if (targetStationsList[i] == currentStation) {
              removeTargetStationById(currentStation);
              lastStateAGV(AGV_STATE_MOVE_BACKWARD);
              agvMode(AGV_STATE_STATION);
              break;
            }
          }
        }
      }

      // Cek apakah RFID terminal terdeteksi
      String currentRfid = String(lastScannedRfidOptimized);
      if (currentRfid.length() > 0 && currentRfid.equals(terminalDropRfidId) && terminalDropRfidId.length() > 0) {
        agvMode(AGV_STATE_TERMINAL_DROP);
        lastStateAGV(AGV_STATE_MOVE_BACKWARD);
        break;
      }
      // Jika tidak ada hambatan dan bukan stasiun target, lanjutkan bergerak
      if (!obstacleDetected) {
        pidLinefollower(2, PID_MODE_MUNDUR);
      }
      break;
  }
}


// Fungsi ini menyimpan status terakhir AGV (maju atau mundur).
void lastStateAGV(AgvState lastState){
    if (lastState == AGV_STATE_MOVE_FORWARD) {
        lastStateAgv = AGV_STATE_MOVE_FORWARD;
    } else if (lastState == AGV_STATE_MOVE_BACKWARD) {
        lastStateAgv = AGV_STATE_MOVE_BACKWARD;
    }
}

// Fungsi untuk mengkonversi AgvState ke string
// Fungsi ini mengkonversi nilai AgvState menjadi representasi string.
String agvStateToString(AgvState state) {
    switch (state) {
        case AGV_STATE_MOVE_FORWARD:
            return "MOVE_FORWARD";
        case AGV_STATE_MOVE_BACKWARD:
            return "MOVE_BACKWARD";
        case AGV_STATE_TERMINAL_PICKUP:
            return "TERMINAL_PICKUP";
        case AGV_STATE_TERMINAL_DROP:
            return "TERMINAL_DROP";
        case AGV_STATE_WAREHOUSE:
            return "WAREHOUSE";
        case AGV_STATE_STATION:
            return "STATION";
        case AGV_STATE_TERMINAL_DROP:
            return "TERMINAL_DROP";
        default:
            return "UNKNOWN";
    }
}

// Fungsi untuk menyimpan state AGV ke Preferences dalam bentuk string
// Fungsi ini menyimpan status AGV saat ini ke Preferences.
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
// Fungsi ini mengkonversi representasi string dari status AGV kembali ke nilai AgvState.
AgvState stringToAgvState(String stateString) {
    if (stateString == "MOVE_FORWARD") {
        return AGV_STATE_MOVE_FORWARD;
    } else if (stateString == "MOVE_BACKWARD") {
        return AGV_STATE_MOVE_BACKWARD;
    } else if (stateString == "TERMINAL_PICKUP") {
        return AGV_STATE_TERMINAL_PICKUP;
    } else if (stateString == "TERMINAL_DROP") {
        return AGV_STATE_TERMINAL_DROP;
    } else if (stateString == "WAREHOUSE") {
        return AGV_STATE_WAREHOUSE;
    } else if (stateString == "STATION") {
        return AGV_STATE_STATION;
    } else {
        return AGV_STATE_STOP; // Default state
    }
}

// Fungsi untuk memuat state AGV dari Preferences
// Fungsi ini memuat status AGV terakhir dari Preferences.
AgvState loadCurrentStateAGVFromPreferences() {
    preferences.begin("agv-state", true);
    String stateString = preferences.getString("current_state", "UNKNOWN");

    preferences.end();
    
    AgvState loadedState = stringToAgvState(stateString);
    currentStateAgv = loadedState;
    
    return loadedState;
}