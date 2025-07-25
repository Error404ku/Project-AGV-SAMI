/*
  INTERFACE.INO - User Interface Management
  
  This file combines all interface-related functions:
  - LCD display management
  - Menu system
  - Web server endpoints
  - Button handling
  - Status display
*/

// ==================== LCD DISPLAY FUNCTIONS ====================

void initLCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Display startup message
  lcd.setCursor(0, 0);
  lcd.print("AGV SAMI v2.0");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  DEBUG_PRINTLN("LCD initialized");
}

void updateLCDDisplay() {
  static unsigned long lastUpdate = 0;
  
  // Update LCD every 500ms to avoid flickering
  if (millis() - lastUpdate < LCD_UPDATE_INTERVAL) {
    return;
  }
  lastUpdate = millis();
  
  lcd.clear();
  
  switch (currentMenuState) {
    case MENU_MAIN:
      displayMainMenu();
      break;
    case MENU_AGV_MODE:
      displayAGVMode();
      break;
    case MENU_STATION_SETUP:
      displayStationSetup();
      break;
    case MENU_RFID_SETUP:
      displayRFIDSetup();
      break;
    case MENU_SENSOR_STATUS:
      displaySensorStatus();
      break;
    case MENU_SYSTEM_INFO:
      displaySystemInfo();
      break;
    case MENU_SETTINGS:
      displaySettings();
      break;
    default:
      displayMainMenu();
      break;
  }
}

void displayMainMenu() {
  lcd.setCursor(0, 0);
  lcd.print("MENU UTAMA");
  lcd.setCursor(0, 1);
  
  switch (menuIndex) {
    case 0:
      lcd.print(">Mode AGV");
      break;
    case 1:
      lcd.print(">Setup Stasiun");
      break;
    case 2:
      lcd.print(">Setup RFID");
      break;
    case 3:
      lcd.print(">Status Sensor");
      break;
    case 4:
      lcd.print(">Info Sistem");
      break;
    case 5:
      lcd.print(">Pengaturan");
      break;
    default:
      lcd.print(">Mode AGV");
      menuIndex = 0;
      break;
  }
}

void displayAGVMode() {
  lcd.setCursor(0, 0);
  lcd.print("MODE AGV");
  lcd.setCursor(0, 1);
  
  switch (systemState.currentMode) {
    case MODE_MANUAL:
      lcd.print("Manual");
      break;
    case MODE_STATION:
      lcd.print("Stasiun: ");
      lcd.print(currentStationId);
      break;
    case MODE_LINE_FOLLOW:
      lcd.print("Line Follow");
      break;
    case MODE_IDLE:
      lcd.print("Idle");
      break;
    default:
      lcd.print("Unknown");
      break;
  }
  
  // Show movement status
  lcd.setCursor(12, 1);
  switch (systemState.currentMovement) {
    case MOVEMENT_FORWARD:
      lcd.print("FWD");
      break;
    case MOVEMENT_BACKWARD:
      lcd.print("BWD");
      break;
    case MOVEMENT_LEFT:
      lcd.print("LFT");
      break;
    case MOVEMENT_RIGHT:
      lcd.print("RGT");
      break;
    case MOVEMENT_STOP:
      lcd.print("STP");
      break;
  }
}

void displayStationSetup() {
  lcd.setCursor(0, 0);
  lcd.print("SETUP STASIUN");
  lcd.setCursor(0, 1);
  lcd.print("Total: ");
  lcd.print(jumlahStasiun);
  lcd.setCursor(10, 1);
  lcd.print("ID: ");
  lcd.print(currentStationId);
}

void displayRFIDSetup() {
  lcd.setCursor(0, 0);
  lcd.print("SETUP RFID");
  lcd.setCursor(0, 1);
  
  if (lastScannedRfid.length() > 0) {
    lcd.print(lastScannedRfid.substring(0, 16));
  } else {
    lcd.print("Scan kartu RFID");
  }
}

void displaySensorStatus() {
  lcd.setCursor(0, 0);
  lcd.print("STATUS SENSOR");
  lcd.setCursor(0, 1);
  
  // Display magnet sensor status
  lcd.print("M:");
  lcd.print(totalSensorAktif);
  lcd.print("/16");
  
  // Display ultrasonic status
  lcd.setCursor(8, 1);
  lcd.print("U:");
  if (sensorData.obstacleDetected) {
    lcd.print("OBS");
  } else {
    lcd.print("OK");
  }
  
  // Display communication status
  lcd.setCursor(13, 1);
  if (deviceStatus[0].isOnline) {
    lcd.print("ON");
  } else {
    lcd.print("OFF");
  }
}

void displaySystemInfo() {
  lcd.setCursor(0, 0);
  lcd.print("INFO SISTEM");
  lcd.setCursor(0, 1);
  
  // Display WiFi status
  lcd.print("WiFi: ");
  if (WiFi.getMode() == WIFI_AP) {
    lcd.print("AP");
  } else {
    lcd.print("OFF");
  }
  
  // Display hook status
  lcd.setCursor(10, 1);
  lcd.print("H:");
  switch (systemState.hookStatus) {
    case HOOK_UP:
      lcd.print("UP");
      break;
    case HOOK_DOWN:
      lcd.print("DN");
      break;
    case HOOK_MOVING:
      lcd.print("MV");
      break;
    default:
      lcd.print("??");
      break;
  }
}

void displaySettings() {
  lcd.setCursor(0, 0);
  lcd.print("PENGATURAN");
  lcd.setCursor(0, 1);
  
  switch (settingsIndex) {
    case 0:
      lcd.print(">Reset System");
      break;
    case 1:
      lcd.print(">Kalibrasi PID");
      break;
    case 2:
      lcd.print(">Test Motor");
      break;
    case 3:
      lcd.print(">Test Sensor");
      break;
    default:
      lcd.print(">Reset System");
      settingsIndex = 0;
      break;
  }
}

void displayMessage(String line1, String line2, int duration_ms) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  
  if (duration_ms > 0) {
    delay(duration_ms);
    updateLCDDisplay();
  }
}

// ==================== MENU SYSTEM ====================

void handleMenuNavigation() {
  static unsigned long lastButtonPress = 0;
  unsigned long currentTime = millis();
  
  // Debounce button presses
  if (currentTime - lastButtonPress < BUTTON_DEBOUNCE_DELAY) {
    return;
  }
  
  // Read button states
  bool upPressed = digitalRead(BUTTON_UP) == HIGH;
  bool downPressed = digitalRead(BUTTON_DOWN) == HIGH;
  bool selectPressed = digitalRead(BUTTON_SELECT) == HIGH;
  bool backPressed = digitalRead(BUTTON_BACK) == HIGH;
  
  if (upPressed || downPressed || selectPressed || backPressed) {
    lastButtonPress = currentTime;
    
    if (upPressed) {
      handleUpButton();
    } else if (downPressed) {
      handleDownButton();
    } else if (selectPressed) {
      handleSelectButton();
    } else if (backPressed) {
      handleBackButton();
    }
    
    updateLCDDisplay();
  }
}

void handleUpButton() {
  switch (currentMenuState) {
    case MENU_MAIN:
      menuIndex = (menuIndex > 0) ? menuIndex - 1 : MAIN_MENU_ITEMS - 1;
      break;
    case MENU_SETTINGS:
      settingsIndex = (settingsIndex > 0) ? settingsIndex - 1 : SETTINGS_MENU_ITEMS - 1;
      break;
    case MENU_STATION_SETUP:
      if (currentStationId > 1) {
        currentStationId--;
      }
      break;
    default:
      break;
  }
}

void handleDownButton() {
  switch (currentMenuState) {
    case MENU_MAIN:
      menuIndex = (menuIndex < MAIN_MENU_ITEMS - 1) ? menuIndex + 1 : 0;
      break;
    case MENU_SETTINGS:
      settingsIndex = (settingsIndex < SETTINGS_MENU_ITEMS - 1) ? settingsIndex + 1 : 0;
      break;
    case MENU_STATION_SETUP:
      if (currentStationId < MAX_STATIONS) {
        currentStationId++;
      }
      break;
    default:
      break;
  }
}

void handleSelectButton() {
  switch (currentMenuState) {
    case MENU_MAIN:
      handleMainMenuSelect();
      break;
    case MENU_AGV_MODE:
      toggleAGVMode();
      break;
    case MENU_STATION_SETUP:
      handleStationSetup();
      break;
    case MENU_RFID_SETUP:
      handleRFIDSetup();
      break;
    case MENU_SETTINGS:
      handleSettingsSelect();
      break;
    default:
      break;
  }
}

void handleBackButton() {
  switch (currentMenuState) {
    case MENU_MAIN:
      // Already at main menu, do nothing
      break;
    default:
      currentMenuState = MENU_MAIN;
      menuIndex = 0;
      break;
  }
}

void handleMainMenuSelect() {
  switch (menuIndex) {
    case 0:
      currentMenuState = MENU_AGV_MODE;
      break;
    case 1:
      currentMenuState = MENU_STATION_SETUP;
      break;
    case 2:
      currentMenuState = MENU_RFID_SETUP;
      break;
    case 3:
      currentMenuState = MENU_SENSOR_STATUS;
      break;
    case 4:
      currentMenuState = MENU_SYSTEM_INFO;
      break;
    case 5:
      currentMenuState = MENU_SETTINGS;
      settingsIndex = 0;
      break;
  }
}

void handleSettingsSelect() {
  switch (settingsIndex) {
    case 0:
      // Reset system
      displayMessage("Resetting...", "Please wait", 1000);
      resetToDefaults();
      ESP.restart();
      break;
    case 1:
      // PID calibration
      displayMessage("PID Calibration", "Not implemented", 2000);
      break;
    case 2:
      // Test motors
      displayMessage("Testing Motors", "Please wait", 1000);
      testAllMotors();
      displayMessage("Motor Test", "Complete", 2000);
      break;
    case 3:
      // Test sensors
      displayMessage("Testing Sensors", "Check serial", 2000);
      testAllSensors();
      break;
  }
}

void toggleAGVMode() {
  switch (systemState.currentMode) {
    case MODE_IDLE:
      systemState.currentMode = MODE_MANUAL;
      displayMessage("Mode Changed", "Manual Mode", 1000);
      break;
    case MODE_MANUAL:
      systemState.currentMode = MODE_LINE_FOLLOW;
      displayMessage("Mode Changed", "Line Follow", 1000);
      break;
    case MODE_LINE_FOLLOW:
      systemState.currentMode = MODE_STATION;
      displayMessage("Mode Changed", "Station Mode", 1000);
      break;
    case MODE_STATION:
      systemState.currentMode = MODE_IDLE;
      stopMovement();
      displayMessage("Mode Changed", "Idle Mode", 1000);
      break;
  }
}

void handleStationSetup() {
  // Add current station to the list
  if (jumlahStasiun < MAX_STATIONS) {
    jumlahStasiun++;
    displayMessage("Station Added", "ID: " + String(currentStationId), 1000);
  } else {
    displayMessage("Error", "Max stations", 2000);
  }
}

void handleRFIDSetup() {
  if (lastScannedRfid.length() > 0) {
    // Associate RFID with current station
    addRfidStation(currentStationId, lastScannedRfid);
    displayMessage("RFID Saved", "Station " + String(currentStationId), 1000);
    lastScannedRfid = "";
  } else {
    displayMessage("Error", "No RFID scanned", 2000);
  }
}

// ==================== WEB SERVER FUNCTIONS ====================

void setupWebServer() {
  // Set up WiFi Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  
  DEBUG_PRINT("WiFi AP started. IP: ");
  DEBUG_PRINTLN(WiFi.softAPIP());
  
  // Define web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/control", HTTP_POST, handleControl);
  server.on("/updatestations", HTTP_POST, handleUpdateStations);
  server.on("/showstations", HTTP_GET, handleShowStations);
  server.on("/config", HTTP_GET, handleConfig);
  server.on("/config", HTTP_POST, handleConfigUpdate);
  
  server.begin();
  DEBUG_PRINTLN("Web server started");
}

void handleRoot() {
  String html = "<!DOCTYPE html>";
  html += "<html><head><title>AGV SAMI Control</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }";
  html += ".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += ".status { background: #e8f5e8; padding: 15px; border-radius: 5px; margin: 10px 0; }";
  html += ".controls { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 10px; margin: 20px 0; }";
  html += "button { padding: 15px; font-size: 16px; border: none; border-radius: 5px; cursor: pointer; transition: background 0.3s; }";
  html += ".btn-primary { background: #007bff; color: white; }";
  html += ".btn-primary:hover { background: #0056b3; }";
  html += ".btn-danger { background: #dc3545; color: white; }";
  html += ".btn-danger:hover { background: #c82333; }";
  html += ".btn-success { background: #28a745; color: white; }";
  html += ".btn-success:hover { background: #1e7e34; }";
  html += "</style></head><body>";
  
  html += "<div class='container'>";
  html += "<h1>AGV SAMI Control Panel</h1>";
  
  // Status section
  html += "<div class='status'>";
  html += "<h3>Status Sistem</h3>";
  html += "<p><strong>Mode:</strong> " + getModeString() + "</p>";
  html += "<p><strong>Movement:</strong> " + getMovementString() + "</p>";
  html += "<p><strong>Hook:</strong> " + getHookString() + "</p>";
  html += "<p><strong>Sensors Active:</strong> " + String(totalSensorAktif) + "/16</p>";
  html += "<p><strong>Obstacle:</strong> " + String(sensorData.obstacleDetected ? "Detected" : "Clear") + "</p>";
  html += "</div>";
  
  // Control buttons
  html += "<div class='controls'>";
  html += "<button class='btn-primary' onclick='sendCommand(\"forward\")'>\u2191 Forward</button>";
  html += "<button class='btn-primary' onclick='sendCommand(\"backward\")'>\u2193 Backward</button>";
  html += "<button class='btn-primary' onclick='sendCommand(\"left\")'>\u2190 Left</button>";
  html += "<button class='btn-primary' onclick='sendCommand(\"right\")'>\u2192 Right</button>";
  html += "<button class='btn-danger' onclick='sendCommand(\"stop\")'>⏹ Stop</button>";
  html += "<button class='btn-success' onclick='sendCommand(\"hook_toggle\")'>🪝 Toggle Hook</button>";
  html += "</div>";
  
  // Mode controls
  html += "<div class='controls'>";
  html += "<button class='btn-primary' onclick='sendCommand(\"mode_manual\")'>\uD83D\uDD27 Manual</button>";
  html += "<button class='btn-primary' onclick='sendCommand(\"mode_line\")'>\uD83D\uDEE4 Line Follow</button>";
  html += "<button class='btn-primary' onclick='sendCommand(\"mode_station\")'>\uD83C\uDFE2 Station</button>";
  html += "<button class='btn-primary' onclick='sendCommand(\"mode_idle\")'>\u23F8 Idle</button>";
  html += "</div>";
  
  // JavaScript for AJAX commands
  html += "<script>";
  html += "function sendCommand(cmd) {";
  html += "  fetch('/control', { method: 'POST', headers: {'Content-Type': 'application/x-www-form-urlencoded'}, body: 'command=' + cmd })";
  html += "  .then(response => response.text())";
  html += "  .then(data => { console.log(data); setTimeout(() => location.reload(), 500); });";
  html += "}";
  html += "setInterval(() => location.reload(), 5000);";
  html += "</script>";
  
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

void handleStatus() {
  String json = "{";
  json += "\"mode\":\"" + getModeString() + "\",";
  json += "\"movement\":\"" + getMovementString() + "\",";
  json += "\"hook\":\"" + getHookString() + "\",";
  json += "\"sensors_active\":" + String(totalSensorAktif) + ",";
  json += "\"obstacle\":" + String(sensorData.obstacleDetected ? "true" : "false") + ",";
  json += "\"station_id\":" + String(currentStationId) + ",";
  json += "\"total_stations\":" + String(jumlahStasiun);
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleControl() {
  if (server.hasArg("command")) {
    String command = server.arg("command");
    
    if (command == "forward") {
      moveForward(systemConfig.baseSpeed);
    } else if (command == "backward") {
      moveBackward(systemConfig.baseSpeed);
    } else if (command == "left") {
      turnLeft(systemConfig.baseSpeed);
    } else if (command == "right") {
      turnRight(systemConfig.baseSpeed);
    } else if (command == "stop") {
      stopMovement();
    } else if (command == "hook_toggle") {
      toggleHook();
    } else if (command == "mode_manual") {
      systemState.currentMode = MODE_MANUAL;
    } else if (command == "mode_line") {
      systemState.currentMode = MODE_LINE_FOLLOW;
    } else if (command == "mode_station") {
      systemState.currentMode = MODE_STATION;
    } else if (command == "mode_idle") {
      systemState.currentMode = MODE_IDLE;
      stopMovement();
    }
    
    server.send(200, "text/plain", "Command executed: " + command);
  } else {
    server.send(400, "text/plain", "No command specified");
  }
}

void handleUpdateStations() {
  // Handle station updates from web interface
  if (server.hasArg("stations")) {
    String stationsData = server.arg("stations");
    // Parse and update stations (implementation depends on format)
    server.send(200, "text/plain", "Stations updated");
  } else {
    server.send(400, "text/plain", "No station data provided");
  }
}

void handleShowStations() {
  String html = "<!DOCTYPE html><html><head><title>AGV Stations</title></head><body>";
  html += "<h1>RFID Stations</h1>";
  html += "<table border='1'><tr><th>Station ID</th><th>RFID</th></tr>";
  
  for (int i = 0; i < MAX_RFID_STATIONS; i++) {
    if (rfidStations[i].stationId != 0) {
      html += "<tr><td>" + String(rfidStations[i].stationId) + "</td>";
      html += "<td>" + rfidStations[i].rfidCode + "</td></tr>";
    }
  }
  
  html += "</table></body></html>";
  server.send(200, "text/html", html);
}

void handleConfig() {
  String json = "{";
  json += "\"base_speed\":" + String(systemConfig.baseSpeed) + ",";
  json += "\"max_pwm\":" + String(systemConfig.maxPwm) + ",";
  json += "\"kp\":" + String(systemConfig.pidLinefollower.kp) + ",";
  json += "\"ki\":" + String(systemConfig.pidLinefollower.ki) + ",";
  json += "\"kd\":" + String(systemConfig.pidLinefollower.kd);
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleConfigUpdate() {
  bool updated = false;
  
  if (server.hasArg("base_speed")) {
    systemConfig.baseSpeed = server.arg("base_speed").toInt();
    updated = true;
  }
  
  if (server.hasArg("max_pwm")) {
    systemConfig.maxPwm = server.arg("max_pwm").toInt();
    updated = true;
  }
  
  if (server.hasArg("kp")) {
    systemConfig.pidLinefollower.kp = server.arg("kp").toFloat();
    updated = true;
  }
  
  if (server.hasArg("ki")) {
    systemConfig.pidLinefollower.ki = server.arg("ki").toFloat();
    updated = true;
  }
  
  if (server.hasArg("kd")) {
    systemConfig.pidLinefollower.kd = server.arg("kd").toFloat();
    updated = true;
  }
  
  if (updated) {
    saveSystemConfig();
    server.send(200, "text/plain", "Configuration updated");
  } else {
    server.send(400, "text/plain", "No valid parameters provided");
  }
}

// ==================== UTILITY FUNCTIONS ====================

String getModeString() {
  switch (systemState.currentMode) {
    case MODE_MANUAL: return "Manual";
    case MODE_STATION: return "Station";
    case MODE_LINE_FOLLOW: return "Line Follow";
    case MODE_IDLE: return "Idle";
    default: return "Unknown";
  }
}

String getMovementString() {
  switch (systemState.currentMovement) {
    case MOVEMENT_FORWARD: return "Forward";
    case MOVEMENT_BACKWARD: return "Backward";
    case MOVEMENT_LEFT: return "Left";
    case MOVEMENT_RIGHT: return "Right";
    case MOVEMENT_STOP: return "Stop";
    default: return "Unknown";
  }
}

String getHookString() {
  switch (systemState.hookStatus) {
    case HOOK_UP: return "Up";
    case HOOK_DOWN: return "Down";
    case HOOK_MOVING: return "Moving";
    default: return "Unknown";
  }
}

void testAllSensors() {
  DEBUG_PRINTLN("=== SENSOR TEST ===");
  DEBUG_PRINTF("Magnet sensors active: %d/16\n", totalSensorAktif);
  DEBUG_PRINTF("Obstacle detected: %s\n", sensorData.obstacleDetected ? "YES" : "NO");
  DEBUG_PRINTF("Front ultrasonic distances: F1=%d, F2=%d, F3=%d\n", 
               sensorData.ultrasonicFront[0], sensorData.ultrasonicFront[1], sensorData.ultrasonicFront[2]);
  DEBUG_PRINTF("Back ultrasonic distances: B1=%d, B2=%d, B3=%d\n", 
               sensorData.ultrasonicBack[0], sensorData.ultrasonicBack[1], sensorData.ultrasonicBack[2]);
  
  for (int i = 0; i < 4; i++) {
    DEBUG_PRINTF("Device %d: %s (Last seen: %lu ms ago)\n", 
                 i, deviceStatus[i].isOnline ? "ONLINE" : "OFFLINE", 
                 millis() - deviceStatus[i].lastSeen);
  }
  DEBUG_PRINTLN("=== END SENSOR TEST ===");
}