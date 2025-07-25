/*
  SETUP.INO - System Initialization Functions
  
  This file contains all initialization functions for the AGV SAMI system.
  Functions are organized by subsystem for better maintainability.
*/

// ==================== MAIN SETUP FUNCTION ====================

void setupAll() {
  DEBUG_PRINTLN("Starting system initialization...");
  
  // Initialize in order of dependency
  setupHardware();      // Basic hardware setup
  loadSystemConfig();   // Load saved configuration
  setupCommunication(); // Communication systems
  setupSensors();       // Sensor systems
  setupInterface();     // User interface
  
  // Mark system as initialized
  systemState.isInitialized = true;
  
  DEBUG_PRINTLN("System initialization complete");
}

// ==================== HARDWARE SETUP ====================

void setupHardware() {
  DEBUG_PRINTLN("Initializing hardware...");
  
  setupMotor();
  setupHook();
  setupMusicAndLed();
  setupButtons();
  
  DEBUG_PRINTLN("Hardware initialization complete");
}

void setupMotor() {
  DEBUG_PRINTLN("Setting up motor control...");
  
  // Configure motor control pins
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);
  
  // Setup PWM channels for motor speed control
  ledcSetup(PWM_CHANNEL_ENA, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_ENB, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(MOTOR_ENA, PWM_CHANNEL_ENA);
  ledcAttachPin(MOTOR_ENB, PWM_CHANNEL_ENB);
  
  // Ensure motors are stopped at startup
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
  ledcWrite(PWM_CHANNEL_ENA, 0);
  ledcWrite(PWM_CHANNEL_ENB, 0);
  
  DEBUG_PRINTLN("Motor control setup complete");
}

void setupHook() {
  DEBUG_PRINTLN("Setting up hook motor...");
  
  // Configure hook control pins
  pinMode(HOOK_RELAY_PIN, OUTPUT);
  pinMode(LIMIT_SWITCH_UP_PIN, INPUT_PULLDOWN);
  pinMode(LIMIT_SWITCH_DOWN_PIN, INPUT_PULLDOWN);
  
  // Ensure hook is stopped at startup
  digitalWrite(HOOK_RELAY_PIN, LOW);
  
  // Initialize hook state
  systemState.currentHookState = HOOK_IDLE;
  
  DEBUG_PRINTLN("Hook motor setup complete");
}

void setupMusicAndLed() {
  DEBUG_PRINTLN("Setting up music/LED pins...");
  
  // Configure music/LED pins as outputs
  pinMode(MUSIC_PIN_0, OUTPUT);
  pinMode(MUSIC_PIN_1, OUTPUT);
  pinMode(MUSIC_PIN_2, OUTPUT);
  pinMode(MUSIC_PIN_3, OUTPUT);
  
  // Set all pins HIGH (assuming active low LEDs)
  digitalWrite(MUSIC_PIN_0, HIGH);
  digitalWrite(MUSIC_PIN_1, HIGH);
  digitalWrite(MUSIC_PIN_2, HIGH);
  digitalWrite(MUSIC_PIN_3, HIGH);
  
  DEBUG_PRINTLN("Music/LED setup complete");
}

void setupButtons() {
  DEBUG_PRINTLN("Setting up button inputs...");
  
  // Configure button pins with pull-down resistors
  pinMode(BUTTON_UP_PIN, INPUT_PULLDOWN);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLDOWN);
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLDOWN);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLDOWN);
  pinMode(BUTTON_START_PIN, INPUT_PULLDOWN);
  pinMode(BUTTON_STOP_PIN, INPUT_PULLDOWN);
  
  DEBUG_PRINTLN("Button setup complete");
}

// ==================== COMMUNICATION SETUP ====================

void setupCommunication() {
  DEBUG_PRINTLN("Initializing communication systems...");
  
  setupUnifiedRS485();
  setupWebServer();
  setupRfid();
  
  DEBUG_PRINTLN("Communication setup complete");
}

void setupUnifiedRS485() {
  DEBUG_PRINTLN("Setting up unified RS485 system...");
  
  // Setup RS485 control pins
  pinMode(MAX485_DE, OUTPUT);
  pinMode(MAX485_RE, OUTPUT);
  digitalWrite(MAX485_RE, LOW);
  digitalWrite(MAX485_DE, LOW);
  
  // Initialize serial communication
  Serial1.begin(BAUDRATE_RS485, SERIAL_8N1, RX_RS485, TX_RS485);
  
  // Initialize Modbus masters
  nodeMagnetFront.begin(ADDR_MAGNET_FRONT, Serial1);
  nodeMagnetFront.preTransmission(preTransmission);
  nodeMagnetFront.postTransmission(postTransmission);
  
  nodeMagnetBack.begin(ADDR_MAGNET_BACK, Serial1);
  nodeMagnetBack.preTransmission(preTransmission);
  nodeMagnetBack.postTransmission(postTransmission);
  
  // Initialize device status
  for (int i = 0; i < MAX_DEVICES; i++) {
    deviceStatus[i] = DeviceStatus();
  }
  
  DEBUG_PRINTLN("Unified RS485 setup complete");
}

void setupWebServer() {
  DEBUG_PRINTLN("Setting up web server...");
  
  // Load stations list from preferences
  loadStationsListFromPreferences();
  
  // Configure WiFi as Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(WIFI_STATIC_IP, WIFI_GATEWAY, WIFI_SUBNET);
  
  bool apStarted = WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  
  if (apStarted) {
    DEBUG_PRINTLN("WiFi AP started successfully");
    DEBUG_PRINT("AP IP address: ");
    DEBUG_PRINTLN(WiFi.softAPIP());
  } else {
    logError(ERROR_WIFI_CONNECTION, "Failed to start WiFi AP");
    return;
  }
  
  // Setup HTTP endpoints
  server.on("/", HTTP_GET, []() {
    String html = "<html><body>";
    html += "<h1>AGV SAMI Control Panel</h1>";
    html += "<p>System Status: " + String(systemState.isInitialized ? "Ready" : "Initializing") + "</p>";
    html += "<p>Current Mode: " + String(systemState.isAgvMode ? "AGV" : "Menu") + "</p>";
    html += "<h2>API Endpoints:</h2>";
    html += "<ul>";
    html += "<li><a href='/showstations'>GET /showstations</a> - Show current stations</li>";
    html += "<li>POST /updatestations - Update stations list</li>";
    html += "</ul>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });
  
  server.on("/updatestations", HTTP_POST, handleUpdateStations);
  server.on("/showstations", HTTP_GET, handleShowStations);
  
  // Handle 404 errors
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not Found");
  });
  
  // Start server
  server.begin();
  
  DEBUG_PRINTLN("Web server setup complete");
}

void setupRfid() {
  DEBUG_PRINTLN("Setting up RFID reader...");
  
  // Install interrupt handlers for Wiegand protocol
  wiegand.onReceive(receivedData, "Card read: ");
  wiegand.onReceiveError(receivedDataError, "Card read error: ");
  wiegand.onStateChange(stateChanged, "State changed: ");
  
  // Initialize Wiegand pins
  wiegand.begin(Wiegand::LENGTH_ANY, true);
  
  // Setup pin change interrupts
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_D1, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_D0), pinStateChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_D1), pinStateChanged, CHANGE);
  
  // Load RFID stations from preferences
  loadRfidStations();
  
  DEBUG_PRINTLN("RFID reader setup complete");
}

// ==================== SENSOR SETUP ====================

void setupSensors() {
  DEBUG_PRINTLN("Initializing sensor systems...");
  
  // Initialize sensor data structure
  sensorData = SensorData();
  
  // Sensor communication is handled by unified RS485 system
  // No additional setup required here
  
  DEBUG_PRINTLN("Sensor setup complete");
}

// ==================== INTERFACE SETUP ====================

void setupInterface() {
  DEBUG_PRINTLN("Setting up user interface...");
  
  setupDisplay();
  setupMenu();
  
  DEBUG_PRINTLN("Interface setup complete");
}

void setupDisplay() {
  DEBUG_PRINTLN("Setting up LCD display...");
  
  // Initialize I2C communication
  Wire.begin();
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Display startup message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AGV SAMI v2.0");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  lcd.setCursor(0, 2);
  lcd.print("Please wait");
  
  delay(2000);  // Show message for 2 seconds
  
  DEBUG_PRINTLN("LCD display setup complete");
}

void setupMenu() {
  DEBUG_PRINTLN("Setting up menu system...");
  
  // Initialize menu state
  systemState.currentMenu = MENU_MAIN;
  selectedItem = 0;
  maxItems = 13;
  menuStartIndex = 0;
  menuNeedsRefresh = true;
  
  // Load RFID stations for menu
  loadRfidStations();
  
  DEBUG_PRINTLN("Menu system setup complete");
}

// ==================== RS485 CALLBACK FUNCTIONS ====================

void preTransmission() {
  digitalWrite(MAX485_RE, HIGH);
  digitalWrite(MAX485_DE, HIGH);
}

void postTransmission() {
  digitalWrite(MAX485_RE, LOW);
  digitalWrite(MAX485_DE, LOW);
}