/*
  PERFORMANCE OPTIMIZATION SYSTEM FOR AGV SAMI

  This system provides:
  1. Non-blocking timer management
  2. Performance monitoring
  3. Memory usage tracking
  4. Loop execution time measurement
  5. Optimized state management

  Author: AGV SAMI Team
  Date: 2024
*/

// ===================================================================
// PERFORMANCE OPTIMIZATION VARIABLES SUDAH DIPINDAHKAN KE config.h
// ===================================================================

/***********************************************************
 *  TIMER MANAGEMENT FUNCTIONS                            *
 ***********************************************************/
void startTimer(Timer* timer, unsigned long interval) {
  timer->previousMillis = millis();
  timer->interval = interval;
  timer->active = true;
  timer->triggered = false;
}

void stopTimer(Timer* timer) {
  timer->active = false;
  timer->triggered = false;
}

bool checkTimer(Timer* timer) {
  if (!timer->active) return false;

  unsigned long currentMillis = millis();
  if (currentMillis - timer->previousMillis >= timer->interval) {
    timer->triggered = true;
    timer->active = false;
    return true;
  }
  return false;
}

bool isTimerActive(Timer* timer) {
  return timer->active;
}

bool wasTimerTriggered(Timer* timer) {
  bool result = timer->triggered;
  timer->triggered = false; // Reset after reading
  return result;
}

/***********************************************************
 *  PERFORMANCE MONITORING FUNCTIONS                      *
 ***********************************************************/
// void startPerformanceMonitoring() {
//   loopStartTime = micros();
// }

// void endPerformanceMonitoring() {
//   loopExecutionTime = micros() - loopStartTime;

//   // Update statistics
//   if (loopExecutionTime > maxLoopTime) {
//     maxLoopTime = loopExecutionTime;
//   }
//   if (loopExecutionTime < minLoopTime) {
//     minLoopTime = loopExecutionTime;
//   }

//   totalLoops++;

//   // Update memory statistics
//   freeHeapSize = ESP.getFreeHeap();
//   if (freeHeapSize < minFreeHeap) {
//     minFreeHeap = freeHeapSize;
//   }

//   // Print performance stats periodically
//   unsigned long currentMillis = millis();
//   if (currentMillis - lastPerformanceUpdate >= performanceUpdateInterval) {
//     printPerformanceStats();
//     lastPerformanceUpdate = currentMillis;
//   }
// }

// void printPerformanceStats() {
//   if (totalLoops == 0) return;

//   // Serial.println() - removed for production
//   // Serial.print() - removed for production
//   // Serial.print() - removed for production
//   // Serial.print() - removed for production
//   // Serial.print() - removed for production
//   // Serial.print() - removed for production
//   // Serial.println() - removed for production

//   // Serial.print() - removed for production
//   // Serial.print() - removed for production
//   // Serial.print() - removed for production
//   // Serial.print() - removed for production
//   // Serial.println() - removed for production

//   // Serial.print() - removed for production
//   // Serial.println() - removed for production

//   // Calculate average loop time
//   // Serial.print() - removed for production
//   // Serial.print() - removed for production
//   // Serial.println() - removed for production

//   // Serial.println() - removed for production
// }

void resetPerformanceStats() {
  maxLoopTime = 0;
  minLoopTime = 999999;
  totalLoops = 0;
  minFreeHeap = ESP.getFreeHeap();
}

/***********************************************************
 *  OPTIMIZED STATE MANAGEMENT                            *
 ***********************************************************/
// Replace String operations with char arrays for better performance
char statusJalanOptimized[16] = "BERHENTI";
char currentModeOptimized[16] = "WAREHOUSE";

void setStatusJalan(const char* status) {
  strncpy(statusJalanOptimized, status, sizeof(statusJalanOptimized) - 1);
  statusJalanOptimized[sizeof(statusJalanOptimized) - 1] = '\0';
}

void setCurrentMode(const char* mode) {
  strncpy(currentModeOptimized, mode, sizeof(currentModeOptimized) - 1);
  currentModeOptimized[sizeof(currentModeOptimized) - 1] = '\0';
}

const char* getStatusJalan() {
  return statusJalanOptimized;
}

const char* getCurrentMode() {
  return currentModeOptimized;
}

/***********************************************************
 *  ERROR RECOVERY SYSTEM                                 *
 ***********************************************************/
// Variables moved to config.h: systemInErrorState, errorRecoveryAttempts
const int maxErrorRecoveryAttempts = 3;

// Global AGV state tracking variables - moved to config.h as extern declarations
// AgvState currentStateAgv, terminalDropRfidId, terminalPickUpRfidId, ujungRfidId, exceptErrorPosition



void initErrorRecovery() {
  systemInErrorState = false;
  errorRecoveryAttempts = 0;
  // Serial.println() - removed for production
}

// Fungsi untuk mereset flag except error position
void resetExceptErrorFlag() {
  exceptErrorPosition = false;
  // Serial.println() - removed for production
}

// Fungsi untuk menyimpan flag except error position ke preferences
void saveExceptErrorFlag() {
  preferences.begin("except-error", false);
  preferences.putBool("position", exceptErrorPosition);
  preferences.end();
  return;
}

// Fungsi untuk memuat flag except error position dari preferences
void loadExceptErrorFlag() {
  preferences.begin("except-error", true);
  exceptErrorPosition = preferences.getBool("position", false);
  preferences.end();
  // Serial.println() - removed for production
}

bool attemptErrorRecovery(int errorCode) {
  if (errorRecoveryAttempts >= maxErrorRecoveryAttempts) {
    return false; // Give up after max attempts
  }

  errorRecoveryAttempts++;
  systemInErrorState = true;

  // Serial.print() - removed for production
  // Serial.print() - removed for production
  // Serial.print() - removed for production
  // Serial.println() - removed for production

  // Start recovery timer
  startTimer(&errorRecoveryTimer, 5000);

  // Attempt recovery based on error type
  switch (errorCode) {
    case 1: // ERROR_SENSOR_COMMUNICATION
      // Reinitialize sensor communication
      return recoverSensorCommunication();

    case 2: // ERROR_MOTOR_CONTROL
      // Reset motor controllers
      return recoverMotorControl();

    case 3: // ERROR_RFID_COMMUNICATION
      // Reinitialize RFID (function commented out)
      return false; // recoverRfidCommunication();

    case 4: // ERROR_WIFI_CONNECTION
      // Attempt WiFi reconnection
      return recoverWifiConnection();

    default:
      return false;
  }
}

bool recoverSensorCommunication() {
  // Reinitialize ModbusMaster untuk sensor magnet (Serial1) dengan current slave ID
  magnetNode.begin(getCurrentMagnetSlaveId(), Serial1);
  delay(50); // Small delay for initialization

  // Reinitialize ModbusMaster untuk sensor ultrasonik (Serial2) dengan current slave ID
  ultrasonicNode.begin(getCurrentUltrasonicSlaveId(), Serial2);
  delay(50); // Small delay for initialization

  // Test communication untuk sensor magnet
  uint8_t resultMagnet = magnetNode.readHoldingRegisters(0x0000, 1);
  bool magnetOk = (resultMagnet == magnetNode.ku8MBSuccess);

  // Test communication untuk sensor ultrasonik
  uint8_t resultUltrasonic = ultrasonicNode.readHoldingRegisters(0x0000, 1);
  bool ultrasonicOk = (resultUltrasonic == ultrasonicNode.ku8MBSuccess);

  return (magnetOk && ultrasonicOk);
}

bool recoverMotorControl() {
  // Stop all motors
  rpmMotor(0, 0);  // Use RPM stop command
  delay(100);

  // Reinitialize motor pins if needed
  // This would depend on your motor setup
  return true;
}

// bool recoverRfidCommunication() {
//   // Reinitialize RFID serial communication
//   Serial2.end();
//   delay(100);
//   Serial2.begin(9600);
//   delay(100);
//   return true;
// }

bool recoverWifiConnection() {
  // Attempt WiFi reconnection
  WiFi.disconnect();
  delay(1000);
  WiFi.begin(ssid, password);

  // Wait for connection with timeout
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    attempts++;
  }

  return (WiFi.status() == WL_CONNECTED);
}

void checkErrorRecovery() {
  if (systemInErrorState && checkTimer(&errorRecoveryTimer)) {
    systemInErrorState = false;
    // Serial.println() - removed for production
  }
}

/***********************************************************
 *  INITIALIZATION                                        *
 ***********************************************************/
void initPerformanceOptimization() {
  // Serial.println() - removed for production

  // Initialize timers
  stopTimer(&stopPelanPelanTimer);
  stopTimer(&ultrasonicSwitchTimer);
  stopTimer(&buttonDebounceTimer);
  stopTimer(&menuDelayTimer);
  stopTimer(&errorRecoveryTimer);

  // Initialize performance monitoring
  resetPerformanceStats();

  // Initialize error recovery
  initErrorRecovery();

  // Set initial optimized states
  setStatusJalan("BERHENTI");
  setCurrentMode("WAREHOUSE");

  // Serial.println() - removed for production
}

/***********************************************************
 *  MAIN LOOP INTEGRATION                                 *
 ***********************************************************/
// void updatePerformanceOptimization() {
//   // Check all active timers
//   checkTimer(&stopPelanPelanTimer);
//   checkTimer(&ultrasonicSwitchTimer);
//   checkTimer(&buttonDebounceTimer);
//   checkTimer(&menuDelayTimer);
//   checkTimer(&errorRecoveryTimer);

//   // Check error recovery
//   checkErrorRecovery();
// }