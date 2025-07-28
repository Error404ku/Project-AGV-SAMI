// Unified RS485 Communication Manager
// Handles communication with multiple devices on single RS485 bus
// Device addresses: 1=magnet front, 2=ultrasonic front, 3=ultrasonic back, 4=magnet back

// Optimized communication timing with priority system
unsigned long lastDeviceSwitch = 0;
int currentDeviceIndex = 0;
const int totalDevices = 4;

// Device priority configuration (0=highest priority)
const int devicePriority[4] = {0, 1, 3, 2}; // Magnet Front, Ultrasonic Front, Magnet Back, Ultrasonic Back
const unsigned long deviceSwitchInterval[4] = {50, 100, 150, 120}; // Different intervals based on priority
const unsigned long deviceTimeout[4] = {3000, 5000, 5000, 3000}; // Different timeouts per device type

// Device communication status
bool deviceOnline[4] = {false, false, false, false};
// lastSuccessfulComm sudah dideklarasikan di debug_rs485.ino
unsigned long lastCommAttempt[4] = {0, 0, 0, 0};
int commFailureCount[4] = {0, 0, 0, 0};
const int maxCommFailures = 3; // Max failures before marking device offline

void setupUnifiedRS485() {
  Serial.println("=== SETUP UNIFIED RS485 SYSTEM ===");
  
  // Setup debug system
  setupRS485Debug();
  
  // Setup RS485 control pins
  pinMode(MAX485_DE, OUTPUT);
  pinMode(MAX485_RE, OUTPUT);
  digitalWrite(MAX485_RE, 0);
  digitalWrite(MAX485_DE, 0);
  Serial.println("RS485 control pins initialized");
  
  // Initialize unified serial communication
  Serial1.begin(BAUDRATE_RS485, SERIAL_8N1, RX_RS485, TX_RS485);
  Serial.print("Serial1 initialized at baudrate: ");
  Serial.println(BAUDRATE_RS485);
  
  // Initialize Modbus masters for magnet sensors
  nodeMagnetFront.begin(ADDR_MAGNET_FRONT, Serial1);
  nodeMagnetFront.preTransmission(preTransmission);
  nodeMagnetFront.postTransmission(postTransmission);
  Serial.print("Magnet Front Modbus initialized with address: ");
  Serial.println(ADDR_MAGNET_FRONT);
  
  nodeMagnetBack.begin(ADDR_MAGNET_BACK, Serial1);
  nodeMagnetBack.preTransmission(preTransmission);
  nodeMagnetBack.postTransmission(postTransmission);
  Serial.print("Magnet Back Modbus initialized with address: ");
  Serial.println(ADDR_MAGNET_BACK);
  
  // Initialize sensor data arrays
  for (int i = 0; i < 16; i++) {
    jumlahMagnetFront[i] = 0;
    jumlahMagnetBack[i] = 0;
  }
  
  for (int i = 0; i < 5; i++) {
    ultrasonicDistancesFront[i] = 0;
    ultrasonicDistancesBack[i] = 0;
  }
  
  Serial.println("Sensor data arrays initialized");
  Serial.println("=== UNIFIED RS485 SETUP COMPLETE ===");
  
  // Run initial diagnosis
  diagnoseCommonIssues();
}

void loopUnifiedRS485() {
  unsigned long currentMillis = millis();
  
  // Priority-based device switching
  if (currentMillis - lastDeviceSwitch >= deviceSwitchInterval[currentDeviceIndex]) {
    lastDeviceSwitch = currentMillis;
    
    // Skip device if it has too many failures and isn't critical
    if (commFailureCount[currentDeviceIndex] >= maxCommFailures && devicePriority[currentDeviceIndex] > 1) {
      currentDeviceIndex = getNextPriorityDevice();
      return;
    }
    
    // Debug: Show current device being communicated with (only in detailed debug)
    const char* deviceNames[] = {"Magnet Front", "Ultrasonic Front", "Ultrasonic Back", "Magnet Back"};
    if (enableDetailedDebug) {
      Serial.print("Communicating with: ");
      Serial.print(deviceNames[currentDeviceIndex]);
      Serial.print(" (Priority: ");
      Serial.print(devicePriority[currentDeviceIndex]);
      Serial.println(")");
    }
    
    lastCommAttempt[currentDeviceIndex] = currentMillis;
    
    // Communicate with current device
    switch (currentDeviceIndex) {
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
    
    // Move to next priority device
    currentDeviceIndex = getNextPriorityDevice();
  }
  
  // Process any incoming ultrasonic data
  processUltrasonicData();
  
  // Check device timeouts
  checkDeviceTimeouts();
  
  // Run debug loop
  debugRS485Loop();
}

void communicateWithMagnetFront() {
  currentDeviceAddress = ADDR_MAGNET_FRONT;
  unsigned long startTime = millis();
  
  // Add small delay before communication
  delay(10);
  
  uint8_t result = nodeMagnetFront.readHoldingRegisters(0x0000, 2);
  
  if (result == nodeMagnetFront.ku8MBSuccess) {
    deviceOnline[0] = true;
    lastSuccessfulComm[0] = millis();
    updateMagnetFrontStats(true);
    
    uint16_t medianValue = nodeMagnetFront.getResponseBuffer(0);
    uint16_t positionValue = nodeMagnetFront.getResponseBuffer(1);
    
    // Debug output (comment out in production)
    if (enableRS485Debug) {
      Serial.print("Magnet Front - Median: ");
      Serial.print(medianValue);
      Serial.print(", Position: 0x");
      Serial.println(positionValue, HEX);
    }
    
    updateMagnetData(positionValue, jumlahMagnetFront);
    
    if (positionValue != 0xFFFF) {
      // Calculate error for front magnet
      errorValue = hitungErrorPosisi(positionValue);
    }
    
    debugTiming("Magnet Front Comm", startTime);
  } else {
    deviceOnline[0] = false;
    updateMagnetFrontStats(false, (result == 0xE2));
    debugModbusError(result, "Magnet Front");
  }
}

void communicateWithMagnetBack() {
  currentDeviceAddress = ADDR_MAGNET_BACK;
  unsigned long startTime = millis();
  
  // Add small delay before communication
  delay(10);
  
  uint8_t result = nodeMagnetBack.readHoldingRegisters(0x0000, 2);
  
  if (result == nodeMagnetBack.ku8MBSuccess) {
    deviceOnline[3] = true;
    lastSuccessfulComm[3] = millis();
    updateMagnetBackStats(true);
    
    uint16_t medianValue = nodeMagnetBack.getResponseBuffer(0);
    uint16_t positionValue = nodeMagnetBack.getResponseBuffer(1);
    
    // Debug output (comment out in production)
    if (enableRS485Debug) {
      Serial.print("Magnet Back - Median: ");
      Serial.print(medianValue);
      Serial.print(", Position: 0x");
      Serial.println(positionValue, HEX);
    }
    
    updateMagnetData(positionValue, jumlahMagnetBack);
    debugTiming("Magnet Back Comm", startTime);
  } else {
    deviceOnline[3] = false;
    updateMagnetBackStats(false, (result == 0xE2));
    debugModbusError(result, "Magnet Back");
  }
}

void communicateWithUltrasonicFront() {
  currentDeviceAddress = ADDR_ULTRASONIC_FRONT;
  // Ultrasonic sensors send data automatically, just set the current address
  // Data will be processed in processUltrasonicData()
}

void communicateWithUltrasonicBack() {
  currentDeviceAddress = ADDR_ULTRASONIC_BACK;
  // Ultrasonic sensors send data automatically, just set the current address
  // Data will be processed in processUltrasonicData()
}

void processUltrasonicData() {
  if (Serial1.available()) {
    byte incomingByte = Serial1.read();
    
    // Check if this byte matches current expected device address
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
  // Debug packet data
  debugPacketData(dataPacket, PACKET_LENGTH, "RX");
  
  // Verify packet header
  if (dataPacket[0] == currentDeviceAddress && dataPacket[1] == 0x03) {
    
    // Validate CRC
    uint16_t calculated_crc = calculate_crc(dataPacket, PACKET_LENGTH - 2);
    uint16_t received_crc = (dataPacket[PACKET_LENGTH - 1] << 8) | dataPacket[PACKET_LENGTH - 2];
    
    if (calculated_crc == received_crc) {
      // Extract distances
      uint16_t dist1 = (dataPacket[3] << 8) | dataPacket[4];
      uint16_t dist2 = (dataPacket[5] << 8) | dataPacket[6];
      uint16_t dist3 = (dataPacket[7] << 8) | dataPacket[8];
      uint16_t dist4 = (dataPacket[9] << 8) | dataPacket[10];
      uint16_t dist5 = (dataPacket[11] << 8) | dataPacket[12];
      
      // Store data based on device address
      if (currentDeviceAddress == ADDR_ULTRASONIC_FRONT) {
        deviceOnline[1] = true;
        lastSuccessfulComm[1] = millis();
        updateUltrasonicFrontStats(true);
        
        ultrasonicDistancesFront[0] = dist1;
        ultrasonicDistancesFront[1] = dist2;
        ultrasonicDistancesFront[2] = dist3;
        ultrasonicDistancesFront[3] = dist4;
        ultrasonicDistancesFront[4] = dist5;
        
        if (enableDetailedDebug) {
          Serial.print("Ultrasonic Front: ");
          Serial.print(dist1); Serial.print(", ");
          Serial.print(dist2); Serial.print(", ");
          Serial.print(dist3); Serial.print(", ");
          Serial.print(dist4); Serial.print(", ");
          Serial.println(dist5);
        }
        
        checkObstaclesFront();
      } else if (currentDeviceAddress == ADDR_ULTRASONIC_BACK) {
        deviceOnline[2] = true;
        lastSuccessfulComm[2] = millis();
        updateUltrasonicBackStats(true);
        
        ultrasonicDistancesBack[0] = dist1;
        ultrasonicDistancesBack[1] = dist2;
        ultrasonicDistancesBack[2] = dist3;
        ultrasonicDistancesBack[3] = dist4;
        ultrasonicDistancesBack[4] = dist5;
        
        if (enableDetailedDebug) {
          Serial.print("Ultrasonic Back: ");
          Serial.print(dist1); Serial.print(", ");
          Serial.print(dist2); Serial.print(", ");
          Serial.print(dist3); Serial.print(", ");
          Serial.print(dist4); Serial.print(", ");
          Serial.println(dist5);
        }
        
        checkObstaclesBack();
      }
    } else {
      // CRC Error
      if (currentDeviceAddress == ADDR_ULTRASONIC_FRONT) {
        updateUltrasonicFrontStats(false, true);
      } else if (currentDeviceAddress == ADDR_ULTRASONIC_BACK) {
        updateUltrasonicBackStats(false, true);
      }
      
      if (enableRS485Debug) {
        Serial.print("[CRC ERROR] Expected: 0x");
        Serial.print(calculated_crc, HEX);
        Serial.print(", Received: 0x");
        Serial.println(received_crc, HEX);
      }
      
      error(ERROR_ULTRASONIC_COMMUNICATION, "CRC Checksum tidak cocok");
    }
  } else {
    if (enableRS485Debug) {
      Serial.print("[PACKET ERROR] Invalid header: ");
      Serial.print(dataPacket[0], HEX);
      Serial.print(" ");
      Serial.println(dataPacket[1], HEX);
    }
  }
}

void updateMagnetData(uint16_t bitmask, uint8_t* magnetArray) {
  for (int i = 0; i < 16; i++) {
    magnetArray[i] = !((bitmask >> i) & 0x01) ? 1 : 0;
  }
}

void checkObstaclesFront() {
  bool obstacleDetected = false;
  
  for (int i = 0; i < 5; i++) {
    if (ultrasonicDistancesFront[i] > 0 && ultrasonicDistancesFront[i] < minSafeDistance) {
      obstacleDetected = true;
      break;
    }
  }
  
  if (obstacleDetected) {
    music("error");
  }
}

void checkObstaclesBack() {
  bool obstacleDetected = false;
  
  for (int i = 0; i < 5; i++) {
    if (ultrasonicDistancesBack[i] > 0 && ultrasonicDistancesBack[i] < minSafeDistance) {
      obstacleDetected = true;
      break;
    }
  }
  
  if (obstacleDetected) {
    music("error");
  }
}

// Get next device based on priority
int getNextPriorityDevice() {
  // Find highest priority device that needs communication
  int nextDevice = (currentDeviceIndex + 1) % totalDevices;
  int bestPriority = 999;
  int bestDevice = nextDevice;
  
  for (int i = 0; i < totalDevices; i++) {
    int deviceIndex = (currentDeviceIndex + 1 + i) % totalDevices;
    if (devicePriority[deviceIndex] < bestPriority) {
      bestPriority = devicePriority[deviceIndex];
      bestDevice = deviceIndex;
    }
  }
  
  return bestDevice;
}

void checkDeviceTimeouts() {
  unsigned long currentMillis = millis();
  
  for (int i = 0; i < 4; i++) {
    if (deviceOnline[i] && (currentMillis - lastSuccessfulComm[i] > deviceTimeout[i])) {
      deviceOnline[i] = false;
      commFailureCount[i]++;
      
      // Only log error for critical devices or first few failures
      if (devicePriority[i] <= 1 || commFailureCount[i] <= 2) {
        switch (i) {
          case 0:
            logError(ERROR_SENSOR_COMMUNICATION, "Magnet Front timeout");
            break;
          case 1:
            logError(ERROR_ULTRASONIC_COMMUNICATION, "Ultrasonic Front timeout");
            break;
          case 2:
            logError(ERROR_ULTRASONIC_COMMUNICATION, "Ultrasonic Back timeout");
            break;
          case 3:
            logError(ERROR_SENSOR_COMMUNICATION, "Magnet Back timeout");
            break;
        }
      }
    }
    
    // Reset failure count on successful communication
    if (deviceOnline[i] && commFailureCount[i] > 0) {
      commFailureCount[i] = max(0, commFailureCount[i] - 1);
    }
  }
}

// Get current magnet data for line following (uses front magnet by default)
uint8_t* getCurrentMagnetData() {
  return jumlahMagnetFront;
}

// Get magnet data for specific position
uint8_t* getMagnetData(bool useFront) {
  return useFront ? jumlahMagnetFront : jumlahMagnetBack;
}

// Get ultrasonic data for specific position
uint16_t* getUltrasonicData(bool useFront) {
  return useFront ? ultrasonicDistancesFront : ultrasonicDistancesBack;
}

// Check if specific device is online
bool isDeviceOnline(int deviceIndex) {
  return deviceOnline[deviceIndex];
}

// Get device status string
String getDeviceStatusString() {
  String status = "Devices: ";
  status += deviceOnline[0] ? "MF:OK " : "MF:ERR ";
  status += deviceOnline[1] ? "UF:OK " : "UF:ERR ";
  status += deviceOnline[2] ? "UB:OK " : "UB:ERR ";
  status += deviceOnline[3] ? "MB:OK" : "MB:ERR";
  return status;
}