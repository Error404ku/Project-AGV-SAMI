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
  hook("turun");
  delay(2000);
  pidLinefollower(2, PID_MODE_MAJU);
  String currentRfid = String(lastScannedRfidOptimized);
  if (currentRfid.length() > 0) {
    agvMode(AGV_STATE_WAREHOUSE);
    lastStateAGV(AGV_STATE_TERMINAL);
    return;
  }
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
          return;
        }
      }
    }
  }

  // Cek apakah RFID terminal terdeteksi
  String currentRfid = String(lastScannedRfidOptimized);
  if (currentRfid.length() > 0) {
    agvMode(AGV_STATE_TERMINAL);
    lastStateAGV(AGV_STATE_MOVE_BACKWARD);
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
    } else if (lastState == AGV_STATE_TERMINAL) {
        lastStateAgv = AGV_STATE_TERMINAL;
    } else if (lastState == AGV_STATE_WAREHOUSE) {
        lastStateAgv = AGV_STATE_WAREHOUSE;
    } else if (lastState == AGV_STATE_STATION) {
        lastStateAgv = AGV_STATE_STATION;
    } else if (lastState == AGV_STATE_STOP) {
        lastStateAgv = AGV_STATE_STOP;
    }
}