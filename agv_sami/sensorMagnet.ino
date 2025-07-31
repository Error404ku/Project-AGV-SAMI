// Current magnet slave ID (default: front sensor)
// ===================================================================
// SENSOR MAGNET VARIABLES SUDAH DIPINDAHKAN KE config.h
// ===================================================================

void preTransmissionMagnet() {
  Serial.println("[DEBUG] preTransmissionMagnet: Setting MAX485 to transmit mode (RE=1, DE=1)");
  digitalWrite(MAX485_RE, 1);
  digitalWrite(MAX485_DE, 1);
  delayMicroseconds(10);  // Small delay to ensure pin state change
}

void postTransmissionMagnet() {
  Serial.println("[DEBUG] postTransmissionMagnet: Setting MAX485 to receive mode (RE=0, DE=0)");
  delayMicroseconds(10);  // Small delay before switching
  digitalWrite(MAX485_RE, 0);
  digitalWrite(MAX485_DE, 0);
}


void bacaSensorGaris() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    bacaSensor();
  }
}

// ==================== Fungsi Membaca Sensor ====================
void bacaSensor() {
  bacaSensor(currentMagnetSlaveId);
}

void bacaSensor(int slaveId) {
  // Verify Serial1 is initialized
  if (!Serial1) {
    Serial.println("[ERROR] Serial1 tidak terinisialisasi!");
    return;
  }

  node.begin(slaveId, Serial1);
  node.preTransmission(preTransmissionMagnet);
  node.postTransmission(postTransmissionMagnet);

  static int consecutiveFailuresFront = 0;
  static int consecutiveFailuresBack = 0;
  int& consecutiveFailures = (slaveId == SLAVEID_MAGNET_DEPAN) ? consecutiveFailuresFront : consecutiveFailuresBack;

  // Add timeout for modbus communication
  unsigned long startTime = millis();
  uint8_t result = node.readHoldingRegisters(0x0000, 2);
  unsigned long endTime = millis();

  if (result == node.ku8MBSuccess) {
    consecutiveFailures = 0;
    uint16_t medianValue = node.getResponseBuffer(0);
    uint16_t positionValue = node.getResponseBuffer(1);
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
    consecutiveFailures++;

    // Reset communication if too many failures
    if (consecutiveFailures >= 5) {
      setupRS485(BAUDRATE);
      consecutiveFailures = 0;
    }
  }
}

// ==================== Fungsi untuk mencetak semua segmen aktif dari Bitmask (Active Low) ====================
void printActiveSegmentsFromBitmask(uint16_t positionValue) {
  if (positionValue == 0xFFFF) {
    return;
  }

  bool foundAny = false;
  for (int i = 0; i < 16; i++) {
    if (!((positionValue >> i) & 0x01)) {
      foundAny = true;
    }
  }
  if (!foundAny) {
    Serial.print(F("Tidak ada segmen aktif (Error Logika)."));
  }
}

int hitungErrorPosisi(uint16_t bitmask) {
  int jumlahSegmenAktif = 0;
  int segmenTertinggi = 0;
  int segmenTerendah = 17;

  for (int i = 0; i < 16; i++) {
    if (!((bitmask >> i) & 0x01)) {
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

  if (segmenTerendah < 7) {
    switch (segmenTerendah) {
      case 6: errorKiri = -1; break;
      case 5: errorKiri = -2; break;
      case 4: errorKiri = -3; break;
      case 3: errorKiri = -4; break;
      case 2: errorKiri = -5; break;
      case 1: errorKiri = -6; break;
    }
  }

  if (segmenTertinggi > 10) {
    switch (segmenTertinggi) {
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
    return 0;  // tengah atau seimbang
}

void updateJumlahMagnet(uint16_t bitmask) {
  for (int i = 0; i < 16; i++) {
    jumlahMagnet[i] = !((bitmask >> i) & 0x01) ? 1 : 0;
  }
}

// ==================== Magnet Sensor Switching Functions ====================

/**
 * Switch between front and back magnet sensors
 */
void switchMagnetSensor(bool useFrontSensor) {
  Serial.printf("[DEBUG] switchMagnetSensor called with useFrontSensor: %s\n", useFrontSensor ? "true" : "false");

  if (useFrontSensor) {
    setMagnetSlaveId(SLAVEID_MAGNET_DEPAN);
    Serial.printf("[INFO] Switched to FRONT magnet sensor (ID: %d)\n", SLAVEID_MAGNET_DEPAN);
  } else {
    setMagnetSlaveId(SLAVEID_MAGNET_BELAKANG);
    Serial.printf("[INFO] Switched to BACK magnet sensor (ID: %d)\n", SLAVEID_MAGNET_BELAKANG);
  }

  Serial.printf("[DEBUG] Current magnet slave ID after switch: %d\n", getCurrentMagnetSlaveId());
}

void setMagnetSlaveId(int slaveId) {
  int previousId = currentMagnetSlaveId;
  currentMagnetSlaveId = slaveId;
  Serial.printf("[DEBUG] setMagnetSlaveId: %d -> %d\n", previousId, slaveId);

  // Verify the change
  if (currentMagnetSlaveId == slaveId) {
    Serial.printf("[SUCCESS] Magnet slave ID successfully set to: %d\n", slaveId);
  } else {
    Serial.printf("[ERROR] Failed to set magnet slave ID to: %d (current: %d)\n", slaveId, currentMagnetSlaveId);
  }
}
/**
 * Get current magnet sensor slave ID
 */
int getCurrentMagnetSlaveId() {
  return currentMagnetSlaveId;
}
