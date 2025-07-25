
// Obstacle detection variables
bool obstacleDetected = false;
uint16_t minSafeDistance = 30; // cm - minimum safe distance
// ultrasonicDistances arrays moved to config.h as ultrasonicDistancesFront and ultrasonicDistancesBack
unsigned long lastObstacleCheck = 0;
const unsigned long obstacleCheckInterval = 100; // Check every 100ms

// Ultrasonic communication now handled by unified RS485 system
// This function is kept for compatibility but functionality moved to unified_rs485.ino
void loopUltrasonik() {
  // Ultrasonic data processing is now handled in loopUnifiedRS485()
  // This function can be called but does nothing as communication is centralized
}


// ------------------- FUNGSI-FUNGSI BANTUAN -------------------

/**
 * Memproses satu paket data yang telah lengkap diterima.
 */
// Packet parsing now handled by unified RS485 system
// This function is kept for compatibility but functionality moved to unified_rs485.ino
void parsePacket() {
  // Packet parsing is now handled in parseUltrasonicPacket() in unified_rs485.ino
  // This function can be called but does nothing as parsing is centralized
}

/**
 * Menghitung CRC-16 untuk validasi data Modbus.
 * Ini adalah fungsi standar dan tidak perlu diubah.
 */
uint16_t calculate_crc(byte* buffer, int len) {
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buffer[pos];
    for (int i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}



/**
 * Check for obstacles - now supports both front and back sensors
 */
void checkObstacles() {
  // Front obstacle detection
  checkObstaclesFront();
  
  // Back obstacle detection (when moving backward)
  if (modeMundur) {
    checkObstaclesBack();
  }
}

/**
 * Get ultrasonic distances for front sensors
 */
uint16_t* getUltrasonicDistancesFront() {
  return ultrasonicDistancesFront;
}

/**
 * Get ultrasonic distances for back sensors
 */
uint16_t* getUltrasonicDistancesBack() {
  return ultrasonicDistancesBack;
}

/**
 * Check if any ultrasonic sensor detects obstacle
 */
bool hasObstacle(bool checkFront = true) {
  uint16_t* distances = checkFront ? ultrasonicDistancesFront : ultrasonicDistancesBack;
  
  for (int i = 0; i < 5; i++) {
    if (distances[i] > 0 && distances[i] < minSafeDistance) {
      return true;
    }
  }
  return false;
}