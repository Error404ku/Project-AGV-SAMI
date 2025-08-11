static bool stopCalledPickup = false;
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

// Fungsi ini menangani logika AGV saat berada di gudang.
void agvWarehouse() {
  static bool trigger = false;
  saveCurrentStateAGVToPreferences(AGV_STATE_WAREHOUSE);
  music(MUSIC_MODE_WARNING);

  if (START()) {
    if (targetStationsList.size() == 0){
      // Tampilkan pesan tidak ada station di warehouse
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tidak ada station");
      lcd.setCursor(0, 1);
      lcd.print("dalam daftar target");
      delay(2000); // Tampilkan pesan selama 2 detik
      return; // Kembali tanpa memulai pergerakan
    } else {
      trigger = true;
    }
  }
  if (!trigger) {
    agvStop();
    modeDisplayWarehouse();
  }else{
    stopCalledPickup = false; // Reset flag untuk penggunaan berikutnya
    agvMode(AGV_STATE_MOVE_FORWARD);
    trigger = false;
    return;
  }
}

// Fungsi ini menangani logika AGV saat berada di stasiun.
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
  if (START()) {
    lastReadTime = 0;
    trigger = true;
  }
  if (trigger) {
    // Cek apakah masih ada station di StationList
    if (targetStationsList.size() == 0) {
      // Tidak ada station tersisa, ubah ke mode backward
      moveStateAgv = AGV_STATE_MOVE_BACKWARD;
      agvMode(AGV_STATE_MOVE_BACKWARD);
    } else {
      // Masih ada station, lanjutkan sesuai arah sebelumnya
      if (moveStateAgv == AGV_STATE_MOVE_FORWARD) {
        agvMode(AGV_STATE_MOVE_FORWARD);
      } else if (moveStateAgv == AGV_STATE_MOVE_BACKWARD) {
        agvMode(AGV_STATE_MOVE_BACKWARD);
      }
    }
    trigger = false;
    return;
  }
}

// Fungsi ini menangani logika AGV saat melakukan penurunan di terminal.
void agvTerminalDrop() {
  static int dropProcessStep = 0; 
  saveCurrentStateAGVToPreferences(AGV_STATE_TERMINAL_DROP);
  modeDisplayTerminalDrop();
  static unsigned long lastStopTime = 0;
  static unsigned long currentTime = millis();
  music(MUSIC_MODE_ON);
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

// Fungsi ini menangani logika AGV saat melakukan pengambilan di terminal.
void agvTerminalPickup() {
  static bool isHookUp = false;
  modeDisplayTerminalPickup(isHookUp);
  static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();
  
  if (currentStateAgv == AGV_STATE_NULL){
    isHookUp = false;
  }
  if (currentStateAgv != AGV_STATE_NULL){
    saveCurrentStateAGVToPreferences(AGV_STATE_TERMINAL_PICKUP);
  }
  if (!stopCalledPickup) {
    agvStop();
    delay(2000);
    stopCalledPickup = true;
  }
  if (!isHookUp) {
    bool triggerNaikOtomatis = (currentStateAgv != AGV_STATE_NULL);
    bool triggerNaikManual = (currentStateAgv == AGV_STATE_NULL && START());
    currentRFID = AGV_STATE_TERMINAL_PICKUP;
    if (triggerNaikOtomatis) {
      saveCurrentStateAGVToPreferences(AGV_STATE_TERMINAL_PICKUP);
      hookPosition = hook(UP_HOOK);
      if (hookPosition == UP_POS){
        isHookUp = true;
      }
    }
    if (triggerNaikManual) {
      saveCurrentStateAGVToPreferences(AGV_STATE_TERMINAL_PICKUP);
      hookPosition = hook(UP_HOOK);
      if (hookPosition == UP_POS){
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

// Fungsi ini menghentikan pergerakan AGV.
void agvStop() {
  pwmMotor(0, 0);
  return;
}

// Fungsi ini menangani logika AGV saat bergerak maju.
void agvMoveForward() {
  saveCurrentStateAGVToPreferences(AGV_STATE_MOVE_FORWARD);
  checkObstacles();
  modeDisplayMoveForward();
  moveStateAGV(AGV_STATE_MOVE_FORWARD);
  // Cek RFID yang terdeteksi untuk mode switching (harus dilakukan sebelum getStationFromLastRfid)
  String currentRfid = String(lastScannedRfidOptimized);
  if (currentRfid.length() > 0 && newRfidScanned) {
    if (isRfidMatch(currentRfid, ujungRfidId)) {
      newRfidScanned = false; // Reset flag
      // Interrupt motor sebelum mengubah mode
      pwmMotor(0, 0);
      delay(2000);
      agvMode(AGV_STATE_MOVE_BACKWARD);
      return;
    } else if (isRfidMatch(currentRfid, terminalDropRfidId) && currentRFID != AGV_STATE_TERMINAL_DROP) {
      newRfidScanned = false; // Reset flag
      currentRFID = AGV_STATE_TERMINAL_DROP;
      agvMode(AGV_STATE_TERMINAL_DROP);
      return;
    } else if (isRfidMatch(currentRfid, terminalPickUpRfidId) && currentRFID != AGV_STATE_TERMINAL_PICKUP) {
      newRfidScanned = false; // Reset flag
      exceptErrorPosition = true;
      saveExceptErrorFlag();
      currentRFID = AGV_STATE_TERMINAL_PICKUP;
      agvMode(AGV_STATE_TERMINAL_PICKUP);
      return;
    } else if (isRfidMatch(currentRfid, warehouseRfidId) && currentRFID != AGV_STATE_WAREHOUSE) {
      newRfidScanned = false; // Reset flag
      exceptErrorPosition = false;
      saveExceptErrorFlag();
      currentRFID = AGV_STATE_WAREHOUSE;
      agvMode(AGV_STATE_WAREHOUSE);
      return;
    }
  }

  // Cek apakah ada RFID yang terbaca untuk stasiun
  int currentStation = getStationFromLastRfid();

  // Jika ada stasiun yang terdeteksi, cek apakah ada di target list
  if (currentStation != -1) {
    // Cari apakah stasiun ini ada di targetStationsList
    for (int i = 0; i < targetStationsList.size(); i++) {
      if (targetStationsList[i] == currentStation) {
        removeTargetStationById(currentStation);
        agvMode(AGV_STATE_STATION);
        return;
      }
    }
  }

  // Jika tidak ada hambatan dan bukan stasiun target, lanjutkan bergerak
  if (!obstacleDetected) {
    music(MUSIC_MODE_ON);
    if (exceptErrorPosition && totalSensorAktif > 5) {
      pidLinefollower(0, PID_MODE_MAJU_MASSA);  
    }else if(forceLeft){
      pwmMotor(baseSpeed/2, baseSpeed/2);
      if (totalSensorAktif == 0){
        inLine = false;
      }
      if (totalSensorAktif > 0 && !inLine){
        inLine = true;
        forceLeft = false;
      }
    }else {
      if (targetStationsList.size() != 0 || currentRFID == AGV_STATE_TERMINAL_PICKUP) {
        pidLinefollower(errorValue, PID_MODE_MAJU_MASSA);  // Error dari sensor magnet
      } else {
        pidLinefollower(errorValue, PID_MODE_MAJU);  // Error dari sensor magnet
      }
    }
  }
}

// Fungsi ini menangani logika AGV saat bergerak mundur.
void agvMoveBackward() {
  saveCurrentStateAGVToPreferences(AGV_STATE_MOVE_BACKWARD);
  modeDisplayMoveBackward();
  checkObstacles();
  moveStateAGV(AGV_STATE_MOVE_BACKWARD);
  // AGV bergerak mundur otomatis tanpa perlu menekan START
  // Logika pergerakan mundur:
  // Cek apakah RFID warehouse terdeteksi untuk mengabaikan error saat mundur
  String currentRfid = String(lastScannedRfidOptimized);
  if (currentRfid.length() > 0 && newRfidScanned && isRfidMatch(currentRfid, warehouseRfidId)) {
    exceptErrorPosition = true;  // Set flag untuk mengabaikan error saat mundur
    saveExceptErrorFlag();       // Simpan flag ke preferences
    newRfidScanned = false;      // Reset flag
  }
  // Kembali ke mode maju ketika mencapai jalur lurus (10+ sensor aktif)
  if (currentRfid.length() > 0 && newRfidScanned && isRfidMatch(currentRfid, pertigaanRfidId)) {
    exceptErrorPosition = false;
    forceLeft = true;
    saveExceptErrorFlag();
    agvMode(AGV_STATE_MOVE_FORWARD);
    return;
  }

  if (targetStationsList.size() != 0) {
    // Cek apakah ada RFID yang terbaca
    int currentStation = getStationFromLastRfid();

    // Jika ada stasiun yang terdeteksi, cek apakah ada di target list
    if (currentStation != -1) {
      // Cari apakah stasiun ini ada di targetStationsList
      for (int i = 0; i < targetStationsList.size(); i++) {
        if (targetStationsList[i] == currentStation) {
          removeTargetStationById(currentStation);
          moveStateAGV(AGV_STATE_MOVE_BACKWARD);
          agvMode(AGV_STATE_STATION);
          return;
        }
      }
    }
  }
  // Jika tidak ada hambatan dan bukan stasiun target, lanjutkan bergerak
  if (!obstacleDetected) {
    music(MUSIC_MODE_ON);
    // Abaikan error jika warehouse RFID pernah terdeteksi dan segment aktif >5
    if (exceptErrorPosition && totalSensorAktif > 5) {
      pidLinefollower(0, PID_MODE_MUNDUR_MASSA);  // Error = 0
    } else {
      if (targetStationsList.size() != 0){
        pidLinefollower(errorValue, PID_MODE_MUNDUR_MASSA);  // Error normal
      } else {
        pidLinefollower(0, PID_MODE_MUNDUR);
      }
    }
  }
}

// Helper function untuk memeriksa kecocokan RFID
bool isRfidMatch(const String& currentRfid, const String& targetRfid) {
  return targetRfid.length() > 0 && currentRfid.equals(targetRfid);
}

// Fungsi ini menyimpan status terakhir AGV (maju atau mundur).
void moveStateAGV(AgvState lastState) {
  if (lastState == AGV_STATE_MOVE_FORWARD) {
    moveStateAgv = AGV_STATE_MOVE_FORWARD;
  } else if (lastState == AGV_STATE_MOVE_BACKWARD) {
    moveStateAgv = AGV_STATE_MOVE_BACKWARD;
  }

  // Simpan moveStateAGV ke preferences
  savemoveStateAGVToPreferences(moveStateAgv);
}

// Fungsi untuk menyimpan moveStateAGV ke Preferences
void savemoveStateAGVToPreferences(AgvState lastState) {
  // Konversi state ke string
  String stateString = agvStateToString(lastState);

  // Simpan ke Preferences
  preferences.begin("agv-state", false);
  preferences.putString("last_state", stateString);
  preferences.end();

  Serial.println("Last AGV State saved to Preferences: " + stateString);
}

// Fungsi gabungan untuk memuat semua state AGV dari Preferences
void loadAllAGVStatesFromPreferences() {
  preferences.begin("agv-state", true);

  // Load move state (last_state)
  String moveStateString = preferences.getString("last_state", "MOVE_FORWARD");
  moveStateAgv = stringToAgvState(moveStateString);

  // Load current state
  String currentStateString = preferences.getString("current_state", "NULL");
  currentStateAgv = stringToAgvState(currentStateString);

  preferences.end();
}

// Fungsi untuk memuat moveStateAGV dari Preferences (backward compatibility)
// AgvState loadmoveStateAGVFromPreferences() {
//     preferences.begin("agv-state", true);
//     String stateString = preferences.getString("last_state", "MOVE_FORWARD");
//     preferences.end();

//     AgvState loadedState = stringToAgvState(stateString);
//     moveStateAGVVar = loadedState;

//     Serial.println("Last AGV State loaded from Preferences: " + stateString);
//     return loadedState;
// }

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
    case AGV_STATE_STOP:
      return "STOP";
    case AGV_STATE_NULL:
      return "NULL";
    default:
      return "NULL";
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
  } else if (stateString == "STOP") {
    return AGV_STATE_STOP;
  } else if (stateString == "NULL") {
    return AGV_STATE_NULL;
  } else {
    return AGV_STATE_NULL;  // Default state
  }
}

// Fungsi untuk memuat current state AGV dari Preferences (backward compatibility)
// AgvState loadCurrentStateAGVFromPreferences() {
//     preferences.begin("agv-state", true);
//     String stateString = preferences.getString("current_state", "STOP");
//     preferences.end();

//     AgvState loadedState = stringToAgvState(stateString);
//     currentStateAgv = loadedState;

//     Serial.println("Current AGV State loaded from Preferences: " + stateString);
//     return loadedState;
// }