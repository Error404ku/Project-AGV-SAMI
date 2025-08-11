extern int totalSensorAktif;  // counter sensor aktif
// ===================================================================
// RFID TERMINAL VARIABLES SUDAH DIPINDAHKAN KE config.h
// ===================================================================




/***********************************************************
 *  WAREHOUSE & UJUNG RFID FUNCTIONS                     *
 ***********************************************************/

// Load RFID Pertigaan from preferences
void loadRfidPertigaanFromPreferences() {
  preferences.begin("rfid_pertigaan", false);
  pertigaanRfidId = preferences.getString("pertigaanRfid", "");
  preferences.end();
}

void loadWarehouseUjungRfid() {
  preferences.begin("warehouse-ujung", false);
  warehouseRfidId = preferences.getString("warehouseRfid", "");
  ujungRfidId = preferences.getString("ujungRfid", "");
  pertigaanRfidId = preferences.getString("pertigaanRfid", "");
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
  Serial.println("Loaded Pertigaan RFID: " + pertigaanRfidId);
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

void savePertigaanRfid(String rfidId) {
  preferences.begin("warehouse-ujung", false);
  preferences.putString("pertigaanRfid", rfidId);
  preferences.end();
  pertigaanRfidId = rfidId;
  Serial.println("Pertigaan RFID saved: " + rfidId);
}

/***********************************************************
 *  TERMINAL DROP & PICKUP RFID FUNCTIONS                *
 ***********************************************************/
void loadTerminalRfid() {
  preferences.begin("terminal-rfid", false);
  terminalDropRfidId = preferences.getString("terminalDrop", "");
  terminalPickUpRfidId = preferences.getString("terminalPickUp", "");
  preferences.end();
  
  Serial.println("Loaded Terminal Drop RFID: " + terminalDropRfidId);
  Serial.println("Loaded Terminal PickUp RFID: " + terminalPickUpRfidId);
}

void saveTerminalDropRfid(String rfidId) {
  preferences.begin("terminal-rfid", false);
  preferences.putString("terminalDrop", rfidId);
  preferences.end();
  terminalDropRfidId = rfidId;
  Serial.println("Terminal Drop RFID saved: " + rfidId);
}

void saveTerminalPickUpRfid(String rfidId) {
  preferences.begin("terminal-rfid", false);
  preferences.putString("terminalPickUp", rfidId);
  preferences.end();
  terminalPickUpRfidId = rfidId;
  Serial.println("Terminal PickUp RFID saved: " + rfidId);
}

/***********************************************************
 *  RFID MAJU FUNCTIONS                                   *
 ***********************************************************/
void loadRfidMaju() {
  preferences.begin("rfid-maju", false);
  rfidMajuId = preferences.getString("rfidMaju", "");
  preferences.end();
}

void saveRfidMaju(String rfidId) {
  preferences.begin("rfid-maju", false);
  preferences.putString("rfidMaju", rfidId);
  preferences.end();
  rfidMajuId = rfidId;
}
