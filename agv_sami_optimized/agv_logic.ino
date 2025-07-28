/*
  AGV_LOGIC.INO - Main AGV Logic and Mode Control
  
  This file contains the main AGV operational logic:
  - Mode-specific behaviors
  - Line following logic
  - Station navigation
  - Manual control
  - Safety systems
*/

// ==================== MAIN AGV LOGIC ====================

void runAGVLogic() {
  // Main AGV logic dispatcher based on current mode
  switch (systemState.currentMode) {
    case MODE_IDLE:
      runIdleMode();
      break;
    case MODE_MANUAL:
      runManualMode();
      break;
    case MODE_LINE_FOLLOW:
      runLineFollowMode();
      break;
    case MODE_STATION:
      runStationMode();
      break;
    default:
      // Unknown mode, switch to idle
      systemState.currentMode = MODE_IDLE;
      logError(ERROR_SYSTEM_STATE, "Unknown AGV mode, switching to idle");
      break;
  }
  
  // Update hook status regardless of mode
  updateHookStatus();
}

// ==================== MODE IMPLEMENTATIONS ====================

void runIdleMode() {
  // Idle mode: AGV is stopped and waiting
  static bool hasStoppedMotors = false;
  
  if (!hasStoppedMotors) {
    stopMovement();
    hasStoppedMotors = true;
    DEBUG_PRINTLN("AGV entered idle mode");
  }
  
  // Reset flag when leaving idle mode
  if (systemState.currentMode != MODE_IDLE) {
    hasStoppedMotors = false;
  }
}

void runManualMode() {
  // Manual mode: AGV responds to direct commands
  // Motor control is handled by web interface or button commands
  // This mode doesn't have autonomous behavior
  
  static unsigned long lastManualCheck = 0;
  
  if (millis() - lastManualCheck > 1000) {
    DEBUG_PRINTLN("AGV in manual mode - awaiting commands");
    lastManualCheck = millis();
  }
  
  // Safety check: stop if obstacle detected
  if (sensorData.obstacleDetected && systemState.currentMovement != MOVEMENT_STOP) {
    emergencyStop();
    DEBUG_PRINTLN("Manual mode: Emergency stop due to obstacle");
  }
}

void runLineFollowMode() {
  // Line following mode: AGV follows magnetic line
  
  // Check if we have valid sensor data
  if (!deviceStatus[0].isOnline && !deviceStatus[1].isOnline) {
    stopMovement();
    DEBUG_PRINTLN("Line follow mode: No magnet sensors online");
    return;
  }
  
  // Safety check: stop if obstacle detected
  if (sensorData.obstacleDetected) {
    pidLinefollower(0, "BERHENTI");
    DEBUG_PRINTLN("Line follow mode: Stopped due to obstacle");
    return;
  }
  
  // Check if we're on the line
  if (totalSensorAktif == 0) {
    handleLineSearching();
    return;
  }
  
  // Normal line following with PID
  int errorPosisi = hitungErrorPosisi();
  pidLinefollower(errorPosisi, "MAJU");
  
  DEBUG_PRINTF("Line follow: Error=%d, Sensors=%d\n", errorPosisi, totalSensorAktif);
}

void runStationMode() {
  // Station mode: AGV follows line and responds to RFID stations
  
  // Check if we have valid sensor data
  if (!deviceStatus[0].isOnline && !deviceStatus[1].isOnline) {
    stopMovement();
    DEBUG_PRINTLN("Station mode: No magnet sensors online");
    return;
  }
  
  // Safety check: stop if obstacle detected
  if (sensorData.obstacleDetected) {
    pidLinefollower(0, "BERHENTI");
    DEBUG_PRINTLN("Station mode: Stopped due to obstacle");
    return;
  }
  
  // Check if we're on the line
  if (totalSensorAktif == 0) {
    handleLineSearching();
    return;
  }
  
  // Check if we recently scanned an RFID
  if (millis() - lastRfidScanTime < 3000) {
    // Recently scanned RFID, might be executing station action
    DEBUG_PRINTLN("Station mode: Recent RFID scan, checking station action");
    return;
  }
  
  // Normal line following with PID
  int errorPosisi = hitungErrorPosisi();
  pidLinefollower(errorPosisi, "MAJU");
  
  DEBUG_PRINTF("Station mode: Error=%d, Sensors=%d, Station=%d\n", 
               errorPosisi, totalSensorAktif, currentStationId);
}

// ==================== LINE FOLLOWING LOGIC ====================

void handleLineSearching() {
  // Handle situation when AGV loses the line
  static unsigned long lineSearchStartTime = 0;
  static bool isSearching = false;
  static int searchDirection = 1; // 1 for right, -1 for left
  
  if (!isSearching) {
    lineSearchStartTime = millis();
    isSearching = true;
    searchDirection = (lastError > 0) ? 1 : -1; // Search in direction of last error
    DEBUG_PRINTLN("Line lost - starting search");
  }
  
  unsigned long searchTime = millis() - lineSearchStartTime;
  
  if (searchTime > LINE_SEARCH_TIMEOUT_MS) {
    // Search timeout, stop AGV
    stopMovement();
    isSearching = false;
    logError(ERROR_LINE_LOST, "Line search timeout");
    DEBUG_PRINTLN("Line search timeout - stopping AGV");
    music("error");
    return;
  }
  
  // Execute search pattern
  if (searchTime < 1000) {
    // First second: turn in the direction of last error
    if (searchDirection > 0) {
      turnRight(systemConfig.baseSpeed / 2);
    } else {
      turnLeft(systemConfig.baseSpeed / 2);
    }
  } else if (searchTime < 3000) {
    // Next 2 seconds: turn in opposite direction
    if (searchDirection > 0) {
      turnLeft(systemConfig.baseSpeed / 2);
    } else {
      turnRight(systemConfig.baseSpeed / 2);
    }
  } else {
    // Last phase: move backward slowly
    moveBackward(systemConfig.baseSpeed / 3);
  }
  
  // Check if line is found
  if (totalSensorAktif > 0) {
    isSearching = false;
    DEBUG_PRINTLN("Line found - resuming normal operation");
    music("connect");
  }
}

int hitungErrorPosisi() {
  // Calculate position error based on magnet sensor readings
  
  if (totalSensorAktif == 0) {
    return lastError; // No sensors active, use last known error
  }
  
  // Get current magnet sensor data
  uint16_t frontMagnet = getCurrentMagnetDataBitmask(true);
  uint16_t backMagnet = getCurrentMagnetDataBitmask(false);
  
  // Use front sensors for primary calculation
  uint16_t activeSensors = frontMagnet;
  
  // If no front sensors, use back sensors
  if (activeSensors == 0) {
    activeSensors = backMagnet;
  }
  
  // Calculate weighted position
  int weightedSum = 0;
  int totalWeight = 0;
  
  for (int i = 0; i < 16; i++) {
    if (activeSensors & (1 << i)) {
      // Sensor positions: -7.5 to +7.5 (center at 0)
      int position = (i - 7.5) * 10; // Scale for better resolution
      weightedSum += position;
      totalWeight++;
    }
  }
  
  int error = 0;
  if (totalWeight > 0) {
    error = weightedSum / totalWeight;
  }
  
  // Apply smoothing to reduce noise
  static int previousError = 0;
  error = (error + previousError) / 2;
  previousError = error;
  
  // Store for line search logic
  lastError = error;
  
  return error;
}

void updateSensorFlags() {
  // Update sensor status flags based on current readings
  uint16_t frontMagnet = getCurrentMagnetDataBitmask(true);
  uint16_t backMagnet = getCurrentMagnetDataBitmask(false);
  
  // Check center sensors (bits 6, 7, 8, 9)
  tengahAktif = (frontMagnet & 0x03C0) != 0;
  
  // Check left sensors (bits 0-5)
  kiriHilang = (frontMagnet & 0x003F) == 0;
  
  // Check right sensors (bits 10-15)
  kananHilang = (frontMagnet & 0xFC00) == 0;
  
  // Update total active sensors
  totalSensorAktif = 0;
  for (int i = 0; i < 16; i++) {
    if (frontMagnet & (1 << i)) totalSensorAktif++;
    if (backMagnet & (1 << i)) totalSensorAktif++;
  }
  
  DEBUG_PRINTF("Sensor flags: Center=%d, LeftLost=%d, RightLost=%d, Total=%d\n", 
               tengahAktif, kiriHilang, kananHilang, totalSensorAktif);
}

// ==================== NAVIGATION FUNCTIONS ====================

void navigateToStation(int targetStationId) {
  // Navigate to a specific station
  DEBUG_PRINTF("Navigating to station %d\n", targetStationId);
  
  // Set target station
  currentStationId = targetStationId;
  
  // Switch to station mode if not already
  if (systemState.currentMode != MODE_STATION) {
    systemState.currentMode = MODE_STATION;
  }
  
  // Start line following towards the station
  // The RFID system will handle station detection
}

void executeEmergencyStop() {
  // Execute emergency stop procedure
  emergencyStop();
  
  // Log the emergency stop
  logError(ERROR_EMERGENCY_STOP, "Emergency stop executed");
  
  // Switch to idle mode
  systemState.currentMode = MODE_IDLE;
  
  // Provide feedback
  music("error");
  displayMessage("EMERGENCY STOP", "Check system", 0);
  
  DEBUG_PRINTLN("Emergency stop executed");
}

void resumeOperation() {
  // Resume operation after emergency stop
  if (systemState.currentMode == MODE_IDLE) {
    // Check if it's safe to resume
    if (isMotorSafe()) {
      systemState.currentMode = MODE_LINE_FOLLOW;
      DEBUG_PRINTLN("Operation resumed - switching to line follow mode");
      music("connect");
    } else {
      DEBUG_PRINTLN("Cannot resume - system not safe");
      music("error");
    }
  }
}

// ==================== SAFETY SYSTEMS ====================

// Renamed to avoid conflict with main.ino
void checkSafetyConditionsLogic() {
  // Check various safety conditions
  
  // Check for obstacles in movement direction
  if (systemState.currentMovement == MOVEMENT_FORWARD && hasObstacle(true)) {
    executeEmergencyStop();
    DEBUG_PRINTLN("Safety: Forward obstacle detected");
    return;
  }
  
  if (systemState.currentMovement == MOVEMENT_BACKWARD && hasObstacle(false)) {
    executeEmergencyStop();
    DEBUG_PRINTLN("Safety: Backward obstacle detected");
    return;
  }
  
  // Check sensor communication timeouts
  bool criticalSensorOffline = false;
  for (int i = 0; i < 2; i++) { // Check magnet sensors
    if (millis() - deviceStatus[i].lastSeen > DEVICE_TIMEOUT_MS) {
      criticalSensorOffline = true;
      break;
    }
  }
  
  if (criticalSensorOffline && systemState.currentMovement != MOVEMENT_STOP) {
    executeEmergencyStop();
    DEBUG_PRINTLN("Safety: Critical sensor offline");
    return;
  }
  
  // Check if AGV has been stuck (not moving) for too long
  static unsigned long lastMovementTime = 0;
  static MovementState lastMovementStatus = MOVEMENT_STOP;
  
  if (systemState.currentMovement != lastMovementStatus) {
    lastMovementTime = millis();
    lastMovementStatus = systemState.currentMovement;
  }
  
  if (systemState.currentMovement != MOVEMENT_STOP && 
      millis() - lastMovementTime > STUCK_TIMEOUT_MS) {
    executeEmergencyStop();
    DEBUG_PRINTLN("Safety: AGV appears to be stuck");
    logError(ERROR_SYSTEM_STUCK, "AGV stuck for too long");
    return;
  }
}

void performSystemHealthCheck() {
  // Perform comprehensive system health check
  static unsigned long lastHealthCheck = 0;
  
  if (millis() - lastHealthCheck < HEALTH_CHECK_INTERVAL_MS) {
    return;
  }
  lastHealthCheck = millis();
  
  DEBUG_PRINTLN("=== SYSTEM HEALTH CHECK ===");
  
  // Check device communication
  for (int i = 0; i < 4; i++) {
    unsigned long timeSinceLastSeen = millis() - deviceStatus[i].lastSeen;
    DEBUG_PRINTF("Device %d: %s (Last seen: %lu ms ago)\n", 
                 i, deviceStatus[i].isOnline ? "ONLINE" : "OFFLINE", timeSinceLastSeen);
  }
  
  // Check sensor readings
  DEBUG_PRINTF("Magnet sensors active: %d/32\n", totalSensorAktif);
  DEBUG_PRINTF("Obstacle detected: %s\n", sensorData.obstacleDetected ? "YES" : "NO");
  
  // Check system state
  DEBUG_PRINTF("Current mode: %s\n", getModeString().c_str());
  DEBUG_PRINTF("Current movement: %s\n", getMovementString().c_str());
  DEBUG_PRINTF("Hook status: %s\n", getHookString().c_str());
  
  // Check memory usage
  DEBUG_PRINTF("Free heap: %d bytes\n", ESP.getFreeHeap());
  
  DEBUG_PRINTLN("=== END HEALTH CHECK ===");
}

// ==================== DIAGNOSTIC FUNCTIONS ====================

void runSystemDiagnostics() {
  DEBUG_PRINTLN("=== SYSTEM DIAGNOSTICS ===");
  
  // Test all subsystems
  testRfidSystem();
  testAllSensors();
  
  // Test motor system
  DEBUG_PRINTLN("Testing motor system...");
  testMotorForward(500);
  delay(1000);
  testMotorBackward(500);
  delay(1000);
  
  // Test hook system
  DEBUG_PRINTLN("Testing hook system...");
  setHookUp();
  delay(2000);
  setHookDown();
  delay(2000);
  
  // Test communication
  DEBUG_PRINTLN("Testing communication...");
  for (int i = 0; i < 10; i++) {
    loopUnifiedRS485();
    delay(100);
  }
  
  DEBUG_PRINTLN("=== DIAGNOSTICS COMPLETE ===");
}

void calibrateSystem() {
  DEBUG_PRINTLN("=== SYSTEM CALIBRATION ===");
  
  // Calibrate sensors
  DEBUG_PRINTLN("Calibrating sensors...");
  for (int i = 0; i < 50; i++) {
    bacaSensorGaris();
    delay(20);
  }
  
  // Reset PID
  DEBUG_PRINTLN("Resetting PID controllers...");
  resetAllPID();
  
  // Test basic movements
  DEBUG_PRINTLN("Testing basic movements...");
  moveForward(systemConfig.baseSpeed / 2);
  delay(1000);
  stopMovement();
  delay(500);
  
  moveBackward(systemConfig.baseSpeed / 2);
  delay(1000);
  stopMovement();
  
  DEBUG_PRINTLN("=== CALIBRATION COMPLETE ===");
  music("startup");
}

// ==================== COMPATIBILITY FUNCTIONS ====================

// These functions maintain compatibility with existing code
void modeAGV() {
  // Legacy function - now handled by runAGVLogic()
  runAGVLogic();
}

void prosesMode() {
  // Legacy function - now handled by runAGVLogic()
  runAGVLogic();
}

void cekMode() {
  // Legacy function - mode checking is now integrated
  DEBUG_PRINTF("Current AGV mode: %s\n", getModeString().c_str());
}

void logicAgv() {
  // Legacy function - now handled by runAGVLogic()
  runAGVLogic();
}

void inStation() {
  // Station entry actions
  music("station");
  DEBUG_PRINTF("Entered station %d\n", currentStationId);
}

void outStation() {
  // Station exit actions
  // Check if this is the last station in the list
  if (indexTarget >= stationsList.size()) {
    DEBUG_PRINTLN("Last station reached via outStation - calling ujungStation!");
    ujungStation();
    return;
  }
  
  // Continue to next station
  indexTarget++;
  DEBUG_PRINTF("Exiting station, next target index: %d\n", indexTarget);
}

void ujungStation() {
  // End station - reverse to warehouse
  DEBUG_PRINTLN("Reached end station - reversing to warehouse");
  
  // Stop current movement
  stopMovement();
  
  // Set reverse mode
  systemState.currentMode = MODE_MANUAL;
  
  // Move backward
  moveBackward(systemConfig.baseSpeed);
  
  // Reset station tracking
  indexTarget = 0;
  currentStationId = 1;
}

void pembacaanStation() {
  // Station detection and processing
  if (lastScannedRfid.length() > 0) {
    int detectedStationId = findRfidStation(lastScannedRfid);
    if (detectedStationId > 0) {
      // Use RFID station ID directly
      station = detectedStationId;
      currentStationId = detectedStationId;
      DEBUG_PRINTF("RFID detected - Station ID: %d\n", detectedStationId);
      
      // Execute station action
      executeStationAction(detectedStationId);
      
      // Mark as processed
      lastScannedRfid = "";
    }
  }
}

// ==================== MODE FUNCTIONS (LEGACY COMPATIBILITY) ====================

void setModeStation() {
  // Set station mode
  systemState.currentMode = MODE_STATION;
  DEBUG_PRINTLN("Mode set to Station");
}

void setModeWarehouse() {
  // Set warehouse mode (manual mode)
  systemState.currentMode = MODE_MANUAL;
  DEBUG_PRINTLN("Mode set to Warehouse (Manual)");
}

void setModeTerminal() {
  // Set terminal mode (idle mode)
  systemState.currentMode = MODE_IDLE;
  stopMovement();
  DEBUG_PRINTLN("Mode set to Terminal (Idle)");
}