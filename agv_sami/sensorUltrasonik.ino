
// ===================================================================
// SENSOR ULTRASONIK VARIABLES SUDAH DIPINDAHKAN KE config.h
// ===================================================================

// Konstanta yang tidak dipindahkan // Check every 100ms

// Function to set ultrasonic slave ID dynamically - INDUSTRY STANDARD
// No need to call begin() here - it will be called automatically in loop when ID changes
void setUltrasonicSlaveId(int slaveId) {
  if (slaveId >= 1 && slaveId <= 247) {  // Valid Modbus RTU range
    if (currentUltrasonicSlaveId != slaveId) {
      currentUltrasonicSlaveId = slaveId;
      Serial.printf("Ultrasonic slave ID set to: %d\n", slaveId);
      // begin() will be called automatically in loopUltrasonik() when ID changes
    }
  }
}

// Pre and post transmission functions for RS485 Serial2
void preTransmissionUltrasonic() {
  digitalWrite(MAX485_RE, HIGH);
  digitalWrite(MAX485_DE, HIGH);
}

void postTransmissionUltrasonic() {
  digitalWrite(MAX485_RE, LOW);
  digitalWrite(MAX485_DE, LOW);
}

void loopUltrasonik() {
  // Rate limiting check
  static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastReadTime < 100) {  // 100ms minimum interval
    return;
  }
  lastReadTime = currentTime;
  
  // Ultra-fast slave ID switching with zero-overhead - INDUSTRY STANDARD
  // Only call begin() when slave ID changes - most efficient approach
  static int lastUltrasonicSlaveId = -1;
  if (currentUltrasonicSlaveId != lastUltrasonicSlaveId) {
    ultrasonicNode.begin(currentUltrasonicSlaveId, Serial2); // Mengatur ID slave dan port serial
    lastUltrasonicSlaveId = currentUltrasonicSlaveId;
    delay(10); // Small delay for stability
  }

  // Skip if Serial2 not ready
  if (!Serial2) {
    return;
  }

  // Membaca 5 register penahan (holding registers) dari alamat 0x0000
  // Register ini berisi data jarak dari Probe 1 hingga Probe 5
  uint8_t modbusResult = ultrasonicNode.readHoldingRegisters(0x0000, 5);

  // Periksa hasil komunikasi Modbus
  if (modbusResult == ultrasonicNode.ku8MBSuccess) {
    // Ekstrak data jarak dari buffer respons Modbus
    for (int i = 0; i < 5; i++) {
      ultrasonicDistances[i] = ultrasonicNode.getResponseBuffer(i); // Simpan jarak ke array
    }

    // Reset error counter on successful read
    static int consecutiveFailures = 0;
    consecutiveFailures = 0;

  } else {
    // Tangani kesalahan komunikasi Modbus dengan retry mechanism
    static int consecutiveFailures = 0;
    
    consecutiveFailures++;
    
    // Reset communication on too many failures
    if (consecutiveFailures >= 5) {
      setupRS485_Serial2(BAUDRATE);
      consecutiveFailures = 0;
    }
    
    // Set default values on communication failure
    for (int i = 0; i < 5; i++) {
      ultrasonicDistances[i] = 999; // Safe default value
    }
  }
}


// ------------------- FUNGSI-FUNGSI BANTUAN -------------------

// Fungsi initUltrasonicSensor dihapus karena tidak digunakan
// Setup sensor ultrasonik menggunakan setupSensorUltrasonic() di setup.ino

/**
 * Get current ultrasonic sensor slave ID
 */
int getCurrentUltrasonicSlaveId() {
  return currentUltrasonicSlaveId;
}

/**
 * Check for obstacles in front of AGV
 */
void checkObstacles() {
  bool previousObstacleState = obstacleDetected;
  obstacleDetected = false;
    // Determine which safe distance to use based on current sensor
  uint16_t currentMinSafeDistance;
  if (currentUltrasonicSlaveId == SLAVEID_ULTRASONIK_DEPAN) {
    currentMinSafeDistance = minSafeDistanceFront;
  } else if (currentUltrasonicSlaveId == SLAVEID_ULTRASONIK_BELAKANG) {
    currentMinSafeDistance = minSafeDistanceBack;
  } else {
    currentMinSafeDistance = minSafeDistanceFront; // Default to front
  }

  if (ultrasonicDistances[2] > 0 && ultrasonicDistances[2] < currentMinSafeDistance) {
    obstacleDetected = true;
  }

  // Check each probe for obstacles
  // for (int i = 1; i < 4; i++) {
  //   if (ultrasonicDistances[i] > 0 && ultrasonicDistances[i] < currentMinSafeDistance) {
  //     obstacleDetected = true;
  //     break;
  //   }
  // }

  // If obstacle just detected, trigger buzzer
  static bool musicAlreadyPlaying = false;
  if (obstacleDetected && !previousObstacleState) {
    agvStop();
    if (!musicAlreadyPlaying) {
      music(MUSIC_MODE_OBSTACLE);
      softStartTime = millis();
      softStartActive = true;
      pidSpeed = baseSpeed / 4;
      musicAlreadyPlaying = true;
    }
  } else if (!obstacleDetected && previousObstacleState) {
    if (musicAlreadyPlaying) {
      stopMusic();
      musicAlreadyPlaying = false;
    }
  } else if (!obstacleDetected) {
    musicAlreadyPlaying = false;
  }
}