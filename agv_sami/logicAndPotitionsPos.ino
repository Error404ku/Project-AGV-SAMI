extern int totalSensorAktif;  // counter sensor aktif
extern int errorValue;        // nilai error PID
// String statusJalan; // Replaced with optimized version in performance_optimization.ino
/***********************************************************
 *  GLOBAL STATE                                          *
 ***********************************************************/
// Removed unused variables: station, sudahDeteksiStasiun, targetStation[], targetStationFromKomputer[], totalTarget, indexTarget

// ― Warehouse dan Ujung RFID IDs ―
String warehouseRfidId = "";
// Removed unused variables: stationListReceived, waitingForStart

// ― Movement flags ― (keeping only used variables)
bool modeBerhenti = true;  // start dalam keadaan berhenti
// Removed unused variables: modeTerminal, modeWarehouse, modeStation, modeMaju, modeMundur, force

// Global StateMode variable definition
StateMode currentStateMode = STATE_MODE_BERHENTI;

// Global AGV state tracking variables definition
AgvState lastStateAgv = AGV_STATE_STOP;


/***********************************************************
 *  WAREHOUSE & UJUNG RFID FUNCTIONS                     *
 ***********************************************************/
void loadWarehouseUjungRfid() {
  preferences.begin("warehouse-ujung", false);
  warehouseRfidId = preferences.getString("warehouseRfid", "");
  ujungRfidId = preferences.getString("ujungRfid", "");
  preferences.end();
  
  // Sinkronisasi dengan data dari rfidWarehouseList dan rfidUjungList
  if (warehouseRfidId.length() == 0 && rfidWarehouseCount > 0 && rfidWarehouseList[0].isActive) {
    warehouseRfidId = rfidWarehouseList[0].rfidId;
    saveWarehouseRfid(warehouseRfidId);
  }
  
  if (ujungRfidId.length() == 0 && rfidUjungCount > 0 && rfidUjungList[0].isActive) {
    ujungRfidId = rfidUjungList[0].rfidId;
    saveUjungRfid(ujungRfidId);
  }
  
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

/***********************************************************
 *  TERMINAL DROP & PICKUP RFID FUNCTIONS                *
 ***********************************************************/
void loadTerminalRfid() {
  preferences.begin("terminal-rfid", false);
  terminalDropRfidId = preferences.getString("terminalDropRfid", "");
  terminalPickUpRfidId = preferences.getString("terminalPickUpRfid", "");
  preferences.end();
  
  Serial.println("Loaded Terminal Drop RFID: " + terminalDropRfidId);
  Serial.println("Loaded Terminal PickUp RFID: " + terminalPickUpRfidId);
}

void saveTerminalDropRfid(String rfidId) {
  preferences.begin("terminal-rfid", false);
  preferences.putString("terminalDropRfid", rfidId);
  preferences.end();
  terminalDropRfidId = rfidId;
  Serial.println("Terminal Drop RFID saved: " + rfidId);
}

void saveTerminalPickUpRfid(String rfidId) {
  preferences.begin("terminal-rfid", false);
  preferences.putString("terminalPickUpRfid", rfidId);
  preferences.end();
  terminalPickUpRfidId = rfidId;
  Serial.println("Terminal PickUp RFID saved: " + rfidId);
}
