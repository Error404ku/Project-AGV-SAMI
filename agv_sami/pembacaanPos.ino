
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
bool modeTerminal = true;  // mulai di terminal & diam
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
  force = true;
}

void inWarehouse() {
  if (modeMundur) {
    force = true;
    if (!modeStation) {
      setModeTerminal();  // balik arah, pulang ke terminal
    }
  } else {
    clearMovement();
    modeBerhenti = true;
  }
}
void outWarehouse() {
  setModeStation();
  modeMaju = true;
  force = true;
}

void inStation() {
  clearMovement();
  modeBerhenti = true;
}
void outStation() {
  modeMaju = true;
  force = true;
}
void ujungStation() {  // ujung station → mundur ke warehouse
  modeMaju = false;
  modeMundur = true;
  force = true;
  pidLinefollower(errorValue, "FORCEMUNDUR");
  delay(1000);
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
  // ― Deteksi marker tengah ―
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
  // ― Ujung station: pindah ke warehouse (mundur) ―
  if (sensorkebacasemua && modeMaju && !force) {
    ujungStation();
    return;
  }
  // ― Keluar station dengan tombol (FORCEMAJU) ―
  if ((kananHilang || kiriHilang) && force) {
    return pidLinefollower(errorValue, "FORCEMAJU");
  }

  // ― Deteksi marker tengah ―
  if (tengahAktif && !force && !sudahDeteksiStasiun) {
    bool kanan = (kananHilang && station % 2 == 0);
    bool kiri = (kiriHilang && station % 2 == 1);

    if ((kanan || kiri) && !modeBerhenti) {
      station++;
      errorValue = 0;
      sudahDeteksiStasiun = true;

      if (station == targetStation[indexTarget]) {
        inStation();
        if (indexTarget < totalTarget - 1)
          indexTarget++;
      }
      pidLinefollower(errorValue, "MAJU");
      return;
    }
  }

  // ― Reset bila keluar garis ―
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

  if (X() && (ms - lastXPress >= xDelay) && !modeStation) {
    lastXPress = ms;
    if (buttonStep == 0) {
      outTerminal();
      buttonStep = 1;
    } else {
      outWarehouse();
      buttonStep = 0;
    }
  } else if (Y()) {
    outStation();
  }
}

/***********************************************************
 *  MAIN LOGIC – PANGGIL DI loop()                        *
 ***********************************************************/
void logicAgv() {
  // ― Update processed sensor flags ―
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
  // ― Prioritas gerakan global ―
  if (statusJalan != "BERHENTI") {
    modeBerhenti = false;
  }
  if (modeBerhenti) {
    pidLinefollower(errorValue, "STOP");
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
    modeTerminal = true;
  }
  display.print("Mode : ");
  display.println(currentMode);
  display.print("Status : ");
  display.println(statusJalan);

  if (modeStation) {
    display.print("Station : ");
    display.println(station);
    display.print("Target : ");
    display.println(targetStation[indexTarget]);
  }
}
