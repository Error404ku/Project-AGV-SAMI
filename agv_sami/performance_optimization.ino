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

/***********************************************************
 *  PERFORMANCE MONITORING VARIABLES                      *
 ***********************************************************/
unsigned long loopStartTime = 0;
unsigned long loopExecutionTime = 0;
unsigned long maxLoopTime = 0;
unsigned long minLoopTime = 999999;
unsigned long totalLoops = 0;
unsigned long performanceUpdateInterval = 5000; // 5 seconds
unsigned long lastPerformanceUpdate = 0;

// Memory tracking
size_t freeHeapSize = 0;
size_t minFreeHeap = 999999;

/***********************************************************
 *  NON-BLOCKING TIMER SYSTEM                             *
 ***********************************************************/
struct Timer {
  unsigned long previousMillis;
  unsigned long interval;
  bool active;
  bool triggered;
};

// Timer instances for different operations
Timer stopPelanPelanTimer = {0, 500, false, false};
Timer ultrasonicSwitchTimer = {0, 100, false, false};
Timer magnetSwitchTimer = {0, 100, false, false};
Timer buttonDebounceTimer = {0, 300, false, false};
Timer menuDelayTimer = {0, 1500, false, false};
Timer errorRecoveryTimer = {0, 5000, false, false};
Timer performanceTimer = {0, 5000, false, false};

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
void startPerformanceMonitoring() {
  loopStartTime = micros();
}

void endPerformanceMonitoring() {
  loopExecutionTime = micros() - loopStartTime;

  // Update statistics
  if (loopExecutionTime > maxLoopTime) {
    maxLoopTime = loopExecutionTime;
  }
  if (loopExecutionTime < minLoopTime) {
    minLoopTime = loopExecutionTime;
  }

  totalLoops++;

  // Update memory statistics
  freeHeapSize = ESP.getFreeHeap();
  if (freeHeapSize < minFreeHeap) {
    minFreeHeap = freeHeapSize;
  }

  // Print performance stats periodically
  unsigned long currentMillis = millis();
  if (currentMillis - lastPerformanceUpdate >= performanceUpdateInterval) {
    printPerformanceStats();
    lastPerformanceUpdate = currentMillis;
  }
}

void printPerformanceStats() {
  if (totalLoops == 0) return;

  Serial.println("\n=== PERFORMANCE STATS ===");
  Serial.print("Loop Time (us) - Current: ");
  Serial.print(loopExecutionTime);
  Serial.print(", Max: ");
  Serial.print(maxLoopTime);
  Serial.print(", Min: ");
  Serial.println(minLoopTime);

  Serial.print("Memory - Free: ");
  Serial.print(freeHeapSize);
  Serial.print(" bytes, Min Free: ");
  Serial.print(minFreeHeap);
  Serial.println(" bytes");

  Serial.print("Total Loops: ");
  Serial.println(totalLoops);

  // Calculate average loop time
  Serial.print("Loop Frequency: ");
  Serial.print(1000000.0 / loopExecutionTime);
  Serial.println(" Hz");

  Serial.println("========================\n");
}

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
char lastScannedRfidOptimized[32] = ""; // Optimized RFID storage

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
bool systemInErrorState = false;
int errorRecoveryAttempts = 0;
const int maxErrorRecoveryAttempts = 3;

// Global AGV state tracking variables
StateMode currentStateMode = STATE_MODE_BERHENTI;
AgvState lastStateAgv = AGV_STATE_MOVE_FORWARD;
AgvState currentStateAgv = AGV_STATE_MOVE_FORWARD;

// Terminal RFID variables
String terminalDropRfidId = "";
String terminalPickUpRfidId = "";

void initErrorRecovery() {
  systemInErrorState = false;
  errorRecoveryAttempts = 0;
}

bool attemptErrorRecovery(int errorCode) {
  if (errorRecoveryAttempts >= maxErrorRecoveryAttempts) {
    return false; // Give up after max attempts
  }

  errorRecoveryAttempts++;
  systemInErrorState = true;

  Serial.print("Attempting error recovery #");
  Serial.print(errorRecoveryAttempts);
  Serial.print(" for error code: ");
  Serial.println(errorCode);

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
      // Reinitialize RFID
      return recoverRfidCommunication();

    case 4: // ERROR_WIFI_CONNECTION
      // Attempt WiFi reconnection
      return recoverWifiConnection();

    default:
      return false;
  }
}

bool recoverSensorCommunication() {
  // Reinitialize ModbusMaster
  node.begin(1, Serial1);
  delay(100); // Small delay for initialization

  // Test communication
  uint8_t result = node.readHoldingRegisters(0x0000, 1);
  return (result == node.ku8MBSuccess);
}

bool recoverMotorControl() {
  // Stop all motors
  pwmMotor(0, 0);
  delay(100);

  // Reinitialize motor pins if needed
  // This would depend on your motor setup
  return true;
}

bool recoverRfidCommunication() {
  // Reinitialize RFID serial communication
  Serial2.end();
  delay(100);
  Serial2.begin(9600);
  delay(100);
  return true;
}

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
    Serial.println("Error recovery timeout - system resumed");
  }
}

/***********************************************************
 *  INITIALIZATION                                        *
 ***********************************************************/
void initPerformanceOptimization() {
  Serial.println("Initializing Performance Optimization System...");

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

  Serial.println("Performance Optimization System initialized successfully!");
}

/***********************************************************
 *  MAIN LOOP INTEGRATION                                 *
 ***********************************************************/
void updatePerformanceOptimization() {
  // Check all active timers
  checkTimer(&stopPelanPelanTimer);
  checkTimer(&ultrasonicSwitchTimer);
  checkTimer(&buttonDebounceTimer);
  checkTimer(&menuDelayTimer);
  checkTimer(&errorRecoveryTimer);

  // Check error recovery
  checkErrorRecovery();
}