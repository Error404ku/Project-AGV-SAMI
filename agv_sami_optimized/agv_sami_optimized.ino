/*
  AGV SAMI - Autonomous Guided Vehicle System
  
  Optimized version with improved code structure and organization.
  
  Features:
  - Unified RS485 communication system
  - PID-based line following
  - Web-based station management
  - RFID station detection
  - Hook motor control with safety
  - LCD menu interface
  - Obstacle detection and avoidance
  
  Hardware:
  - ESP32-S3 DevKit-C v1
  - L298N Motor Driver
  - RS485 Sensors (Magnet & Ultrasonic)
  - LCD 20x4 I2C Display
  - Wiegand RFID Reader
  - SSR Relay for Hook Motor
  
  Author: AGV SAMI Team
  Version: 2.0 (Optimized)
*/

#include "config.h"
#include "types.h"

// ==================== GLOBAL OBJECT DEFINITIONS ====================

// Hardware Objects
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
WebServer server(80);
Preferences preferences;
ModbusMaster nodeMagnetFront;
ModbusMaster nodeMagnetBack;
Wiegand wiegand;

// ==================== GLOBAL VARIABLE DEFINITIONS ====================

// System State
SystemState systemState;
SystemConfig systemConfig;
SensorData sensorData;

// Communication Variables
bool inPacket = false;
int byteCounter = 0;
byte dataPacket[PACKET_LENGTH];
int currentDeviceAddress = 0;
DeviceStatus deviceStatus[MAX_DEVICES];

// Station Management
StationsList stationsList;
RfidStation rfidStations[MAX_RFID_STATIONS];
int rfidStationCount = 0;

// Menu Variables
int selectedItem = 0;
int maxItems = 13;
int menuStartIndex = 0;
bool menuNeedsRefresh = true;

// RFID Variables
String lastScannedRfid = "";
bool newRfidScanned = false;

// Music Variables
bool statusMusic = false;

// Movement Variables
bool modeMaju = false;
bool modeMundur = false;
bool modeBerhenti = true;
bool force = false;

// Sensor State Variables
bool tengahAktif = false;
bool kananHilang = false;
bool kiriHilang = false;
bool sensorkebacasemua = false;

// Station Variables
int station = 0;
bool sudahDeteksiStasiun = false;
int indexTarget = 0;
int totalSensorAktif = 0;
int currentStationId = 1;
int jumlahStasiun = 0;

// Timing Variables
unsigned long previousMillis = 0;
unsigned long lastDeviceSwitch = 0;
unsigned long lastButtonPress = 0;
unsigned long lastObstacleCheck = 0;
unsigned long lastRfidScanTime = 0;

// PID Variables
int lastError = 0;

// ==================== SETUP FUNCTION ====================

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    delay(10);
  }
  
  Serial.println("\n=== AGV SAMI v2.0 Starting ===");
  Serial.println("Optimized version with improved structure");
  
  // Initialize all subsystems
  setupAll();
  
  Serial.println("=== AGV SAMI Ready ===");
  Serial.println("System initialized successfully");
  Serial.println("Press START button to enter AGV mode");
  Serial.println("Use menu buttons for configuration\n");
}

// ==================== MAIN LOOP ====================

void loop() {
  // Handle web server clients
  server.handleClient();
  
  // Main system loops
  loopCommunication();  // RS485, WiFi, RFID communication
  loopSensors();        // Sensor data processing
  loopInterface();      // Menu, display, buttons
  loopControl();        // Motor control and navigation
  loopSafety();         // Safety checks and error handling
  
  // Small delay to prevent watchdog timeout
  delay(1);
}

// ==================== MAIN LOOP FUNCTIONS ====================

void loopCommunication() {
  // Handle unified RS485 communication
  loopUnifiedRS485();
  
  // Handle RFID scanning
  loopRfid();
  
  // Process any pending communication tasks
  // This could include WiFi reconnection, data logging, etc.
}

void loopSensors() {
  unsigned long currentMillis = millis();
  
  // Read sensors at specified interval
  if (currentMillis - previousMillis >= SENSOR_READ_INTERVAL) {
    previousMillis = currentMillis;
    
    // Update sensor data
    bacaSensorGaris();
    
    // Check for obstacles
    if (currentMillis - lastObstacleCheck >= OBSTACLE_CHECK_INTERVAL) {
      lastObstacleCheck = currentMillis;
      checkObstacles();
    }
  }
}

void loopInterface() {
  if (systemState.isAgvMode) {
    // AGV mode - show sensor data and status
    displayPrint();
    
    // Check for exit condition (STOP button)
    if (buttonStop()) {
      systemState.isAgvMode = false;
      systemState.currentMenu = MENU_MAIN;
      pwmMotor(0, 0);  // Stop motors
      menuNeedsRefresh = true;
      DEBUG_PRINTLN("Exited AGV mode");
    }
  } else {
    // Menu mode - handle menu navigation
    handleMenu();
  }
}

void loopControl() {
  if (systemState.isAgvMode) {
    // Execute AGV logic
    logicAgv();
  }
  
  // Update hook status regardless of mode
  updateHookStatus();
}

void loopSafety() {
  // Check device communication timeouts
  checkDeviceTimeouts();
  
  // Monitor system health
  // This could include memory usage, temperature, etc.
  
  // Handle any critical errors
  if (sensorData.obstacleDetected && (modeMaju || modeMundur)) {
    // Emergency stop if obstacle detected while moving
    pwmMotor(0, 0);
    music("error");
  }
}

// ==================== UTILITY FUNCTIONS ====================

void logError(ErrorCode code, String message) {
  // Log error with timestamp
  unsigned long timestamp = millis();
  
  DEBUG_PRINTF("[ERROR %lu] Code %d: %s\n", timestamp, code, message.c_str());
  
  // Store error in preferences for later analysis
  preferences.begin(PREF_NAMESPACE_ERRORS, false);
  
  // Get current error count
  int errorCount = preferences.getInt("errorCount", 0);
  
  // Store error (limit to last 10 errors)
  String errorKey = "error" + String(errorCount % 10);
  String errorData = String(timestamp) + "|" + String(code) + "|" + message;
  preferences.putString(errorKey.c_str(), errorData);
  
  // Update error count
  preferences.putInt("errorCount", errorCount + 1);
  
  preferences.end();
}

void saveSystemConfig() {
  preferences.begin(PREF_NAMESPACE_SETTINGS, false);
  
  // Save PID parameters
  preferences.putDouble("kpLinefollower", systemConfig.pidLinefollower.kp);
  preferences.putDouble("kiLinefollower", systemConfig.pidLinefollower.ki);
  preferences.putDouble("kdLinefollower", systemConfig.pidLinefollower.kd);
  
  // Save motor settings
  preferences.putInt("baseSpeed", systemConfig.baseSpeed);
  preferences.putBool("invertY", systemConfig.invertMotorY);
  preferences.putBool("invertX", systemConfig.invertMotorX);
  preferences.putBool("invertKanan", systemConfig.invertMotorKanan);
  preferences.putBool("invertKiri", systemConfig.invertMotorKiri);
  preferences.putBool("invertHook", systemConfig.invertHook);
  
  // Save music mapping
  preferences.putInt("musicStation", systemConfig.musicStationPin);
  preferences.putInt("musicError", systemConfig.musicErrorPin);
  preferences.putInt("musicDetect", systemConfig.musicDetectPin);
  preferences.putInt("musicKomputer", systemConfig.musicKomputerPin);
  
  preferences.end();
  
  DEBUG_PRINTLN("System configuration saved");
}

void loadSystemConfig() {
  preferences.begin(PREF_NAMESPACE_SETTINGS, false);
  
  // Load PID parameters
  systemConfig.pidLinefollower.kp = preferences.getDouble("kpLinefollower", DEFAULT_KP_LINEFOLLOWER);
  systemConfig.pidLinefollower.ki = preferences.getDouble("kiLinefollower", DEFAULT_KI_LINEFOLLOWER);
  systemConfig.pidLinefollower.kd = preferences.getDouble("kdLinefollower", DEFAULT_KD_LINEFOLLOWER);
  
  // Load motor settings
  systemConfig.baseSpeed = preferences.getInt("baseSpeed", DEFAULT_BASE_SPEED);
  systemConfig.invertMotorY = preferences.getBool("invertY", false);
  systemConfig.invertMotorX = preferences.getBool("invertX", false);
  systemConfig.invertMotorKanan = preferences.getBool("invertKanan", false);
  systemConfig.invertMotorKiri = preferences.getBool("invertKiri", false);
  systemConfig.invertHook = preferences.getBool("invertHook", false);
  
  // Load music mapping
  systemConfig.musicStationPin = preferences.getInt("musicStation", 0);
  systemConfig.musicErrorPin = preferences.getInt("musicError", 1);
  systemConfig.musicDetectPin = preferences.getInt("musicDetect", 2);
  systemConfig.musicKomputerPin = preferences.getInt("musicKomputer", 3);
  
  preferences.end();
  
  DEBUG_PRINTLN("System configuration loaded");
}

void resetToDefaults() {
  // Reset system configuration to defaults
  systemConfig = SystemConfig();
  
  // Clear all preferences
  preferences.begin(PREF_NAMESPACE_SETTINGS, false);
  preferences.clear();
  preferences.end();
  
  preferences.begin(PREF_NAMESPACE_STATIONS, false);
  preferences.clear();
  preferences.end();
  
  preferences.begin(PREF_NAMESPACE_RFID, false);
  preferences.clear();
  preferences.end();
  
  // Clear station lists
  stationsList.clear();
  rfidStationCount = 0;
  
  DEBUG_PRINTLN("System reset to defaults");
}