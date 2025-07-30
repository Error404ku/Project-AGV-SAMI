void agvMode(AgvState state) {
  if (state == AGV_STATE_MOVE_FORWARD) {
    agvMoveForward();
  } else if (state == AGV_STATE_MOVE_BACKWARD) {
    agvMoveBackward();
  } else if (state == AGV_STATE_STOP) {
    agvStop();
  } else if (state == AGV_STATE_TERMINAL) {
    agvTerminal();
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
  pidLinefollower(2, PID_MODE_STOPPELANPELAN);
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
        }
      }
    }
  }

  // Cek apakah RFID ujung terdeteksi
  String currentRfid = String(lastScannedRfidOptimized);
  if (currentRfid.length() > 0 && currentRfid.equals(terminalDropRfidId) && terminalDropRfidId.length() > 0) {
    agvMode(AGV_STATE_TERMINAL);
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
    }
}