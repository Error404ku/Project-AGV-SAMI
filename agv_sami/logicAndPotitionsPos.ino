// ===================================================================
// RFID TERMINAL VARIABLES SUDAH DIPINDAHKAN KE config.h
// ===================================================================

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
}

void saveWarehouseRfid(String rfidId) {
  preferences.begin("warehouse-ujung", false);
  preferences.putString("warehouseRfid", rfidId);
  preferences.end();
  warehouseRfidId = rfidId;
}

void saveUjungRfid(String rfidId) {
  preferences.begin("warehouse-ujung", false);
  preferences.putString("ujungRfid", rfidId);
  preferences.end();
  ujungRfidId = rfidId;
}



/***********************************************************
 *  TERMINAL DROP & PICKUP RFID FUNCTIONS                *
 ***********************************************************/
void loadTerminalRfid() {
  preferences.begin("terminal-rfid", false);
  terminalDropRfidId = preferences.getString("terminalDrop", "");
  terminalPickUpRfidId = preferences.getString("terminalPickUp", "");
  preferences.end();
}

void saveTerminalDropRfid(String rfidId) {
  preferences.begin("terminal-rfid", false);
  preferences.putString("terminalDrop", rfidId);
  preferences.end();
  terminalDropRfidId = rfidId;
}

void saveTerminalPickUpRfid(String rfidId) {
  preferences.begin("terminal-rfid", false);
  preferences.putString("terminalPickUp", rfidId);
  preferences.end();
  terminalPickUpRfidId = rfidId;
}

/***********************************************************
 *  UJUNG RFID SLOW MODE FUNCTIONS                       *
 ***********************************************************/
void loadUjungSlowMode() {
  preferences.begin("agv-ujung", true);
  isUjungSlowMode = preferences.getBool("slowMode", false);
  preferences.end();
}

void saveUjungSlowMode() {
  preferences.begin("agv-ujung", false);
  preferences.putBool("slowMode", isUjungSlowMode);
  preferences.end();
}

void resetUjungSlowMode() {
  isUjungSlowMode = false;
  saveUjungSlowMode();
}
