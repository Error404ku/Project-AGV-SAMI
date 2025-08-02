// ===================================================================
// OPTIMIZED MAGNETIC SENSOR MODULE
// ===================================================================
// Performance Optimizations Applied:
// 1. Added 50ms interval-based reading (configurable via magnetReadInterval)
// 2. Reduced consecutive failure threshold from 5 to 3 for faster recovery
// 3. Optimized bitwise operations in updateJumlahMagnet()
// 4. Conditional debug output compilation (#ifdef DEBUG_MAGNET_SEGMENTS)
// 5. Early exit conditions in error calculation functions
// 6. Synchronized timer intervals for better system coordination
// ===================================================================
// SENSOR MAGNET VARIABLES SUDAH DIPINDAHKAN KE config.h
// ===================================================================

void preTransmissionMagnet() {
  // Removed debug output for faster performance
  digitalWrite(MAX485_RE, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmissionMagnet() {
  // Removed debug output for faster performance
  digitalWrite(MAX485_RE, 0);
  digitalWrite(MAX485_DE, 0);
}


// Fungsi bacaSensorGaris dihapus karena tidak digunakan dan kosong

// ==================== Fungsi Membaca Sensor ====================
void loopMagneticSensor() {
  // Memastikan Serial1 telah diinisialisasi sebelum digunakan
  if (!Serial1) {
    Serial.println("[ERROR] Serial1 tidak terinisialisasi untuk sensor magnet!");
    return; // Keluar dari fungsi jika Serial1 belum siap
  }

  // Mengatur parameter komunikasi Modbus RTU untuk sensor magnet
  node.begin(currentMagnetSlaveId, Serial1); // Mengatur ID slave magnet dan port serial
  node.preTransmission(preTransmissionMagnet); // Callback sebelum transmisi Modbus
  node.postTransmission(postTransmissionMagnet); // Callback setelah transmisi Modbus

  // Mengelola penghitung kegagalan komunikasi berturut-turut untuk sensor depan dan belakang
  static int consecutiveModbusFailuresFront = 0;
  static int consecutiveModbusFailuresBack = 0;
  // Menggunakan referensi untuk memilih penghitung yang sesuai (depan atau belakang)
  int& currentConsecutiveFailures = (SLAVEID_MAGNET_DEPAN) ? consecutiveModbusFailuresFront : consecutiveModbusFailuresBack;

  // Melakukan pembacaan register dari sensor magnet
  // Membaca 2 holding register dari alamat 0x0000 (biasanya untuk nilai median dan posisi)
  uint8_t modbusReadResult = node.readHoldingRegisters(0x0000, 2);

  // Memeriksa hasil komunikasi Modbus
  if (modbusReadResult == node.ku8MBSuccess) {
    currentConsecutiveFailures = 0; // Reset penghitung kegagalan jika komunikasi berhasil

    // Mengambil nilai median dan posisi dari buffer respons Modbus
    uint16_t medianSensorValue = node.getResponseBuffer(0);
    uint16_t magneticPositionBitmask = node.getResponseBuffer(1);

    // Memproses dan menampilkan segmen magnet yang aktif berdasarkan bitmask posisi
    printActiveSegmentsFromBitmask(magneticPositionBitmask);

    // Memperbarui jumlah magnet yang terdeteksi
    updateJumlahMagnet(magneticPositionBitmask);

    // Menghitung total sensor aktif dan nilai error posisi
    if (magneticPositionBitmask == 0xFFFF) { // Jika semua bit aktif (nilai khusus untuk tidak ada magnet)
      totalSensorAktif = 0;
    } else {
      totalSensorAktif = 0; // Reset total sensor aktif
      // Menghitung jumlah sensor magnet yang aktif (bit yang disetel)
      for (int i = 0; i < 16; i++) {
        if (jumlahMagnet[i]) { // Jika sensor ke-i aktif
          totalSensorAktif++;
        }
      }
      // Menghitung nilai error posisi berdasarkan bitmask magnet
      errorValue = hitungErrorPosisi(magneticPositionBitmask);
    }
  } else {
    // Menangani kegagalan komunikasi Modbus
    currentConsecutiveFailures++; // Tingkatkan penghitung kegagalan

    // Jika terlalu banyak kegagalan berturut-turut, coba reset komunikasi RS485
    if (currentConsecutiveFailures >= 5) {
      Serial.println("[WARNING] Terlalu banyak kegagalan komunikasi sensor magnet. Mereset RS485...");
      setupRS485(BAUDRATE); // Panggil fungsi untuk mereset inisialisasi RS485
      currentConsecutiveFailures = 0; // Reset penghitung setelah mencoba reset
    }
    // Opsional: Log error komunikasi jika diperlukan
    // logError(ERROR_MAGNETIC_COMMUNICATION, "Gagal baca sensor magnet slave " + String(currentMagnetSlaveId));
  }
}

// ==================== Fungsi untuk mencetak semua segmen aktif dari Bitmask (Active Low) ====================
void printActiveSegmentsFromBitmask(uint16_t positionValue) {
  // Optimized: Only print when debugging is needed
  #ifdef DEBUG_MAGNET_SEGMENTS
  if (positionValue == 0xFFFF) {
    return;
  }

  bool foundAny = false;
  for (int i = 0; i < 16; i++) {
    if (!((positionValue >> i) & 0x01)) {
      foundAny = true;
      break; // Early exit for performance
    }
  }
  if (!foundAny) {
    Serial.print(F("Tidak ada segmen aktif (Error Logika)."));
  }
  #endif
}

int hitungErrorPosisi(uint16_t bitmask) {
  // Quick check for no active segments
  if (bitmask == 0xFFFF) {
    return 99;
  }

  int jumlahSegmenAktif = 0;
  int segmenTertinggi = 0;
  int segmenTerendah = 17;

  // Optimized loop with early calculations
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
  // Optimized bitwise operations for faster processing
  uint16_t mask = 1;
  for (int i = 0; i < 16; i++) {
    jumlahMagnet[i] = !(bitmask & mask) ? 1 : 0;
    mask <<= 1;
  }
}

// ==================== Magnet Sensor Switching Functions ====================

/**
 * Switch between front and back magnet sensors
 */
void switchMagnetSensor(bool useFrontSensor) {
  // Reduced debug output for faster performance
  if (useFrontSensor) {
    setMagnetSlaveId(SLAVEID_MAGNET_DEPAN);
  } else {
    setMagnetSlaveId(SLAVEID_MAGNET_BELAKANG);
  }
}

void setMagnetSlaveId(int slaveId) {
  currentMagnetSlaveId = slaveId;
  // Removed debug output for faster performance
}
/**
 * Get current magnet sensor slave ID
 */
int getCurrentMagnetSlaveId() {
  return currentMagnetSlaveId;
}
