
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
  // Ultra-fast slave ID switching with zero-overhead
  static int lastUltrasonicSlaveId = -1;
  if (currentUltrasonicSlaveId != lastUltrasonicSlaveId) {
    node.begin(currentUltrasonicSlaveId, Serial1); // Mengatur ID slave dan port serial
    lastUltrasonicSlaveId = currentUltrasonicSlaveId;
  }

  // Membaca 5 register penahan (holding registers) dari alamat 0x0000
  // Register ini berisi data jarak dari Probe 1 hingga Probe 5
  uint8_t modbusResult = node.readHoldingRegisters(0x0000, 5);

  // Periksa hasil komunikasi Modbus
  if (modbusResult == node.ku8MBSuccess) {
    Serial.printf("=== Data Ultrasonik (Slave ID: %d) ===\n", currentUltrasonicSlaveId);

    // Ekstrak data jarak dari buffer respons Modbus
    for (int i = 0; i < 5; i++) {
      ultrasonicDistances[i] = node.getResponseBuffer(i); // Simpan jarak ke array
      Serial.printf("  Probe %d: %d cm\n", i + 1, ultrasonicDistances[i]); // Cetak jarak
    }

    // Setelah membaca semua data jarak, periksa apakah ada halangan
    // checkObstacles();

  } else {
    // Tangani kesalahan komunikasi Modbus
    Serial.printf("Error membaca sensor ultrasonik (Slave ID: %d), kode error: 0x%02X\n",currentUltrasonicSlaveId, modbusResult);
    // Catat kesalahan tetapi jangan hentikan sistem
    logError(ERROR_ULTRASONIC_COMMUNICATION, "Gagal membaca sensor ultrasonik slave " + String(currentUltrasonicSlaveId));
  }
}


// ------------------- FUNGSI-FUNGSI BANTUAN -------------------

/**
 * Initialize ultrasonic sensor to polling mode
 * Call this during setup for each ultrasonic sensor
 */
void initUltrasonicSensor(int slaveId) {
  // Set current slave ID
  setUltrasonicSlaveId(slaveId);

  // Configure node for this slave - set callbacks only once
  static bool callbacksSet = false;
  if (!callbacksSet) {
    node.preTransmission(preTransmissionUltrasonic);
    node.postTransmission(postTransmissionUltrasonic);
    callbacksSet = true;
  }
  
  node.begin(slaveId, Serial1);

  // Set sensor to polling mode (register 0x0007 = 0x0000)
  uint8_t setMode = node.writeSingleRegister(0x0007, 0x0000);
  if (setMode == node.ku8MBSuccess) {
    Serial.printf("Ultrasonic sensor (Slave ID: %d) set to polling mode successfully.\n", slaveId);
  } else {
    Serial.printf("Failed to set ultrasonic sensor (Slave ID: %d) mode, error code: 0x%02X\n", slaveId, setMode);
    logError(ERROR_ULTRASONIC_COMMUNICATION,
             "Gagal set mode sensor ultrasonik slave " + String(slaveId));
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
      Serial.printf("OBSTACLE DETECTED! Probe %d: %d cm (Slave ID: %d)\n",
                    i + 1, ultrasonicDistances[i], currentUltrasonicSlaveId);
      break;
    }
  }

  // If obstacle just detected, trigger buzzer
  if (obstacleDetected && !previousObstacleState) {
    agvStop();
    // buzzerError();
    music(MUSIC_MODE_ERROR);
  } else if (!obstacleDetected && previousObstacleState) {
    stopMusic();
  }
  // If obstacle cleared, notify
  if (!obstacleDetected && previousObstacleState) {
    Serial.println("Path clear - obstacle removed");
  }
}