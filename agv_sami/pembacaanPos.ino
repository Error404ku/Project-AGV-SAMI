extern int totalSensorAktif;  // counter sensor aktif
extern int errorValue;        // nilai error PID
String statusJalan;
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

// ― Modes (hanya SATU TRUE sekaligus) ―
bool modeTerminal = false;  // mulai di terminal & diam
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
  // modeMaju = true;
  // changeStateMode("maju");
  force = true;
}

// void inWarehouse() {
//   if (modeMundur) {
//     force = true;
//     if (!modeStation) {
//       setModeTerminal();  // balik arah, pulang ke terminal
//     }
//   } else {
//     clearMovement();
//     modeBerhenti = true;
//   }
// }
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
  // modeMaju = true;
  // changeStateMode("maju");
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
    // Serial.println("Last station reached via outStation - calling ujungStation!");
    ujungStation();
  } else {
    // modeMaju = true;
    // changeStateMode("maju");
    force = true;
    sudahStopPelanPelan = false;
  }
  statusMusic = false;
}
void ujungStation() {  // ujung station → mundur ke warehouse
  // modeMaju = false;
  // modeMundur = true;
  // changeStateMode("mundur");
  force = true;
  pidLinefollower(errorValue, "FORCEMUNDUR");
  // delay(1000);
  setModeWarehouse();
}

/***********************************************************
 *  MODE HANDLERS                                         *
 ***********************************************************/
void pembacaanTerminal() {
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
    if (detectedStationId > 0) {
      // Use RFID station ID directly
      station = detectedStationId;
      // Serial.print("RFID detected - Station ID: ");
    // Serial.println(detectedStationId);

      errorValue = 0;
      sudahDeteksiStasiun = true;

      // Check if current station is in stationsList (from preferences)
      bool isTargetStation = false;
      for (size_t i = 0; i < stationsList.size(); i++) {
        if (stationsList[i] == station) {
          // Serial.println("Target station reached via RFID!");

          // Check if this is the last station
          if (indexTarget >= stationsList.size() - 1) {
            // Serial.println("Last station reached - calling ujungStation!");
            ujungStation();
          } else {
            inStation();
            indexTarget++;
          }

          isTargetStation = true;
          break;
        }
      }

      if (!isTargetStation) {
        // Serial.println("Non-target station detected via RFID - continue");
        pidLinefollower(errorValue, "MAJU");
      }
      return;
    }
  }

  // ― Ujung station: pindah ke warehouse (mundur) ―
  if (sensorkebacasemua && modeMaju && !force) {
    ujungStation();
    return;
  }
  // ― Keluar station dengan tombol (FORCEMAJU) ―
  if ((kananHilang || kiriHilang) && force) {
    return pidLinefollower(errorValue, "FORCEMAJU");
  }

  // ― Deteksi marker tengah (Fallback) ―
  // if (tengahAktif && !force && !sudahDeteksiStasiun) {
  //   bool kanan = (kananHilang && station % 2 == 0);
  //   bool kiri = (kiriHilang && station % 2 == 1);

  //   if ((kanan || kiri) && !modeBerhenti) {
  //     // Fallback: increment station if no RFID detected
  //     station++;
  //     Serial.print("Fallback sensor detection - Station: ");
  //     Serial.println(station);

  //     errorValue = 0;
  //     sudahDeteksiStasiun = true;

  //     // Check if current station is in stationsList (from preferences)
  //     bool isTargetStation = false;
  //     for (size_t i = 0; i < stationsList.size(); i++) {
  //       if (stationsList[i] == station) {
  //         Serial.println("Target station reached via sensor!");

  //         // Check if this is the last station
  //         if (indexTarget >= stationsList.size() - 1) {
  //           Serial.println("Last station reached - calling ujungStation!");
  //           ujungStation();
  //         } else {
  //           inStation();
  //           indexTarget++;
  //         }

  //         isTargetStation = true;
  //         break;
  //       }
  //     }

  //     if (!isTargetStation) {
  //       Serial.println("Non-target station passed via sensor");
  //     }

  //     pidLinefollower(errorValue, "MAJU");
  //     return;
  //   }
  // }

  // ― Reset bila keluar garis ―
  if (!sensorkebacasemua) {
    force = false;
    sudahDeteksiStasiun = false;
  }
  // Serial.println("Maju tes");
  pidLinefollower(errorValue, "MAJU");
}

/***********************************************************
 *  BUTTON HANDLER (PS3)                                  *
 ***********************************************************/
void tombolAgv() {
  unsigned long ms = millis();

  if (START() && (ms - lastXPress >= xDelay) && !modeStation) {
    lastXPress = ms;
    // if (buttonStep == 0) {
    //   outTerminal();
    //   buttonStep = 1;
    // } else {
    //   buttonStep = 0;
    // }
    outWarehouse();
  } else if (START() && (ms - lastXPress >= xDelay) && modeStation) {
    outStation();
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
  if (statusJalan != "BERHENTI") {
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
    statusJalan = "MAJU";
  } else if (modeMundur) {
    statusJalan = "MUNDUR";
  } else if (modeBerhenti) {
    statusJalan = "BERHENTI";
  } else {
    statusJalan = "BERHENTI";
    modeBerhenti = true;
  }

  // Tentukan mode aktif
  String currentMode = "UNKNOWN";
  if (modeTerminal) {
    currentMode = "TERMINAL";
  } else if (modeWarehouse) {
    currentMode = "WAREHOUSE";
  } else if (modeStation) {
    currentMode = "STATION";
  } else {
    // modeTerminal = true;
    modeWarehouse = true;
  }

  // Display on row 3 (last available row)
  lcd.setCursor(0, 3);
  lcd.print(currentMode.substring(0, 8));  // First 8 chars
  lcd.print(" ");
  lcd.print(statusJalan.substring(0, 7));  // Fit remaining space

  if (modeStation) {
    // Clear part of row 3 and show station info
    lcd.setCursor(0, 3);
    lcd.print("ST:");
    lcd.print(station);
    lcd.print(" TG:");
    if (indexTarget < stationsList.size()) {
      lcd.print(stationsList[indexTarget]);
    } else {
      lcd.print("END");
    }
  }
}
