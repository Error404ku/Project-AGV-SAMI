/*
  SENSORS.INO - Unified Sensor Management
  
  This file combines all sensor-related functions:
  - Magnet sensor reading via RS485
  - Ultrasonic sensor reading via RS485
  - Sensor data processing and error calculation
  - Obstacle detection
*/

// ==================== MAIN SENSOR FUNCTIONS ====================

void bacaSensorGaris() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= SENSOR_READ_INTERVAL) {
    previousMillis = currentMillis;
    
    // Use unified RS485 communication for all sensors
    loopUnifiedRS485();
    
    // Update total active sensors
    updateTotalSensorAktif();
    
    // Update sensor state flags
    updateSensorFlags();
  }
}

void updateTotalSensorAktif() {
  sensorData.totalActiveSensors = 0;
  
  // Count active sensors from front magnet array
  for (int i = 0; i < MAX_MAGNET_SENSORS; i++) {
    if (sensorData.magnetFront[i]) {
      sensorData.totalActiveSensors++;
    }
  }
}

void updateSensorFlags() {
  // Update derived sensor flags based on current sensor data
  tengahAktif = false;
  kananHilang = true;
  kiriHilang = true;
  sensorkebacasemua = true;
  
  // Check center sensors (7-10)
  for (int i = 6; i < 10; i++) {
    if (sensorData.magnetFront[i]) {
      tengahAktif = true;
      break;
    }
  }
  
  // Check right sensors (11-15)
  for (int i = 10; i < 16; i++) {
    if (sensorData.magnetFront[i]) {
      kananHilang = false;
      break;
    }
  }
  
  // Check left sensors (0-5)
  for (int i = 0; i < 6; i++) {
    if (sensorData.magnetFront[i]) {
      kiriHilang = false;
      break;
    }
  }
  
  // Check if all sensors detect line
  sensorkebacasemua = (sensorData.totalActiveSensors >= 12);
}

// ==================== RS485 COMMUNICATION ====================

void loopUnifiedRS485() {
  unsigned long currentMillis = millis();
  
  // Switch between devices periodically
  if (currentMillis - lastDeviceSwitch >= DEVICE_SWITCH_INTERVAL) {
    lastDeviceSwitch = currentMillis;
    
    // Communicate with current device
    switch (currentDeviceAddress % MAX_DEVICES) {
      case 0: // Magnet Front
        communicateWithMagnetFront();
        break;
      case 1: // Ultrasonic Front
        communicateWithUltrasonicFront();
        break;
      case 2: // Ultrasonic Back
        communicateWithUltrasonicBack();
        break;
      case 3: // Magnet Back
        communicateWithMagnetBack();
        break;
    }
    
    // Move to next device
    currentDeviceAddress = (currentDeviceAddress + 1) % MAX_DEVICES;
  }
  
  // Process any incoming ultrasonic data
  processUltrasonicData();
  
  // Check device timeouts
  checkDeviceTimeouts();
}

void communicateWithMagnetFront() {
  uint8_t result = nodeMagnetFront.readHoldingRegisters(0x0000, 2);
  
  if (result == nodeMagnetFront.ku8MBSuccess) {
    deviceStatus[0].isOnline = true;
    deviceStatus[0].lastSuccessfulComm = millis();
    deviceStatus[0].errorCount = 0;
    
    uint16_t medianValue = nodeMagnetFront.getResponseBuffer(0);
    uint16_t positionValue = nodeMagnetFront.getResponseBuffer(1);
    
    DEBUG_PRINTF("Magnet Front - Median: %d, Position: 0x%04X\n", medianValue, positionValue);
    
    // Update magnet data
    updateMagnetData(positionValue, sensorData.magnetFront);
    
    // Calculate error for line following
    if (positionValue != 0xFFFF) {
      sensorData.errorValue = hitungErrorPosisi(positionValue);
    }
  } else {
    deviceStatus[0].isOnline = false;
    deviceStatus[0].errorCount++;
    
    if (deviceStatus[0].errorCount >= 5) {
      logError(ERROR_SENSOR_COMMUNICATION, "Magnet Front comm failed");
      deviceStatus[0].errorCount = 0;  // Reset to prevent spam
    }
  }
}

void communicateWithMagnetBack() {
  uint8_t result = nodeMagnetBack.readHoldingRegisters(0x0000, 2);
  
  if (result == nodeMagnetBack.ku8MBSuccess) {
    deviceStatus[3].isOnline = true;
    deviceStatus[3].lastSuccessfulComm = millis();
    deviceStatus[3].errorCount = 0;
    
    uint16_t medianValue = nodeMagnetBack.getResponseBuffer(0);
    uint16_t positionValue = nodeMagnetBack.getResponseBuffer(1);
    
    DEBUG_PRINTF("Magnet Back - Median: %d, Position: 0x%04X\n", medianValue, positionValue);
    
    // Update back magnet data
    updateMagnetData(positionValue, sensorData.magnetBack);
  } else {
    deviceStatus[3].isOnline = false;
    deviceStatus[3].errorCount++;
    
    if (deviceStatus[3].errorCount >= 5) {
      logError(ERROR_SENSOR_COMMUNICATION, "Magnet Back comm failed");
      deviceStatus[3].errorCount = 0;
    }
  }
}

void communicateWithUltrasonicFront() {
  // Ultrasonic sensors send data automatically
  // Just mark that we're expecting data from this device
  currentDeviceAddress = ADDR_ULTRASONIC_FRONT;
}

void communicateWithUltrasonicBack() {
  // Ultrasonic sensors send data automatically
  // Just mark that we're expecting data from this device
  currentDeviceAddress = ADDR_ULTRASONIC_BACK;
}

void processUltrasonicData() {
  if (Serial1.available()) {
    byte incomingByte = Serial1.read();
    
    if (!inPacket) {
      // Look for packet start with current device address
      if (incomingByte == currentDeviceAddress) {
        dataPacket[0] = incomingByte;
        byteCounter = 1;
        inPacket = true;
      }
    } else {
      // Continue filling packet buffer
      dataPacket[byteCounter] = incomingByte;
      byteCounter++;
      
      // If packet is complete
      if (byteCounter >= PACKET_LENGTH) {
        parseUltrasonicPacket();
        inPacket = false;
        byteCounter = 0;
      }
    }
  }
}

void parseUltrasonicPacket() {
  // Validate packet checksum
  uint16_t receivedCrc = (dataPacket[PACKET_LENGTH-2] << 8) | dataPacket[PACKET_LENGTH-1];
  uint16_t calculatedCrc = calculate_crc(dataPacket, PACKET_LENGTH-2);
  
  if (receivedCrc != calculatedCrc) {
    DEBUG_PRINTLN("Ultrasonic packet CRC error");
    return;
  }
  
  // Extract device address
  uint8_t deviceAddr = dataPacket[0];
  
  // Update device status
  int deviceIndex = (deviceAddr == ADDR_ULTRASONIC_FRONT) ? 1 : 2;
  deviceStatus[deviceIndex].isOnline = true;
  deviceStatus[deviceIndex].lastSuccessfulComm = millis();
  deviceStatus[deviceIndex].errorCount = 0;
  
  // Extract distance data (5 sensors, 2 bytes each)
  uint16_t* targetArray = (deviceAddr == ADDR_ULTRASONIC_FRONT) ? 
                         sensorData.ultrasonicFront : sensorData.ultrasonicBack;
  
  for (int i = 0; i < MAX_ULTRASONIC_SENSORS; i++) {
    targetArray[i] = (dataPacket[1 + i*2] << 8) | dataPacket[2 + i*2];
  }
  
  DEBUG_PRINTF("Ultrasonic %s: %d %d %d %d %d\n", 
               (deviceAddr == ADDR_ULTRASONIC_FRONT) ? "Front" : "Back",
               targetArray[0], targetArray[1], targetArray[2], targetArray[3], targetArray[4]);
}

void checkDeviceTimeouts() {
  unsigned long currentMillis = millis();
  
  for (int i = 0; i < MAX_DEVICES; i++) {
    if (deviceStatus[i].isOnline && 
        (currentMillis - deviceStatus[i].lastSuccessfulComm) > COMM_TIMEOUT_MS) {
      deviceStatus[i].isOnline = false;
      
      const char* deviceNames[] = {"Magnet Front", "Ultrasonic Front", "Ultrasonic Back", "Magnet Back"};
      logError(ERROR_SENSOR_COMMUNICATION, String(deviceNames[i]) + " timeout");
    }
  }
}

// ==================== SENSOR DATA PROCESSING ====================

void updateMagnetData(uint16_t bitmask, int* magnetArray) {
  // Convert bitmask to individual sensor values (active low)
  for (int i = 0; i < MAX_MAGNET_SENSORS; i++) {
    magnetArray[i] = !((bitmask >> i) & 0x01);
  }
}

int hitungErrorPosisi(uint16_t bitmask) {
  int jumlahSegmenAktif = 0;
  int segmenTertinggi = 0;
  int segmenTerendah = 17;
  
  // Count active segments and find range
  for (int i = 0; i < MAX_MAGNET_SENSORS; i++) {
    if (!((bitmask >> i) & 0x01)) {  // Active low
      jumlahSegmenAktif++;
      int segmenSaatIni = i + 1;
      if (segmenSaatIni < segmenTerendah) segmenTerendah = segmenSaatIni;
      if (segmenSaatIni > segmenTertinggi) segmenTertinggi = segmenSaatIni;
    }
  }
  
  if (jumlahSegmenAktif == 0) return 99;  // No line detected
  
  // Calculate error based on line position
  int errorKiri = 0, errorKanan = 0;
  
  // Left side error calculation
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
  
  // Right side error calculation
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
  
  // Return dominant error
  if (abs(errorKiri) > abs(errorKanan)) {
    return errorKiri;
  } else if (abs(errorKanan) > abs(errorKiri)) {
    return errorKanan;
  } else {
    return 0;  // Centered or balanced
  }
}

// ==================== OBSTACLE DETECTION ====================

void checkObstacles() {
  sensorData.obstacleDetected = false;
  
  // Check front obstacles when moving forward
  if (modeMaju) {
    sensorData.obstacleDetected = hasObstacle(true);
  }
  
  // Check back obstacles when moving backward
  if (modeMundur) {
    sensorData.obstacleDetected = hasObstacle(false);
  }
}

bool hasObstacle(bool checkFront) {
  uint16_t* distances = checkFront ? sensorData.ultrasonicFront : sensorData.ultrasonicBack;
  
  for (int i = 0; i < MAX_ULTRASONIC_SENSORS; i++) {
    if (distances[i] > 0 && distances[i] < systemConfig.minSafeDistance) {
      return true;
    }
  }
  return false;
}

void checkObstaclesFront() {
  if (hasObstacle(true)) {
    sensorData.obstacleDetected = true;
    DEBUG_PRINTLN("Front obstacle detected!");
  }
}

void checkObstaclesBack() {
  if (hasObstacle(false)) {
    sensorData.obstacleDetected = true;
    DEBUG_PRINTLN("Back obstacle detected!");
  }
}

// ==================== UTILITY FUNCTIONS ====================

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

// ==================== COMPATIBILITY FUNCTIONS ====================

// These functions maintain compatibility with existing code
int* getCurrentMagnetData() {
  return sensorData.magnetFront;
}

uint16_t* getUltrasonicDistancesFront() {
  return sensorData.ultrasonicFront;
}

uint16_t* getUltrasonicDistancesBack() {
  return sensorData.ultrasonicBack;
}

void printActiveSegmentsFromBitmask(uint16_t positionValue) {
  if (positionValue == 0xFFFF) {
    DEBUG_PRINTLN("No line detected");
    return;
  }
  
  DEBUG_PRINT("Active segments: ");
  for (int i = 0; i < MAX_MAGNET_SENSORS; i++) {
    if (!((positionValue >> i) & 0x01)) {
      DEBUG_PRINTF("%d ", i + 1);
    }
  }
  DEBUG_PRINTLN();
}