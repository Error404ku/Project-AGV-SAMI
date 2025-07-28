extern bool modeMaju;
extern bool modeMundur;

// Current magnet slave ID (default: front sensor)
int currentMagnetSlaveId = SLAVEID_MAGNET_DEPAN;

void changeStateMode(String mode){
    if (mode == "maju" && !modeMundur){
        setupSensorMagnet(SLAVEID_MAGNET_DEPAN, BAUDRATE);
        setupUltrasonikWithParams(SLAVEID_ULTRASONIK_DEPAN, BAUDRATE);
        modeMaju = true;
        modeMundur = false;
    }else if (mode == "mundur" && !modeMaju){
        setupSensorMagnet(SLAVEID_MAGNET_BELAKANG, BAUDRATE);
        setupUltrasonikWithParams(SLAVEID_ULTRASONIK_BELAKANG, BAUDRATE);
        modeMaju = false;
        modeMundur = true;
    }
}

void preTransmission()
{
    digitalWrite(MAX485_RE, 1);
    digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
    digitalWrite(MAX485_RE, 0);
    digitalWrite(MAX485_DE, 0);
}


void bacaSensorGaris()
{
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        bacaSensor();
    }
}

// ==================== Fungsi Membaca Sensor ====================
void bacaSensor(int slaveId) {
    node.begin(slaveId, Serial1);
    Serial.println(F("Mengirim permintaan pembacaan..."));
    static int consecutiveFailures = 0;
    uint8_t result = node.readHoldingRegisters(0x0000, 2);
    Serial.print("Modbus result: ");
    Serial.println(result);
    if (result == node.ku8MBSuccess) {
        consecutiveFailures = 0;
        uint16_t medianValue = node.getResponseBuffer(0);
        uint16_t positionValue = node.getResponseBuffer(1);
        Serial.print("positionValue: 0x");
        Serial.println(positionValue, HEX);
        printActiveSegmentsFromBitmask(positionValue);
        updateJumlahMagnet(positionValue);
        if (positionValue == 0xFFFF) {
            totalSensorAktif = 0;
        } else {
            totalSensorAktif = 0;
            for (int i = 0; i < 16; i++) {
                if (jumlahMagnet[i]) totalSensorAktif++;
            }
            errorValue = hitungErrorPosisi(positionValue);
        }
    } else {
        Serial.print(F("Gagal membaca data sensor. Error Code: 0x"));
        Serial.println(result, HEX);
        consecutiveFailures++;
        Serial.println(consecutiveFailures);
    }
}
// Overload agar tetap kompatibel
void bacaSensor() {
    bacaSensor(currentMagnetSlaveId);
}

// ==================== Fungsi untuk mencetak semua segmen aktif dari Bitmask (Active Low) ====================
void printActiveSegmentsFromBitmask(uint16_t positionValue)
{
    if (positionValue == 0xFFFF)
    {
        return;
    }

    bool foundAny = false;
    for (int i = 0; i < 16; i++)
    {
        if (!((positionValue >> i) & 0x01))
        {
            Serial.print(i + 1);
            Serial.print(" ");
            foundAny = true;
        }
    }
    if (!foundAny)
    {
        Serial.print(F("Tidak ada segmen aktif (Error Logika)."));
    }
    Serial.println();
}
int hitungErrorPosisi(uint16_t bitmask)
{
    int jumlahSegmenAktif = 0;
    int segmenTertinggi = 0;
    int segmenTerendah = 17;

    for (int i = 0; i < 16; i++)
    {
        if (!((bitmask >> i) & 0x01))
        {
            jumlahSegmenAktif++;
            int segmenSaatIni = i + 1;
            if (segmenSaatIni < segmenTerendah)
                segmenTerendah = segmenSaatIni;
            if (segmenSaatIni > segmenTertinggi)
                segmenTertinggi = segmenSaatIni;
        }
    }

    if (jumlahSegmenAktif == 0)
        return 99;

    // Logika baru: cek dua-duanya lalu ambil dominasi
    int errorKiri = 0, errorKanan = 0;

    if (segmenTerendah < 7)
    {
        switch (segmenTerendah)
        {
        case 6: errorKiri = -1; break;
        case 5: errorKiri = -2; break;
        case 4: errorKiri = -3; break;
        case 3: errorKiri = -4; break;
        case 2: errorKiri = -5; break;
        case 1: errorKiri = -6; break;
        }
    }

    if (segmenTertinggi > 10)
    {
        switch (segmenTertinggi)
        {
        case 11: errorKanan = 1; break;
        case 12: errorKanan = 2; break;
        case 13: errorKanan = 3; break;
        case 14: errorKanan = 4; break;
        case 15: errorKanan = 5; break;
        case 16: errorKanan = 6; break;
        }
    }

    // Bandingkan dominasi sisi kiri vs kanan
    if (abs(errorKiri) > abs(errorKanan))
        return errorKiri;
    else if (abs(errorKanan) > abs(errorKiri))
        return errorKanan;
    else
        return 0; // tengah atau seimbang
}

void updateJumlahMagnet(uint16_t bitmask)
{
    for (int i = 0; i < 16; i++)
    {
        jumlahMagnet[i] = !((bitmask >> i) & 0x01) ? 1 : 0;
    }
}

// ==================== Magnet Sensor Switching Functions ====================

/**
 * Switch between front and back magnet sensors
 */
void switchMagnetSensor(bool useFrontSensor) {
  if (useFrontSensor) {
    currentMagnetSlaveId = SLAVEID_MAGNET_DEPAN;
    Serial.println("Switched to FRONT magnet sensor");
  } else {
    currentMagnetSlaveId = SLAVEID_MAGNET_BELAKANG;
    Serial.println("Switched to BACK magnet sensor");
  }
}

/**
 * Get current magnet sensor slave ID
 */
int getCurrentMagnetSlaveId() {
  return currentMagnetSlaveId;
}

