
// ===================================================================
// SENSOR ULTRASONIK VARIABLES SUDAH DIPINDAHKAN KE config.h
// ===================================================================

// Konstanta yang tidak dipindahkan // Check every 100ms

// Function to set ultrasonic slave ID dynamically
void setUltrasonicSlaveId(int slaveId) {
  if (slaveId >= 1 && slaveId <= 247) {  // Valid Modbus RTU range
    if (currentUltrasonicSlaveId != slaveId) {
      currentUltrasonicSlaveId = slaveId;
      Serial.printf("Ultrasonic slave ID set to: %d\n", slaveId);
      
      // Force reinitialization when slave ID changes
      if (Serial1) {
        node.begin(currentUltrasonicSlaveId, Serial1);
      }
    }
  }
}

// Pre and post transmission functions for RS485
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
  // Ensure ultrasonic slave ID is initialized on first run
  // static bool firstRun = true;
  // if (firstRun) {
  //   node.begin( SLAVEID_ULTRASONIK_DEPAN, Serial1); // Mengatur ID slave dan port serial
  //   delay(50); // Allow initialization to complete
  //   firstRun = false;
  // }
  
  // Ultra-fast slave ID switching with zero-overhead
  // static int lastUltrasonicSlaveId = -1;
  // if (currentUltrasonicSlaveId != lastUltrasonicSlaveId) {
  //   node.begin(currentUltrasonicSlaveId, Serial1); // Mengatur ID slave dan port serial
  //   lastUltrasonicSlaveId = currentUltrasonicSlaveId;
  //   delay(10); // Small delay for stability
  // }
  node.begin(currentUltrasonicSlaveId, Serial1); // Mengatur ID slave dan port serial

  // Skip if Serial1 not ready
  if (!Serial1) {
    return;
  }

  // Membaca 5 register penahan (holding registers) dari alamat 0x0000
  // Register ini berisi data jarak dari Probe 1 hingga Probe 5
  uint8_t modbusResult = node.readHoldingRegisters(0x0000, 5);

  // Periksa hasil komunikasi Modbus
  if (modbusResult == node.ku8MBSuccess) {
    // Ekstrak data jarak dari buffer respons Modbus
    for (int i = 0; i < 5; i++) {
      ultrasonicDistances[i] = node.getResponseBuffer(i); // Simpan jarak ke array
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
      setupRS485(BAUDRATE);
      consecutiveFailures = 0;
    }
    
    // Set default values on communication failure
    for (int i = 0; i < 5; i++) {
      ultrasonicDistances[i] = 999; // Safe default value
    }
  }
}


// ------------------- FUNGSI-FUNGSI BANTUAN -------------------

/**
 * Initialize ultrasonic sensor to polling mode
 * Call this during setup for each ultrasonic sensor
 */
void initUltrasonicSensor(int slaveId) {
  // Set current slave ID
  // setUltrasonicSlaveId(slaveId);
  node.begin(slaveId, Serial1);
  
  currentUltrasonicSlaveId = slaveId;
  // Add small delay to ensure initialization
  delay(50);

  // Test basic communication first
  uint8_t testResult = node.readHoldingRegisters(0x0000, 1);
  if (testResult != node.ku8MBSuccess) {
    Serial.printf("[WARNING] Initial ultrasonic test failed for slave %d, error: 0x%02X\n", slaveId, testResult);
    // Don't fail setup, let it retry in loop
    return;
  }

  // Set sensor to polling mode (register 0x0007 = 0x0000)
  uint8_t setMode = node.writeSingleRegister(0x0007, 0x0000);
  if (setMode == node.ku8MBSuccess) {
    Serial.printf("Ultrasonic sensor (Slave ID: %d) set to polling mode successfully.\n", slaveId);
  } else {
    Serial.printf("Failed to set ultrasonic sensor (Slave ID: %d) mode, error code: 0x%02X\n", slaveId, setMode);
    // Don't log as error, let it continue working in default mode
  }
}

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

  // Check each probe for obstacles
  for (int i = 1; i < 4; i++) {
    if (ultrasonicDistances[i] > 0 && ultrasonicDistances[i] < minSafeDistance) {
      obstacleDetected = true;
      break;
    }
  }

  // If obstacle just detected, trigger buzzer
  static bool musicAlreadyPlaying = false;
  if (obstacleDetected && !previousObstacleState) {
    agvStop();
    if (!musicAlreadyPlaying) {
      music(MUSIC_MODE_ERROR);
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