// Fungsi ini mengatur mode operasi AGV berdasarkan status yang diberikan.
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

// Global flag untuk reset warehouse state
static bool warehouseNeedReset = false;

// Fungsi untuk reset state warehouse saat load dari preferences
void resetWarehouseState() {
  // Set flag global agar agvWarehouse() melakukan reset saat dipanggil berikutnya
  // Ini diperlukan agar setelah restart, AGV tidak langsung jalan
  warehouseNeedReset = true;
}

// Fungsi ini menangani logika AGV saat berada di gudang.
void agvWarehouse() {
  static bool trigger = false;
  static bool showingErrorMessage = false;
  static unsigned long errorMessageStartTime = 0;
  static bool needsDisplayRefresh = false;
  
  // Reset trigger saat pertama kali dipanggil setelah restart/load state
  if (warehouseNeedReset) {
    trigger = false;
    showingErrorMessage = false;
    needsDisplayRefresh = false;
    warehouseNeedReset = false;  // Reset hanya sekali
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

  if (START()) {
    if (targetStationsList.size() == 0){
      // Tampilkan pesan tidak ada station di warehouse
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tidak ada station");
      lcd.setCursor(0, 1);
      lcd.print("di daftar target"); // Shortened to fit 16 chars
      showingErrorMessage = true;
      errorMessageStartTime = millis();
      return; // Kembali tanpa memulai pergerakan
    } else {
      trigger = true;
    }
  }
  if (!trigger) {
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
    softStartTime = millis();
    softStartActive = true;
    pidSpeed = maxMotorRpm / 2;  // 🔧 FIX #2: 50% initial speed (was 25%, was baseSpeed)
    stopCalledPickup = false; // Reset flag untuk penggunaan berikutnya
    updatestations = false;
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
      // Tidak ada station tersisa
      softStartTime = millis();
      softStartActive = true;
      pidSpeed = maxMotorRpm / 2;  // 🔧 FIX #2: 50% initial speed (was 25%, was baseSpeed)
      agvMode(AGV_STATE_MOVE_FORWARD);
    } else {
      // Masih ada station, lanjutkan maju
      agvMode(AGV_STATE_MOVE_FORWARD);
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
    exceptErrorPosition = false;
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
      softStartTime = millis();
      softStartActive = true;
      pidSpeed = maxMotorRpm / 2;  // 🔧 FIX #2: 50% initial speed (was 25%, was baseSpeed)
      agvMode(AGV_STATE_MOVE_FORWARD);
    }
  }
}

// Fungsi ini menghentikan pergerakan AGV.
void agvStop() {
  rpmMotor(0, 0);
  Serial.println("STOP");  // Use PWM stop command
  return;
}

// Fungsi ini menangani logika AGV saat bergerak maju.
void agvMoveForward() {
  saveCurrentStateAGVToPreferences(AGV_STATE_MOVE_FORWARD);
  checkObstacles();
  modeDisplayMoveForward();
  // Cek RFID yang terdeteksi untuk mode switching (harus dilakukan sebelum getStationFromLastRfid)
  String currentRfid = String(lastScannedRfidOptimized);
  unsigned long currentTime = millis();
  
  // ✅ Deteksi Ujung RFID dengan Debounce 2 detik
  if (currentRfid.length() > 0) {
    if (isRfidMatch(currentRfid, ujungRfidId)) {
      // Cek apakah sudah lewat 5 detik sejak deteksi terakhir (debounce protection)
      if (currentTime - lastUjungDetectionTime >= UJUNG_IGNORE_DURATION) {
        lastUjungDetectionTime = currentTime;  // Update timer
        
        // Toggle mode: false (cepat) <-> true (lambat)
        isUjungSlowMode = !isUjungSlowMode;
        saveUjungSlowMode();  // Simpan ke preferences
        
        // Set kecepatan berdasarkan mode
        if (isUjungSlowMode) {
          pidSpeed = maxMotorRpm / 3;  // Mode LAMBAT: 50% speed
        } else {
          pidSpeed = maxMotorRpm;       // Mode CEPAT: 100% speed
        }
      }
      // Jika belum 2 detik: abaikan deteksi (debounce protection)
    }
  }
    
  if (currentRfid.length() > 0 && newRfidScanned) {
    if (isRfidMatch(currentRfid, terminalPickUpRfidId) && currentRFID != AGV_STATE_TERMINAL_PICKUP) {
      stopMusic();
      newRfidScanned = false; // Reset flag
      isHookUp = false;
      exceptErrorPosition = false;
      saveExceptErrorFlag();
      currentRFID = AGV_STATE_TERMINAL_PICKUP;
      softStartTime = millis();
      softStartActive = true;
      pidSpeed = maxMotorRpm / 2;  // 🔧 FIX #2: 50% initial speed (was 25%, was baseSpeed)
      agvMode(AGV_STATE_TERMINAL_PICKUP);
      return;
    } else if (isRfidMatch(currentRfid, warehouseRfidId) && currentRFID != AGV_STATE_WAREHOUSE) {
      stopMusic();
      newRfidScanned = false; // Reset flag
      exceptErrorPosition = true;
      saveExceptErrorFlag();
      currentRFID = AGV_STATE_WAREHOUSE;
      agvMode(AGV_STATE_WAREHOUSE);
      softStartTime = millis();
      softStartActive = true;
      pidSpeed = maxMotorRpm / 2;  // 🔧 FIX #2: 50% initial speed (was 25%, was baseSpeed)
      return;
    } else if (isRfidMatch(currentRfid, getRfidForStation(1)) && exceptErrorPosition != false) {
      // newRfidScanned = false; // Reset flag
      exceptErrorPosition = false;
      saveExceptErrorFlag();
      return;
    }else  if (isRfidMatch(currentRfid, terminalDropRfidId) && currentRFID != AGV_STATE_TERMINAL_DROP) {
      newRfidScanned = false; // Reset flag
      currentRFID = AGV_STATE_TERMINAL_DROP;
      agvMode(AGV_STATE_TERMINAL_DROP);
      return;
    }
  } else if (currentRfid.length() > 0 && newRfidScanned && isRfidMatch(currentRfid, getRfidForStation(1))) {
    newRfidScanned = false; // Reset flag
    if (currentRFID == AGV_STATE_WAREHOUSE){
      exceptErrorPosition = false;
    } else {
      exceptErrorPosition = true;
    }
    currentRFID = AGV_STATE_NULL;
    saveExceptErrorFlag();
  }

  // Cek apakah ada RFID yang terbaca untuk stasiun
  int currentStation = getStationFromLastRfid();

  // Jika ada stasiun yang terdeteksi, cek apakah ada di target list
  if (currentStation != -1) {
    // Cari apakah stasiun ini ada di targetStationsList
    for (int i = 0; i < targetStationsList.size(); i++) {
      if (targetStationsList[i] == currentStation) {
        removeTargetStationById(currentStation);
        stopMusic();
        agvMode(AGV_STATE_STATION);
        return;
      }
    }
  }

  // Jika tidak ada hambatan dan bukan stasiun target, lanjutkan bergerak
  if (!obstacleDetected) {
    music(MUSIC_MODE_ON);
    if (exceptErrorPosition && totalSensorAktif > 7) {
      pidLinefollower(0, PID_MODE_MAJU);
    } else {
      if (targetStationsList.size() != 0 || currentRFID == AGV_STATE_TERMINAL_PICKUP) {
        pidLinefollower(errorValue, PID_MODE_MAJU);  // Error dari sensor magnet
      } else {
        pidLinefollower(errorValue, PID_MODE_MAJU);  // Error dari sensor magnet
      }
    }
  }
}

// Helper function untuk memeriksa kecocokan RFID
bool isRfidMatch(const String& currentRfid, const String& targetRfid) {
  return targetRfid.length() > 0 && currentRfid.equals(targetRfid);
}

// Fungsi gabungan untuk memuat semua state AGV dari Preferences
void loadAllAGVStatesFromPreferences() {
  preferences.begin("agv-state", true);

  // Load current state
  String currentStateString = preferences.getString("current_state", "NULL");
  currentStateAgv = stringToAgvState(currentStateString);

  preferences.end();
  
  // Reset warehouse state jika AGV di-load dalam state WAREHOUSE
  // Ini mencegah AGV langsung jalan setelah restart
  if (currentStateAgv == AGV_STATE_WAREHOUSE) {
    resetWarehouseState();
  }
}

String agvStateToString(AgvState state) {
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

  // Serial.println() - removed for production
}

// Fungsi untuk mengkonversi string ke AgvState
// Fungsi ini mengkonversi representasi string dari status AGV kembali ke nilai AgvState.
AgvState stringToAgvState(String stateString) {
  if (stateString == "MOVE_FORWARD") {
    return AGV_STATE_MOVE_FORWARD;
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