
// Obstacle detection variables
bool obstacleDetected = false;
uint16_t minSafeDistance = 30;            // cm - minimum safe distance
uint16_t ultrasonicDistances[5] = { 0 };  // Store distances from 5 probes
unsigned long lastObstacleCheck = 0;
const unsigned long obstacleCheckInterval = 100;  // Check every 100ms

// Current ultrasonic slave ID (default: front sensor)
int currentUltrasonicSlaveId = SLAVEID_ULTRASONIK_DEPAN;

// Function to set ultrasonic slave ID dynamically
void setUltrasonicSlaveId(int slaveId) {
  currentUltrasonicSlaveId = slaveId;
  Serial.printf("Ultrasonic slave ID set to: %d\n", slaveId);
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
  static unsigned long lastReadTime = 0;
  const unsigned long readInterval = 100;  // Read every 100ms

  unsigned long currentTime = millis();
  if (currentTime - lastReadTime >= readInterval) {
    lastReadTime = currentTime;

    // Set current slave ID for node communication
    node.begin(currentUltrasonicSlaveId, Serial1);
    node.preTransmission(preTransmissionUltrasonic);
    node.postTransmission(postTransmissionUltrasonic);

    // Read 5 holding registers from address 0x0000 (Probe 1-5)
    uint8_t result = node.readHoldingRegisters(0x0000, 5);

    if (result == node.ku8MBSuccess) {
      Serial.printf("=== Ultrasonic Data (Slave ID: %d) ===\n", currentUltrasonicSlaveId);

      // Extract distance data from response buffer
      for (int i = 0; i < 5; i++) {
        ultrasonicDistances[i] = node.getResponseBuffer(i);
        Serial.printf("  Probe %d: %d cm\n", i + 1, ultrasonicDistances[i]);
      }

      // Check for obstacles
      checkObstacles();
      Serial.println("========================\n");

    } else {
      // Handle communication error
      Serial.printf("Error reading ultrasonic sensor (Slave ID: %d), error code: 0x%02X\n",
                    currentUltrasonicSlaveId, result);

      // Log error but don't stop system
      logError(ERROR_ULTRASONIC_COMMUNICATION,
               "Gagal baca sensor ultrasonik slave " + String(currentUltrasonicSlaveId));
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
  setUltrasonicSlaveId(slaveId);

  // Configure node for this slave
  node.begin(slaveId, Serial1);
  node.preTransmission(preTransmissionUltrasonic);
  node.postTransmission(postTransmissionUltrasonic);

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
 * Switch between front and back ultrasonic sensors
 */
void switchUltrasonicSensor(bool useFrontSensor) {
  if (useFrontSensor) {
    setUltrasonicSlaveId(SLAVEID_ULTRASONIK_DEPAN);
    Serial.println("Switched to FRONT ultrasonic sensor");
  } else {
    setUltrasonicSlaveId(SLAVEID_ULTRASONIK_BELAKANG);
    Serial.println("Switched to BACK ultrasonic sensor");
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
    Serial.println("EMERGENCY STOP - Obstacle detected!");
    // buzzerError();
    music("error");
  } else if (!obstacleDetected && previousObstacleState) {
    stopMusic();
  }
  // If obstacle cleared, notify
  if (!obstacleDetected && previousObstacleState) {
    Serial.println("Path clear - obstacle removed");
  }
}