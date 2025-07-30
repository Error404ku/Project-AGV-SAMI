extern int totalSensorAktif;  // counter sensor aktif
extern int errorValue;        // nilai error PID
// String statusJalan; // Replaced with optimized version in performance_optimization.ino
/***********************************************************
 *  GLOBAL STATE                                          *
 ***********************************************************/
int station = 0;
bool sudahDeteksiStasiun = false;
/* ---------- Konfigurasi rute stasiun ---------- */
int targetStation[] = { 2, 3 };  // <-- DEFINISI nyata!
int targetStationFromKomputer[] = { 1, 2, 4, 5 };
const int totalTarget = sizeof(targetStation) / sizeof(targetStation[0]);
int indexTarget = 0;

// ― Warehouse dan Ujung RFID IDs ―
String warehouseRfidId = "";
String ujungRfidId = "";
bool stationListReceived = false;  // Flag untuk menandai apakah stationList sudah diterima
bool waitingForStart = true;       // Flag untuk menunggu tombol start

// Current states - actual definitions
AGVMode currentAGVMode = MODE_TERMINAL;
MovementState currentMovement = MOVEMENT_STOP;
bool force = false;        // override manual

// ― Derived sensor flags ―
bool tengahAktif = false;
bool kananHilang = false;
bool kiriHilang = false;
bool sensorkebacasemua = false;

// ― Debounce tombol X ―
unsigned long lastXPress = 0;
const unsigned long xDelay = 200;
extern bool sudahStopPelanPelan;

/***********************************************************
 *  STATE MANAGEMENT HELPERS                              *
 ***********************************************************/

// Movement state helpers
void setMovementState(MovementState newState) {
  currentMovement = newState;
  force = (newState == MOVEMENT_FORCE_FORWARD || newState == MOVEMENT_FORCE_BACKWARD);
}

void clearMovement() {
  currentMovement = MOVEMENT_STOP;
  force = false;
}

// AGV mode helpers
void setAGVMode(AGVMode newMode) {
  currentAGVMode = newMode;
}

bool isMode(AGVMode mode) {
  return currentAGVMode == mode;
}

bool isMoving(MovementState state) {
  return currentMovement == state;
}

// Legacy compatibility helpers
bool modeTerminal() { return currentAGVMode == MODE_TERMINAL; }
bool modeWarehouse() { return currentAGVMode == MODE_WAREHOUSE; }
bool modeStation() { return currentAGVMode == MODE_STATION; }
bool modeMaju() { return currentMovement == MOVEMENT_FORWARD || currentMovement == MOVEMENT_FORCE_FORWARD; }
bool modeMundur() { return currentMovement == MOVEMENT_BACKWARD || currentMovement == MOVEMENT_FORCE_BACKWARD; }
bool modeBerhenti() { return currentMovement == MOVEMENT_STOP; }

int currentMillis = 0;

/***********************************************************
 *  TRANSITIONS (MASUK / KELUAR)                          *
 ***********************************************************/
void inTerminal() {
  setAGVMode(MODE_TERMINAL);
  setMovementState(MOVEMENT_STOP);
}

void outTerminal() {
  setAGVMode(MODE_WAREHOUSE);
  setMovementState(MOVEMENT_FORCE_FORWARD);
}

void inWarehouse() {
  music("komputer");
  setAGVMode(MODE_WAREHOUSE);
  setMovementState(MOVEMENT_STOP);
  clearStationsData();
}

void outWarehouse() {
  setAGVMode(MODE_STATION);
  sortStationsList();
  setMovementState(MOVEMENT_FORCE_FORWARD);
  statusMusic = false;
}

void inStation() {
  music("station");
  setAGVMode(MODE_STATION);
  setMovementState(MOVEMENT_STOP);
}

void outStation() {
  // Check if this is the last station in the list
  if (indexTarget >= stationsList.size()) {
    Serial.println("Last station reached via outStation - calling ujungStation!");
    ujungStation();
  } else {
    setMovementState(MOVEMENT_FORCE_FORWARD);
    sudahStopPelanPelan = false;
  }
  statusMusic = false;
}

void ujungStation() {  // ujung station → mundur ke warehouse
  setMovementState(MOVEMENT_FORCE_BACKWARD);
  pidLinefollower(errorValue, "FORCEMUNDUR");
  setAGVMode(MODE_WAREHOUSE);
}

/***********************************************************
 *  STATE MODE FUNCTIONS                                  *
 ***********************************************************/

/**
 * Mengubah state/mode operasional AGV
 * @param mode String mode yang akan diaktifkan ("maju", "mundur", "berhenti", "forcemaju", "forcemundur")
 */
void changeStateMode(String mode) {
  if (mode == "maju") {
    setMovementState(MOVEMENT_FORWARD);
    Serial.println("[INFO] Mode berubah: MAJU");
  } else if (mode == "mundur") {
    setMovementState(MOVEMENT_BACKWARD);
    Serial.println("[INFO] Mode berubah: MUNDUR");
  } else if (mode == "berhenti") {
    setMovementState(MOVEMENT_STOP);
    Serial.println("[INFO] Mode berubah: BERHENTI");
  } else if (mode == "forcemaju") {
    setMovementState(MOVEMENT_FORCE_FORWARD);
    Serial.println("[INFO] Mode berubah: FORCE MAJU");
  } else if (mode == "forcemundur") {
    setMovementState(MOVEMENT_FORCE_BACKWARD);
    Serial.println("[INFO] Mode berubah: FORCE MUNDUR");
  } else {
    Serial.println("[ERROR] Mode tidak dikenal: " + mode);
  }
  
  // Reset detection flag when changing movement state
  if (mode != "berhenti") {
    sudahDeteksiStasiun = false;
  }
}

/***********************************************************
 *  MODE HANDLERS                                         *
 ***********************************************************/
void pembacaanTerminal() {
  // AGV menunggu di terminal sampai stationList diterima dan tombol start ditekan
  if (!stationListReceived) {
    // Cek apakah stationList sudah diterima dari server/komputer
    if (stationsList.size() > 0) {
      stationListReceived = true;
      Serial.println("StationList received! Ready to start journey.");
      Serial.print("Stations to visit: ");
      for (size_t i = 0; i < stationsList.size(); i++) {
        Serial.print(stationsList[i]);
        if (i < stationsList.size() - 1) Serial.print(", ");
      }
      Serial.println();
    }
  }
  
  if (waitingForStart) {
    // AGV tetap berhenti di terminal menunggu tombol start
    setMovementState(MOVEMENT_STOP);
    pidLinefollower(errorValue, "STOP");
    return;
  }
  
  // Logika lama untuk kembali ke terminal (mode mundur)
  if (!sensorkebacasemua) {
    force = false;
  }
  if (sensorkebacasemua && !force) {
    inTerminal();
    return;
  }
  if (sensorkebacasemua && modeMundur && force) {
    pidLinefollower(errorValue, "FORCEMUNDUR");
    return;
  }
  if (modeMundur) {
    pidLinefollower(errorValue, "MUNDUR");
    return;
  }

  pidLinefollower(errorValue, "MAJU");
}

void pembacaanWarehouse() {

  if (sensorkebacasemua && force && modeMaju) {
    pidLinefollower(errorValue, "FORCEMAJU");
    return;
  }
  // Serial.println(force);
  if (sensorkebacasemua && modeMaju && !force) {
    inWarehouse();
    return;
  }
  if (!sensorkebacasemua && force) {
    force = false;
  }
  if (sensorkebacasemua && modeMundur && !force) {
    inWarehouse();
    // pidLinefollower(errorValue, "FORCEMUNDUR");
    return;
  }
  // ― Deteksi marker tengah ―
  if (tengahAktif && !force && modeMundur) {
    bool kanan = (kananHilang && station % 2 == 0);
    bool kiri = (kiriHilang && station % 2 == 1);

    if ((kanan || kiri) && !isMoving(MOVEMENT_STOP)) {
        errorValue = 0;
      }
    pidLinefollower(errorValue, "MUNDUR");
    return;
  }
  pidLinefollower(errorValue, "MAJU");
}

void pembacaanStation() {
  // ― RFID Detection (Priority) - Independent of sensor conditions ―
  if (!sudahDeteksiStasiun) {
    handleRfidDetection();
  }

  // ― Ujung station: pindah ke warehouse (mundur) jika tidak ada ujung RFID ―
  if (sensorkebacasemua && isMoving(MOVEMENT_FORWARD) && !force && indexTarget >= stationsList.size()) {
    Serial.println("Reached end without ujung RFID - returning to warehouse");
    ujungStation();
    return;
  }
  
  // ― Keluar station dengan tombol (FORCEMAJU) ―
  if ((kananHilang || kiriHilang) && force) {
    return pidLinefollower(errorValue, "FORCEMAJU");
  }

  // ― Reset bila keluar garis ―
  if (!sensorkebacasemua) {
    force = false;
    sudahDeteksiStasiun = false;
  }
  
  pidLinefollower(errorValue, "MAJU");
}

// Helper function for RFID detection
void handleRfidDetection() {
  int detectedStationId = getStationFromLastRfid();
  String currentRfid = String(lastScannedRfidOptimized);
  
  if (detectedStationId > 0 || currentRfid.length() > 0) {
    errorValue = 0;
    sudahDeteksiStasiun = true;
    
    // Check if this is warehouse RFID
    if (currentRfid.equals(warehouseRfidId) && warehouseRfidId.length() > 0) {
      Serial.println("Warehouse RFID detected - entering warehouse mode");
      inWarehouse();
      return;
    }
    
    // Check if this is ujung RFID
    if (currentRfid.equals(ujungRfidId) && ujungRfidId.length() > 0) {
      Serial.println("Ujung RFID detected - returning to warehouse");
      ujungStation();
      return;
    }
    
    // Check if this is a target station RFID
    if (detectedStationId > 0 && indexTarget < stationsList.size()) {
      if (stationsList[indexTarget] == detectedStationId) {
        Serial.println("Target station " + String(detectedStationId) + " reached!");
        station = detectedStationId;
        inStation();
        return;
      } else {
        Serial.println("Non-target station " + String(detectedStationId) + " detected - continue");
      }
    }
    
    // If no specific action needed, continue moving
    pidLinefollower(errorValue, "MAJU");
    return;
  }
}

/***********************************************************
 *  BUTTON HANDLER (PS3)                                  *
 ***********************************************************/
void tombolAgv() {
  unsigned long ms = millis();

  if (START() && (ms - lastXPress >= xDelay)) {
    lastXPress = ms;
    
    if (isMode(MODE_TERMINAL) && stationListReceived && waitingForStart) {
      // Mulai perjalanan dari terminal ke station pertama
      Serial.println("Starting journey from terminal to first station");
      setAGVMode(MODE_STATION);
      changeStateMode("maju");
      force = true;
      waitingForStart = false;
      indexTarget = 0;  // Reset ke station pertama
    } else if (isMode(MODE_STATION) && isMoving(MOVEMENT_STOP)) {
      // Lanjut ke station berikutnya atau ke ujung
      indexTarget++;
      if (indexTarget >= stationsList.size()) {
        Serial.println("All stations completed - heading to ujung");
        // Tetap di mode station untuk mencari ujung
        setMovementState(MOVEMENT_FORCE_FORWARD);
      } else {
        Serial.println("Moving to next station: " + String(stationsList[indexTarget]));
        setMovementState(MOVEMENT_FORCE_FORWARD);
      }
      sudahStopPelanPelan = false;
    }
  }
}

/***********************************************************
 *  MAIN LOGIC – PANGGIL DI loop()                        *
 ***********************************************************/
void logicAgv() {
  // ― Update processed sensor flags ―
  tengahAktif = jumlahMagnet[SENSOR_CENTER_LEFT] && jumlahMagnet[SENSOR_CENTER_RIGHT];
  kananHilang = jumlahMagnet[SENSOR_RIGHT_1] || jumlahMagnet[SENSOR_RIGHT_2];
  kiriHilang = jumlahMagnet[SENSOR_LEFT_1] || jumlahMagnet[SENSOR_LEFT_2];
  sensorkebacasemua = (totalSensorAktif > SENSOR_THRESHOLD_ALL_ACTIVE);

  tombolAgv();

  // ― Jalankan handler mode aktif ―
  if (modeTerminal)
    pembacaanTerminal();
  else if (modeWarehouse)
    pembacaanWarehouse();
  else if (modeStation)
    pembacaanStation();

  // ― Prioritas gerakan global ―
  if (modeBerhenti()) {
    if (modeStation()) {
      pidLinefollower(errorValue, "STOPPELANPELAN");
    } else {
      pidLinefollower(errorValue, "STOP");
    }
    return;
  }
  
  // ― Main logic based on current mode ―
  switch (currentAGVMode) {
    case MODE_TERMINAL:
      // Terminal mode: stay put until START
      if (isMoving(MOVEMENT_STOP)) {
        pidLinefollower(errorValue, "BERHENTI");
      }
      break;
      
    case MODE_WAREHOUSE:
      // Warehouse mode: handle movement
      if (isMoving(MOVEMENT_FORCE_FORWARD)) {
        pidLinefollower(errorValue, "FORCEMAJU");
      } else if (isMoving(MOVEMENT_FORCE_BACKWARD)) {
        pidLinefollower(errorValue, "FORCEMUNDUR");
      } else if (isMoving(MOVEMENT_STOP)) {
        pidLinefollower(errorValue, "BERHENTI");
      }
      break;
      
    case MODE_STATION:
      // Station mode: handle station visits
      if (isMoving(MOVEMENT_FORCE_FORWARD)) {
        pidLinefollower(errorValue, "FORCEMAJU");
      } else if (isMoving(MOVEMENT_FORCE_BACKWARD)) {
        pidLinefollower(errorValue, "FORCEMUNDUR");
      } else if (isMoving(MOVEMENT_STOP)) {
        pidLinefollower(errorValue, "BERHENTI");
      }
      break;
  }
  
  // Handle non-force movements
  if (isMoving(MOVEMENT_FORWARD) && !force)
    pidLinefollower(errorValue, "MAJU");
  else if (isMoving(MOVEMENT_BACKWARD) && !force)
    pidLinefollower(errorValue, "MUNDUR");
  else if (isMoving(MOVEMENT_STOP) && !force)
    pidLinefollower(errorValue, "BERHENTI");
}

void displayLogicAgv() {

  if (modeMaju()) {
    setStatusJalan("MAJU");
  } else if (modeMundur()) {
    setStatusJalan("MUNDUR");
  } else if (modeBerhenti()) {
    setStatusJalan("BERHENTI");
  } else {
    setStatusJalan("BERHENTI");
    setMovementState(MOVEMENT_STOP);
  }
  // Tentukan mode aktif
  if (modeTerminal()) {
    setCurrentMode("TERMINAL");
  } else if (modeWarehouse()) {
    setCurrentMode("WAREHOUSE");
  } else if (modeStation()) {
    setCurrentMode("STATION");
  } else {
    setCurrentMode("UNKNOWN");
    setAGVMode(MODE_WAREHOUSE);
  }

  // Display on row 3 (last available row)
  lcd.setCursor(0, 3);
  
  // Use optimized char arrays instead of String operations
  char displayBuffer[21]; // LCD width + null terminator
  snprintf(displayBuffer, sizeof(displayBuffer), "%.8s %.7s", getCurrentMode(), getStatusJalan());
  lcd.print(displayBuffer);

  if (modeStation()) {
    // Clear part of row 3 and show station info
    lcd.setCursor(0, 3);
    lcd.print(station);
    if (indexTarget < stationsList.size()) {
      lcd.print(stationsList[indexTarget]);
    } else {
      lcd.print("END");
    }
  }
}


/***********************************************************
 *  WAREHOUSE & UJUNG RFID FUNCTIONS                     *
 ***********************************************************/
void loadWarehouseUjungRfid() {
  preferences.begin("warehouse-ujung", false);
  warehouseRfidId = preferences.getString("warehouseRfid", "");
  ujungRfidId = preferences.getString("ujungRfid", "");
  preferences.end();
  
  Serial.println("Loaded Warehouse RFID: " + warehouseRfidId);
  Serial.println("Loaded Ujung RFID: " + ujungRfidId);
}

void saveWarehouseRfid(String rfidId) {
  preferences.begin("warehouse-ujung", false);
  preferences.putString("warehouseRfid", rfidId);
  preferences.end();
  warehouseRfidId = rfidId;
  Serial.println("Warehouse RFID saved: " + rfidId);
}

void saveUjungRfid(String rfidId) {
  preferences.begin("warehouse-ujung", false);
  preferences.putString("ujungRfid", rfidId);
  preferences.end();
  ujungRfidId = rfidId;
  Serial.println("Ujung RFID saved: " + rfidId);
}
