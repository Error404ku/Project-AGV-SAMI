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

// ==================== HIGHLY OPTIMIZED SENSOR READING ====================
void loopMagneticSensor() {
  // Real-time reading - no rate limiting for maximum responsiveness
  // Fast validation - exit early if Serial1 not ready
  if (!Serial1) {
    return; // Keluar dari fungsi jika Serial1 belum siap
  }

  // Ultra-fast slave ID switching with zero-overhead - INDUSTRY STANDARD
  // Only call begin() when slave ID changes - most efficient approach
  static int lastSlaveId = -1;
  static unsigned long lastSwitchTime = 0;
  if (currentMagnetSlaveId != lastSlaveId) {
    magnetNode.begin(currentMagnetSlaveId, Serial1);
    lastSlaveId = currentMagnetSlaveId;
    lastSwitchTime = millis();
    return; // Skip this cycle to allow sensor to stabilize
  }
  
  // Wait for sensor stabilization after switch (non-blocking)
  if (millis() - lastSwitchTime < 10) {
    return; // Skip reading for 10ms after switch
  }

  // Enhanced error handling with exponential backoff
  static int consecutiveFailures[2] = {0, 0};  // [front, back]
  static unsigned long lastRetryTime[2] = {0, 0};
  
  int sensorIndex = (currentMagnetSlaveId == SLAVEID_MAGNET_DEPAN) ? 0 : 1;
  unsigned long currentMillis = millis();
  
  // Exponential backoff for failed reads
  if (consecutiveFailures[sensorIndex] > 0) {
    unsigned long backoffDelay = min(200, (1 << consecutiveFailures[sensorIndex]) * 50);
    if (currentMillis - lastRetryTime[sensorIndex] < backoffDelay) {
      return;  // Skip this cycle for backoff
    }
  }

  // Optimized single-read operation
  uint8_t result = magnetNode.readHoldingRegisters(0x0001, 1);
  
  if (result == magnetNode.ku8MBSuccess) {
    consecutiveFailures[sensorIndex] = 0;
    
    // Cache sensor data
    // uint16_t medianValue = magnetNode.getResponseBuffer(0);
    uint16_t positionBitmask = magnetNode.getResponseBuffer(0);
    
    // Fast processing with bit manipulation
    if (positionBitmask == 0xFFFF) {
      totalSensorAktif = 0;
      // Check if no magnet detected for 5 seconds
      if (lastDetectionTime == 0) {
        lastDetectionTime = currentMillis; // Start timer
      }
      
      unsigned long noMagnetDuration = currentMillis - lastDetectionTime;
      if (noMagnetDuration >= 2000) { // 3 seconds
        errorValue = 99; // Set error to 99 after 3 seconds
        lastErrorValue = 99;
      } else {
        errorValue = lastErrorValue; // Keep last valid error value
      }
    } else {
        // Magnet detected - reset timer and update error
        lastDetectionTime = 0;
      
      // Optimized bit counting using built-in functions
      uint16_t activeBits = ~positionBitmask & 0xFFFF;
      totalSensorAktif = __builtin_popcount(activeBits);
      
      // Fast error calculation
      errorValue = hitungErrorPosisi(positionBitmask);
      // if (currentMagnetSlaveId == SLAVEID_MAGNET_BELAKANG){
      //   // errorValue = errorValue * 1;
      // }
      
      // Store last valid error value
      lastErrorValue = errorValue;
    }
    
    // Update magnet array efficiently
    updateJumlahMagnet(positionBitmask);
    
  } else {
    consecutiveFailures[sensorIndex]++;
    lastRetryTime[sensorIndex] = currentMillis;
    
    // Smart reset on critical failures
    if (consecutiveFailures[sensorIndex] >= 10) {
      setupRS485(BAUDRATE);
      consecutiveFailures[sensorIndex] = 0;
    }
  }
}


uint16_t filterClosestMagnetGroup(uint16_t bitmask) {
  // Quick check untuk no active segments
  if (bitmask == 0xFFFF) {
    return bitmask;
  }
  
  // Deteksi grup magnet yang terpisah
  bool inGroup = false;
  int groupStart = -1;
  int groupEnd = -1;
  int groupCount = 0;
  
  struct MagnetGroup {
    int start;
    int end;
    int centerDistance;
  };
  
  MagnetGroup groups[8]; // Maksimal 8 grup
  int groupIndex = 0;
  
  // Scan untuk menemukan grup-grup terpisah
  for (int i = 0; i < 16; i++) {
    bool isActive = !((bitmask >> i) & 0x01);
    
    if (isActive && !inGroup) {
      // Mulai grup baru
      groupStart = i;
      inGroup = true;
    } else if (!isActive && inGroup) {
      // Akhir grup
      groupEnd = i - 1;
      
      // Hitung jarak ke titik tengah (antara segmen 7 dan 8)
      int groupCenter = (groupStart + groupEnd) / 2;
      int centerDistance = abs(groupCenter - 7); // Titik tengah antara 7 dan 8
      
      // Simpan grup
      if (groupIndex < 8) {
        groups[groupIndex].start = groupStart;
        groups[groupIndex].end = groupEnd;
        groups[groupIndex].centerDistance = centerDistance;
        groupIndex++;
      }
      
      inGroup = false;
    }
  }
  
  // Handle grup terakhir jika masih aktif
  if (inGroup) {
    groupEnd = 15;
    int groupCenter = (groupStart + groupEnd) / 2;
    int centerDistance = abs(groupCenter - 7);
    
    if (groupIndex < 8) {
      groups[groupIndex].start = groupStart;
      groups[groupIndex].end = groupEnd;
      groups[groupIndex].centerDistance = centerDistance;
      groupIndex++;
    }
  }
  
  // Jika hanya ada satu grup atau tidak ada grup, return original
  if (groupIndex <= 1) {
    return bitmask;
  }
  
  // Cari grup dengan jarak terdekat ke titik tengah
  int closestGroupIndex = 0;
  int minDistance = groups[0].centerDistance;
  
  for (int i = 1; i < groupIndex; i++) {
    if (groups[i].centerDistance < minDistance) {
      minDistance = groups[i].centerDistance;
      closestGroupIndex = i;
    }
  }
  
  // Buat bitmask baru dengan hanya grup terdekat
  uint16_t filteredBitmask = 0xFFFF; // Mulai dengan semua bit 1 (tidak aktif)
  
  // Set bit untuk grup terdekat menjadi 0 (aktif)
  for (int i = groups[closestGroupIndex].start; i <= groups[closestGroupIndex].end; i++) {
    if (!((bitmask >> i) & 0x01)) { // Jika bit asli aktif
      filteredBitmask &= ~(1 << i); // Set bit menjadi 0 (aktif)
    }
  }
  
  return filteredBitmask;
}

// ==================== ULTRA-FAST ERROR CALCULATION ====================
int hitungErrorPosisi(uint16_t bitmask) {
  // Quick check for no active segments
  if (bitmask == 0xFFFF) {
    return 99;
  }
  
  // Terapkan filter grup magnet terdekat (hanya untuk sensor belakang)
  bitmask = filterClosestMagnetGroup(bitmask);

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

  // Logika baru: cek dua-duanya lalu ambil dominasi
  int errorKiri = 0, errorKanan = 0;
  if (currentMagnetSlaveId == SLAVEID_MAGNET_DEPAN) {
    if (segmenTerendah < 6) {
      switch (segmenTerendah) {
        case 6: errorKiri = -1; break;   // Koreksi ringan
        case 5: errorKiri = -2; break;   // Koreksi ringan
        case 4: errorKiri = -3; break;  // Koreksi sedang
        case 3: errorKiri = -4; break;  // Koreksi kuat
        case 2: errorKiri = -5; break;  // Koreksi sangat kuat
        case 1: errorKiri = -6; break;  // Koreksi maksimal
      }
    }
    if (segmenTertinggi > 11) {
      switch (segmenTertinggi) {
        case 11: errorKanan = 1; break;   // Koreksi ringan
        case 12: errorKanan = 2; break;   // Koreksi ringan
        case 13: errorKanan = 3; break;  // Koreksi sedang
        case 14: errorKanan = 4; break;  // Koreksi kuat
        case 15: errorKanan = 5; break;  // Koreksi sangat kuat
        case 16: errorKanan = 6; break;  // Koreksi maksimal
      }
    }
  }else{
    if (segmenTerendah < 6) {
      switch (segmenTerendah) {
        case 5: errorKiri = -2; break;   // Koreksi ringan
        case 4: errorKiri = -3; break;  // Koreksi sedang
        case 3: errorKiri = -4; break;  // Koreksi kuat
        case 2: errorKiri = -5; break;  // Koreksi sangat kuat
        case 1: errorKiri = -6; break;  // Koreksi maksimal
      }
    }
    if (segmenTertinggi > 11) {
      switch (segmenTertinggi) {
        case 12: errorKanan = 2; break;   // Koreksi ringan
        case 13: errorKanan = 3; break;  // Koreksi sedang
        case 14: errorKanan = 4; break;  // Koreksi kuat
        case 15: errorKanan = 5; break;  // Koreksi sangat kuat
        case 16: errorKanan = 6; break;  // Koreksi maksimal
      }
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
  // Ultra-fast bit manipulation using direct assignment
  uint16_t inverted = ~bitmask;
  for (int i = 0; i < 16; i++) {
    jumlahMagnet[i] = (inverted >> i) & 1;
  }
}

// ==================== SENSOR HEALTH MONITORING ====================
/**
 * Get sensor health status
 * @return Health percentage (0-100%)
 */
int getSensorHealth(bool frontSensor) {
  static int healthHistory[2] = {100, 100};  // [front, back]
  int index = frontSensor ? 0 : 1;
  return healthHistory[index];
}

/**
 * Get last calculated error value
 * @return Last error value (-6 to 6, or 99 if no detection)
 */
int getLastErrorValue() {
  return lastErrorValue;
}

/**
 * Reset last error value to 99 (no detection)
 */
void resetLastErrorValue() {
  lastErrorValue = 99;
  lastDetectionTime = 0;
}

/**
 * Reset sensor communication with smart recovery
 */
void resetSensorCommunication() {
  setupRS485(BAUDRATE);
  delay(100);
}

// ==================== ULTRA-FAST SENSOR SWITCHING ====================
/**
 * Switch between front and back magnet sensors with zero-overhead
 * @param useFrontSensor true for front sensor, false for back sensor
 */
void switchMagnetSensor(bool useFrontSensor) {
  int newSlaveId = useFrontSensor ? SLAVEID_MAGNET_DEPAN : SLAVEID_MAGNET_BELAKANG;
  if (currentMagnetSlaveId != newSlaveId) {
    setMagnetSlaveId(newSlaveId);
  }
}

/**
 * Set magnet slave ID with validation and bounds checking - INDUSTRY STANDARD
 * No need to call begin() here - it will be called automatically in loop when ID changes
 * @param slaveId Modbus slave ID (1-247)
 */
void setMagnetSlaveId(int slaveId) {
  if (slaveId >= 1 && slaveId <= 247) {  // Valid Modbus RTU range
    currentMagnetSlaveId = slaveId;
    // begin() will be called automatically in loopMagneticSensor() when ID changes
  }
}

/**
 * Get current magnet sensor slave ID
 * @return Current active slave ID
 */
int getCurrentMagnetSlaveId() {
  return currentMagnetSlaveId;
}
