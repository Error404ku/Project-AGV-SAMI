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

// ― Modes (hanya SATU TRUE sekaligus) ―
bool modeTerminal = true;   // mulai di terminal & diam
bool modeWarehouse = false;
bool modeStation = false;

// ― Movement flags ―
bool modeMaju = false;
bool modeMundur = false;
bool modeBerhenti = true;  // start dalam keadaan berhenti
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
 *  MODE HELPERS                                          *
 ***********************************************************/
inline void clearMovement() {
  modeMaju = modeMundur = false;
}
inline void setModeTerminal() {
  modeTerminal = true;
  modeWarehouse = modeStation = false;
}
int currentMillis = 0;
inline void setModeWarehouse() {
  modeWarehouse = true;
  modeTerminal = modeStation = false;
}
inline void setModeStation() {
  modeStation = true;
  modeTerminal = modeWarehouse = false;
}

/***********************************************************
 *  TRANSITIONS (MASUK / KELUAR)                          *
 ***********************************************************/
void inTerminal() {
  clearMovement();
  modeBerhenti = true;
}
void outTerminal() {
  setModeWarehouse();
  modeMaju = true;
  // ("maju");
  force = true;
}

void inWarehouse() {
  music("komputer");
  clearMovement();
  clearStationsData();
  modeBerhenti = true;
}

void outWarehouse() {
  setModeStation();
  sortStationsList();
  // modeMundur = false;
  modeMaju = true;
  // ("maju");
  force = true;
  statusMusic = false;
}

void inStation() {
  music("station");
  clearMovement();
  modeBerhenti = true;
}
void outStation() {
  // Check if this is the last station in the list
  if (indexTarget >= stationsList.size()) {
    Serial.println("Last station reached via outStation - calling ujungStation!");
    ujungStation();
  } else {
    // modeMaju = true;
    changeStateMode("maju");
    force = true;
    sudahStopPelanPelan = false;
  }
  statusMusic = false;
}
void ujungStation() {  // ujung station → mundur ke warehouse
  modeMaju = false;
  // modeMundur = true;
  changeStateMode("mundur");
  force = true;
  pidLinefollower(errorValue, "FORCEMUNDUR");
  // delay(1000);
  setModeWarehouse();
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
    modeMaju = true;
    modeMundur = false;
    modeBerhenti = false;
    Serial.println("[INFO] Mode berubah: MAJU");
  } else if (mode == "mundur") {
    modeMaju = false;
    modeMundur = true;
    modeBerhenti = false;
    Serial.println("[INFO] Mode berubah: MUNDUR");
  } else if (mode == "berhenti") {
    modeMaju = false;
    modeMundur = false;
    modeBerhenti = true;
    Serial.println("[INFO] Mode berubah: BERHENTI");
  } else if (mode == "forcemaju") {
    modeMaju = true;
    modeMundur = false;
    modeBerhenti = false;
    force = true;
    Serial.println("[INFO] Mode berubah: FORCE MAJU");
  } else if (mode == "forcemundur") {
    modeMaju = false;
    modeMundur = true;
    modeBerhenti = false;
    force = true;
    Serial.println("[INFO] Mode berubah: FORCE MUNDUR");
  } else {
    Serial.println("[WARNING] Mode tidak dikenal: " + mode);
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
    modeBerhenti = true;
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

    if ((kanan || kiri) && !modeBerhenti) {
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

  // ― Ujung station: pindah ke warehouse (mundur) jika tidak ada ujung RFID ―
  if (sensorkebacasemua && modeMaju && !force && indexTarget >= stationsList.size()) {
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

/***********************************************************
 *  BUTTON HANDLER (PS3)                                  *
 ***********************************************************/
void tombolAgv() {
  unsigned long ms = millis();

  if (START() && (ms - lastXPress >= xDelay)) {
    lastXPress = ms;
    
    if (modeTerminal && stationListReceived && waitingForStart) {
      // Mulai perjalanan dari terminal ke station pertama
      Serial.println("Starting journey from terminal to first station");
      setModeStation();
      changeStateMode("maju");
      force = true;
      waitingForStart = false;
      indexTarget = 0;  // Reset ke station pertama
    } else if (modeStation && modeBerhenti) {
      // Lanjut ke station berikutnya atau ke ujung
      indexTarget++;
      if (indexTarget >= stationsList.size()) {
        Serial.println("All stations completed - heading to ujung");
        // Tetap di mode station untuk mencari ujung
        changeStateMode("maju");
        force = true;
      } else {
        Serial.println("Moving to next station: " + String(stationsList[indexTarget]));
        changeStateMode("maju");
        force = true;
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
  tengahAktif = jumlahMagnet[7] && jumlahMagnet[8];
  kananHilang = jumlahMagnet[14] || jumlahMagnet[15];
  kiriHilang = jumlahMagnet[0] || jumlahMagnet[2];
  sensorkebacasemua = (totalSensorAktif > 14);

  tombolAgv();

  // ― Jalankan handler mode aktif ―
  if (modeTerminal)
    pembacaanTerminal();
  else if (modeWarehouse)
    pembacaanWarehouse();
  else if (modeStation)
    pembacaanStation();

  if (force)
    modeBerhenti = false;
  // ― Prioritas gerakan global ―
  if (strcmp(getStatusJalan(), "BERHENTI") != 0) {
    modeBerhenti = false;
  }
  if (modeBerhenti) {
    if (modeStation) {
      pidLinefollower(errorValue, "STOPPELANPELAN");
    } else {
      pidLinefollower(errorValue, "STOP");
    }
    return;
  }
  if (modeMaju && !force)
    pidLinefollower(errorValue, "MAJU");
  if (modeMundur && !force)
    pidLinefollower(errorValue, "MUNDUR");
}

void displayLogicAgv() {

  if (modeMaju) {
    setStatusJalan("MAJU");
  } else if (modeMundur) {
    setStatusJalan("MUNDUR");
  } else if (modeBerhenti) {
    setStatusJalan("BERHENTI");
  } else {
    setStatusJalan("BERHENTI");
    modeBerhenti = true;
  }
  // Tentukan mode aktif
  if (modeTerminal) {
    setCurrentMode("TERMINAL");
  } else if (modeWarehouse) {
    setCurrentMode("WAREHOUSE");
  } else if (modeStation) {
    setCurrentMode("STATION");
  } else {
    setCurrentMode("UNKNOWN");
    // modeTerminal = true;
    modeWarehouse = true;
  }

  // Display on row 3 (last available row)
  lcd.setCursor(0, 3);
  
  // Use optimized char arrays instead of String operations
  char displayBuffer[21]; // LCD width + null terminator
  snprintf(displayBuffer, sizeof(displayBuffer), "%.8s %.7s", getCurrentMode(), getStatusJalan());
  lcd.print(displayBuffer);

  if (modeStation) {
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
