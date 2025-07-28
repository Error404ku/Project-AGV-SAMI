/*
  COMMUNICATION.INO - Communication System
  
  This file contains all communication-related functions for AGV SAMI:
  - RS485 communication with sensors
  - Web server communication
  - RFID communication
*/

// ==================== RS485 COMMUNICATION FUNCTIONS ====================

void loopUnifiedRS485() {
  static unsigned long lastDeviceSwitch = 0;
  static int currentDeviceIndex = 0;
  
  // Switch between devices at regular intervals
  if (millis() - lastDeviceSwitch >= DEVICE_SWITCH_INTERVAL) {
    lastDeviceSwitch = millis();
    
    // Cycle through devices
    currentDeviceIndex = (currentDeviceIndex + 1) % MAX_DEVICES;
    currentDeviceAddress = currentDeviceIndex + 1; // Device addresses start at 1
    
    // Read data from current device
    readDeviceData(currentDeviceAddress);
    
    // Update device status
    updateDeviceStatus(currentDeviceIndex);
  }
}

void readDeviceData(int deviceAddress) {
  uint8_t result;
  uint16_t data[16]; // Buffer for received data
  
  switch (deviceAddress) {
    case ADDR_MAGNET_FRONT:
      // Read magnet sensor data (front)
      result = nodeMagnetFront.readHoldingRegisters(0, 8);
      if (result == nodeMagnetFront.ku8MBSuccess) {
        // Process data
        for (int i = 0; i < 16; i++) {
          // Extract bit values from registers
          if (i < 8) {
            sensorData.magnetFront[i] = (nodeMagnetFront.getResponseBuffer(0) >> i) & 0x01;
          } else {
            sensorData.magnetFront[i] = (nodeMagnetFront.getResponseBuffer(1) >> (i-8)) & 0x01;
          }
        }
        deviceStatus[0].isOnline = true;
        deviceStatus[0].lastSuccessfulComm = millis();
        deviceStatus[0].lastSeen = millis();
      } else {
        deviceStatus[0].errorCount++;
        if (millis() - deviceStatus[0].lastSuccessfulComm > COMM_TIMEOUT_MS) {
          deviceStatus[0].isOnline = false;
        }
      }
      break;
      
    case ADDR_MAGNET_BACK:
      // Read magnet sensor data (back)
      result = nodeMagnetBack.readHoldingRegisters(0, 8);
      if (result == nodeMagnetBack.ku8MBSuccess) {
        // Process data
        for (int i = 0; i < 16; i++) {
          // Extract bit values from registers
          if (i < 8) {
            sensorData.magnetBack[i] = (nodeMagnetBack.getResponseBuffer(0) >> i) & 0x01;
          } else {
            sensorData.magnetBack[i] = (nodeMagnetBack.getResponseBuffer(1) >> (i-8)) & 0x01;
          }
        }
        deviceStatus[1].isOnline = true;
        deviceStatus[1].lastSuccessfulComm = millis();
        deviceStatus[1].lastSeen = millis();
      } else {
        deviceStatus[1].errorCount++;
        if (millis() - deviceStatus[1].lastSuccessfulComm > COMM_TIMEOUT_MS) {
          deviceStatus[1].isOnline = false;
        }
      }
      break;
      
    case ADDR_ULTRASONIC_FRONT:
      // Read ultrasonic sensor data (front)
      result = nodeMagnetFront.readHoldingRegisters(8, 5);
      if (result == nodeMagnetFront.ku8MBSuccess) {
        // Process data
        for (int i = 0; i < 5; i++) {
          sensorData.ultrasonicFront[i] = nodeMagnetFront.getResponseBuffer(i);
        }
        deviceStatus[2].isOnline = true;
        deviceStatus[2].lastSuccessfulComm = millis();
        deviceStatus[2].lastSeen = millis();
        
        // Check for obstacles
        checkObstacles(true);
      } else {
        deviceStatus[2].errorCount++;
        if (millis() - deviceStatus[2].lastSuccessfulComm > COMM_TIMEOUT_MS) {
          deviceStatus[2].isOnline = false;
        }
      }
      break;
      
    case ADDR_ULTRASONIC_BACK:
      // Read ultrasonic sensor data (back)
      result = nodeMagnetBack.readHoldingRegisters(8, 5);
      if (result == nodeMagnetBack.ku8MBSuccess) {
        // Process data
        for (int i = 0; i < 5; i++) {
          sensorData.ultrasonicBack[i] = nodeMagnetBack.getResponseBuffer(i);
        }
        deviceStatus[3].isOnline = true;
        deviceStatus[3].lastSuccessfulComm = millis();
        deviceStatus[3].lastSeen = millis();
        
        // Check for obstacles
        checkObstacles(false);
      } else {
        deviceStatus[3].errorCount++;
        if (millis() - deviceStatus[3].lastSuccessfulComm > COMM_TIMEOUT_MS) {
          deviceStatus[3].isOnline = false;
        }
      }
      break;
  }
}

void updateDeviceStatus(int deviceIndex) {
  // Check if device has timed out
  if (millis() - deviceStatus[deviceIndex].lastSeen > DEVICE_TIMEOUT_MS) {
    deviceStatus[deviceIndex].isOnline = false;
    
    // Log error if device was previously online
    if (deviceStatus[deviceIndex].isOnline) {
      logErrorComm(ERROR_SENSOR_COMMUNICATION, "Device " + String(deviceIndex + 1) + " timeout");
    }
  }
}

void checkObstacles(bool checkFront) {
  static unsigned long lastObstacleCheck = 0;
  
  // Check at regular intervals
  if (millis() - lastObstacleCheck < OBSTACLE_CHECK_INTERVAL) {
    return;
  }
  lastObstacleCheck = millis();
  
  // Check ultrasonic sensors for obstacles
  bool obstacleDetected = false;
  
  if (checkFront) {
    // Check front sensors
    for (int i = 0; i < 5; i++) {
      if (sensorData.ultrasonicFront[i] < systemConfig.minSafeDistance && 
          sensorData.ultrasonicFront[i] > 0) {
        obstacleDetected = true;
        break;
      }
    }
  } else {
    // Check back sensors
    for (int i = 0; i < 5; i++) {
      if (sensorData.ultrasonicBack[i] < systemConfig.minSafeDistance && 
          sensorData.ultrasonicBack[i] > 0) {
        obstacleDetected = true;
        break;
      }
    }
  }
  
  // Update sensor data
  sensorData.obstacleDetected = obstacleDetected;
  
  // Debug output
  if (obstacleDetected) {
    DEBUG_PRINTLN("Obstacle detected!");
  }
}

// ==================== WEB SERVER FUNCTIONS ====================

void handleUpdateStations() {
  String jsonData = server.arg("plain");
  
  // Parse JSON data
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonData);
  
  if (error) {
    server.send(400, "text/plain", "Invalid JSON data");
    return;
  }
  
  // Clear existing stations list
  stationsList.clear();
  
  // Add stations from JSON
  JsonArray stations = doc["stations"];
  for (JsonVariant station : stations) {
    stationsList.push_back(station.as<int>());
  }
  
  // Save to preferences
  saveStationsListToPreferences();
  
  // Send response
  server.send(200, "text/plain", "Stations updated successfully");
}

void handleShowStations() {
  // Create JSON response
  DynamicJsonDocument doc(1024);
  JsonArray stations = doc.createNestedArray("stations");
  
  for (int stationId : stationsList) {
    stations.add(stationId);
  }
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
}

void saveStationsListToPreferences() {
  preferences.begin(PREF_NAMESPACE_STATIONS, false);
  
  // Convert vector to array for storage
  int stationsArray[MAX_MANUAL_TARGETS];
  int count = min((int)stationsList.size(), MAX_MANUAL_TARGETS);
  
  for (int i = 0; i < count; i++) {
    stationsArray[i] = stationsList[i];
  }
  
  // Save count and data
  preferences.putInt("count", count);
  preferences.putBytes("stations", stationsArray, count * sizeof(int));
  
  preferences.end();
  
  DEBUG_PRINTF("Saved %d stations to preferences\n", count);
}

void loadStationsListFromPreferences() {
  preferences.begin(PREF_NAMESPACE_STATIONS, true);
  
  // Clear existing list
  stationsList.clear();
  
  // Get count and data
  int count = preferences.getInt("count", 0);
  
  if (count > 0 && count <= MAX_MANUAL_TARGETS) {
    int stationsArray[MAX_MANUAL_TARGETS];
    size_t bytesRead = preferences.getBytes("stations", stationsArray, count * sizeof(int));
    
    if (bytesRead == count * sizeof(int)) {
      for (int i = 0; i < count; i++) {
        stationsList.push_back(stationsArray[i]);
      }
      DEBUG_PRINTF("Loaded %d stations from preferences\n", count);
    }
  }
  
  preferences.end();
  
  // If no stations loaded, add default station
  if (stationsList.empty()) {
    stationsList.push_back(1);
    DEBUG_PRINTLN("No stations found, added default station 1");
  }
}

// ==================== ERROR LOGGING ====================

// Renamed to avoid conflict with agv_sami_optimized.ino
void logErrorComm(ErrorCode code, String message) {
  // Print error to debug output
  DEBUG_PRINTF("ERROR [%d]: %s\n", code, message.c_str());
  
  // Store in preferences if needed
  preferences.begin(PREF_NAMESPACE_ERRORS, false);
  
  // Get error count
  int errorCount = preferences.getInt("count", 0);
  
  // Store new error
  String errorKey = "err_" + String(errorCount);
  String errorValue = String(code) + ": " + message;
  
  preferences.putString(errorKey.c_str(), errorValue);
  preferences.putInt("count", errorCount + 1);
  
  preferences.end();
  
  // Play error sound
  music("error");
}