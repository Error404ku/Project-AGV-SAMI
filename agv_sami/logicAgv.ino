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
  }
  } else if (state == AGV_STATE_WAREHOUSE) {
    agvWarehouse();
  } else if (state == AGV_STATE_STATION) {
    agvStation();
  } else {
    agvStop();
  }
}

void agvWarehouse() {
  agvStop();
  if (START()) {
    agvMode(AGV_STATE_MOVE_FORWARD);
  }
}

void agvStation() {
  if (START()) {
    if (lastStateAgv == AGV_STATE_MOVE_FORWARD) {
      agvMode(AGV_STATE_MOVE_BACKWARD);
    }else if (lastStateAgv == AGV_STATE_MOVE_BACKWARD) {
      agvMode(AGV_STATE_MOVE_FORWARD);
    }
  }
}

void agvTerminal() {
  agvStop();

}

void agvStop() {
  pwmMotor(0, 0);
}

void agvMoveForward() {
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
  // Cek apakah ada RFID warehouse yang terdeteksi
  String currentRfid = String(lastScannedRfidOptimized);
  
  // Cek apakah mencapai warehouse
  for (int i = 0; i < rfidWarehouseCount; i++) {
    if (rfidWarehouseList[i].isActive && currentRfid.equals(rfidWarehouseList[i].rfidId)) {
      agvMode(AGV_STATE_WAREHOUSE);
      lastStateAGV(AGV_STATE_MOVE_BACKWARD);
      return;
    }
  }

  // Jika tidak ada hambatan, lanjutkan bergerak mundur
  if (!obstacleDetected) {
    pidLinefollower(2, PID_MODE_MUNDUR);
  }
}

void lastStateAGV(AgvState lastState){
    if (lastState == AGV_STATE_MOVE_FORWARD) {
        lastStateAgv = AGV_STATE_MOVE_FORWARD;
    } else if (lastState == AGV_STATE_MOVE_BACKWARD) {
        lastStateAgv = AGV_STATE_MOVE_BACKWARD;
    }
}