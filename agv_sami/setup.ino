void setupMotor() {
  // Inisialisasi Serial0 untuk komunikasi dengan ESP32 kedua
  Serial.begin(921600);
  
  // Small delay untuk stabilisasi serial
  delay(100);
  
  // Flush serial buffer
  while (Serial.available()) {
    Serial.read();
  }

  // Request PID data dari motor controller saat startup
  requestPidDataFromSlave();

  // Wait for PID data from motor controller before proceeding
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for PID");
  lcd.setCursor(0, 1);
  lcd.print("Data from Slave");
  
  // Loop dengan timeout protection
  unsigned long startWait = millis();
  const unsigned long maxWaitTime = 5000; // 5 detik timeout
  
  while (!checkSystemReadyStatus()) {
    // Handle incoming serial data while waiting
    handleMotorControllerSerial();
    
    // Check timeout
    if (millis() - startWait > maxWaitTime) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("PID Timeout!");
      lcd.setCursor(0, 1);
      lcd.print("Using defaults");
      delay(1500);
      
      // Force system ready dengan default values
      systemReadyToRun = true;
      break;
    }
    
    delay(50); // Small delay to prevent tight loop
  }
  
  // Test komunikasi - use RPM command for stopping
  sendRPM(0, 0);  // Stop semua motor saat startup dengan RPM command
  delay(50);

}

void setupMusic() {
  pinMode(pinMusic1, OUTPUT);
  pinMode(pinMusic2, OUTPUT);
  pinMode(pinMusic3, OUTPUT);
  pinMode(pinMusic4, OUTPUT);
  pinMode(pinMusic5, OUTPUT);
  pinMode(pinMusic6, OUTPUT);
  // Set all music pins to HIGH (relay OFF)
  digitalWrite(pinMusic1, HIGH);
  digitalWrite(pinMusic2, HIGH);
  digitalWrite(pinMusic3, HIGH);
  digitalWrite(pinMusic4, HIGH);
  digitalWrite(pinMusic5, HIGH);
  digitalWrite(pinMusic6, HIGH);
}

void setupHook() {
  pinMode(pinHook1, INPUT_PULLDOWN);
  pinMode(pinHook2, INPUT_PULLDOWN);
  pinMode(pinMotorHook, OUTPUT);
  digitalWrite(pinMotorHook, HIGH);
}

void setuplamp() {
  pinMode(lampPin, OUTPUT);
  digitalWrite(lampPin, HIGH);
}


// setup display


void setupDisplay() {
  // Initialize I2C SDA 3, SCL 8
  Wire.begin(sdaPin, sclPin);

  delay(100);

  Wire.beginTransmission(LCD_ADDRESS);

  // Initialize LCD with error handling
  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  
  // Clear LCD and display startup message
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("AGV System Ready   ");
}

void setupWebServer() {
  lcd.setCursor(0, 0);
  lcd.print("SETUP WIFI");
 
  // Non-blocking WiFi setup - just initialize, don't wait for connection
  lcd.setCursor(0, 1);
  lcd.print("WiFi Initialized");
  delay(500); // Brief delay for display
  
  // Registrasi Endpoint HTTP yang diminta
  server.on("/updatestations", HTTP_POST, handleUpdateTargetStations);  // Untuk menyimpan/menimpa daftar stasiun
  server.on("/showstations", HTTP_GET, handleShowTargetStations);       // Untuk menampilkan daftar stasiun
  server.on("/showallrfid", HTTP_GET, handleShowAllRfid);               // Untuk menampilkan SEMUA RFID yang terdaftar
  
  // WiFi Configuration endpoints
  server.on("/", HTTP_GET, handleRoot);                          // Halaman utama dengan menu
  server.on("/wifi", HTTP_GET, handleWifiConfig);                // Halaman konfigurasi WiFi
  server.on("/wifi-config", HTTP_GET, handleWifiConfig);         // Halaman konfigurasi WiFi (alias)
  server.on("/savewifi", HTTP_POST, handleSaveWifi);             // Simpan konfigurasi WiFi
  server.on("/save-wifi", HTTP_POST, handleSaveWifi);            // Simpan konfigurasi WiFi (alias)

  // Handler untuk endpoint tidak ditemukan
  server.onNotFound([]() {
    server.send(404, "text/plain", "Endpoint tidak ditemukan.");
  });

  server.begin();  // Memulai server HTTP
  // Serial debug removed for production
  delay(500);
}

void setupUltrasonikWithParams(int slaveId) {
  // Initialize ultrasonic sensor with ModbusMaster (Serial2)
  Serial.print("[SETUP] setupUltrasonikWithParams called with slaveId: ");
  Serial.println(slaveId);
  
  // Check if Serial2 is available
  if (!Serial2) {
    Serial.println("[ERROR] Serial2 not available for ultrasonic sensor");
    return;
  }

  Serial.println("[SETUP] Initializing ultrasonic node...");
  ultrasonicNode.begin(slaveId, Serial2);
  ultrasonicNode.preTransmission(preTransmissionUltrasonic);
  ultrasonicNode.postTransmission(postTransmissionUltrasonic);

  Serial.println("[SETUP] Setting ultrasonic slave ID...");
  setUltrasonicSlaveId(slaveId);

  // Test communication
  Serial.println("[SETUP] Testing ultrasonic communication...");
  uint8_t testResult = ultrasonicNode.readHoldingRegisters(0x0000, 5);
  if (testResult == ultrasonicNode.ku8MBSuccess) {
    Serial.println("[OK] Ultrasonic sensor communication successful");
  } else {
    Serial.print("[WARNING] Ultrasonic sensor communication failed, error code: ");
    Serial.println(testResult);
  }
}

void setupRS485(int baudrate) {

  // Initialize Serial1 for RS485 communication (Magnet sensors)
  Serial1.begin(baudrate, SERIAL_8N1, RS485_RX, RS485_TX);

  // Setup control pins for MAX485
  pinMode(MAX485_RE, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);

  // Set to receive mode (RE=0, DE=0)
  digitalWrite(MAX485_RE, 0);
  digitalWrite(MAX485_DE, 0);

  // Wait for Serial1 to be ready
  delay(100);
}

void setupRS485_Serial2(int baudrate) {
  Serial.print("[SETUP] Initializing RS485 Serial2 with baudrate: ");
  Serial.println(baudrate);

  // Initialize Serial2 for RS485 communication (Ultrasonic sensors)
  Serial2.begin(baudrate, SERIAL_8N1, RS485_RX2, RS485_TX2);

  // Setup control pins for MAX485 Serial2 (menggunakan pin terpisah dari Serial1)
  pinMode(MAX485_RE, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);

  // Set to receive mode (RE=0, DE=0)
  digitalWrite(MAX485_RE, 0);
  digitalWrite(MAX485_DE, 0);

  // Wait for Serial2 to be ready
  delay(100);

  if (Serial2) {
    Serial.println("[OK] Serial2 initialized successfully");
  } else {
    Serial.println("[ERROR] Serial2 initialization failed!");
  }

  Serial.println("[SETUP] RS485 Serial2 setup complete");
}

void setupSensorMagnet(int slaveId) {
  Serial.print("[SETUP] setupSensorMagnet called with slaveId: ");
  Serial.println(slaveId);
  
  // Check if Serial1 is available
  if (!Serial1) {
    Serial.println("[ERROR] Serial1 not available for magnet sensor");
    return;
  }

  Serial.println("[SETUP] Initializing magnet node...");
  magnetNode.begin(slaveId, Serial1);
  magnetNode.preTransmission(preTransmissionMagnet);
  magnetNode.postTransmission(postTransmissionMagnet);

  Serial.println("[SETUP] Setting magnet slave ID...");
  setMagnetSlaveId(slaveId);

  // Test communication
  Serial.println("[SETUP] Testing magnet sensor communication...");
  uint8_t testResult = magnetNode.readHoldingRegisters(0x0000, 2);
  if (testResult == magnetNode.ku8MBSuccess) {
    Serial.println("[OK] Magnet sensor communication successful");
  } else {
    Serial.print("[WARNING] Magnet sensor communication failed, error code: ");
    Serial.println(testResult);
  }

  Serial.println("[SETUP] Magnet sensor setup complete");
}

void setupSensorUltrasonic(int slaveId) {
  Serial.print("[SETUP] setupSensorUltrasonic called with slaveId: ");
  Serial.println(slaveId);

  // REMOVED INFINITE RECURSION BUG!
  // The recursive call to setupUltrasonikWithParams was causing stack overflow
  // Now setupUltrasonikWithParams handles the actual initialization
  
  Serial.println("[SETUP] Ultrasonic sensor setup delegated to setupUltrasonikWithParams");
}


void setupRfid() {

  //Install listeners and initialize Wiegand reader
  wiegand.onReceive(receivedData, "Card readed: ");
  wiegand.onReceiveError(receivedDataError, "Card read error: ");
  wiegand.onStateChange(stateChanged, "State changed: ");
  wiegand.begin(Wiegand::LENGTH_ANY, true);

  //initialize pins as INPUT and attaches interruptions
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_D1, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_D0), pinStateChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_D1), pinStateChanged, CHANGE);

  //Sends the initial pin state to the Wiegand library
  pinStateChanged();
}

void setupMenu() {
  preferences.begin("agv-settings", true);

  Serial.println("\n=== LOADING SETTINGS ===");
  
  // Load PID settings with proper default values
  kp = preferences.getDouble("kpLinefollower", 70.0);
  ki = preferences.getDouble("kiLinefollower", 0.0);
  kd = preferences.getDouble("kdLinefollower", 0.0);
  

  
  // Load Forward PID WithMassa settings with proper default values
  kpForwardWithMassa = preferences.getDouble("kpFwdMassa", 70.0);
  kiForwardWithMassa = preferences.getDouble("kiFwdMassa", 0.0);
  kdForwardWithMassa = preferences.getDouble("kdFwdMassa", 0.0);
  
  // Load Forward PID Default settings with proper default values
  kpForwardDefault = preferences.getDouble("kpFwdDefault", 70.0);
  Serial.print("Loaded kpFwdDefault: ");
  Serial.println(kpForwardDefault, 2);
  
  kiForwardDefault = preferences.getDouble("kiFwdDefault", 0.0);
  Serial.print("Loaded kiFwdDefault: ");
  Serial.println(kiForwardDefault, 2);
  
  kdForwardDefault = preferences.getDouble("kdFwdDefault", 0.0);
  Serial.print("Loaded kdFwdDefault: ");
  Serial.println(kdForwardDefault, 2);
  
  // Load Backward PID WithMassa settings with proper default values
  kpBackwardWithMassa = preferences.getDouble("kpBwdMassa", 70.0);
  kiBackwardWithMassa = preferences.getDouble("kiBwdMassa", 0.0);
  kdBackwardWithMassa = preferences.getDouble("kdBwdMassa", 0.0);
  
  // Load Backward PID Default settings with proper default values
  kpBackwardDefault = preferences.getDouble("kpBwdDefault", 70.0);
  kiBackwardDefault = preferences.getDouble("kiBwdDefault", 0.0);
  kdBackwardDefault = preferences.getDouble("kdBwdDefault", 0.0);

  // Load Motor settings
  baseSpeed = preferences.getInt("baseSpeed", 1000);

  // Load Motor invert settings
  invertMotorY = preferences.getBool("invertY", false);
  invertMotorX = preferences.getBool("invertX", false);
  invertMotorKanan = preferences.getBool("invertKanan", false);
  invertMotorKiri = preferences.getBool("invertKiri", false);
  invertHook = preferences.getBool("invertHook", false);

  // Load Music mapping settings
  musicOnPin = preferences.getInt("musicOn", 0);
  musicObstaclePin = preferences.getInt("musicObstacle", 1);
  musicStationPin = preferences.getInt("musicStation", 2);
  musicOutOfLinePin = preferences.getInt("musicOutOfLine", 3);
  musicWarningPin = preferences.getInt("musicWarning", 4);
  
  // Load Ultrasonic settings
  minSafeDistanceFront = preferences.getUShort("SafeDistFront", 30);
  minSafeDistanceBack = preferences.getUShort("SafeDistBack", 20);
  minSafeDistanceFrontSerong = preferences.getUShort("SafeDistFrontS", 25);
  minSafeDistanceBackSerong = preferences.getUShort("SafeDistBackS", 15);

  // Load Motor Control settings
  maxMotorRpm = preferences.getInt("maxMotorRpm", 90);
  // PID parameters are managed by motor controller, set defaults
  motorPidKp = 1.0;   // Default value, not loaded from preferences
  motorPidKi = 0.15;  // Default value, not loaded from preferences  
  motorPidKd = 0.0;   // Default value, not loaded from preferences
  
  // Initialize individual motor PID values to same defaults
  motorPidKpRight = motorPidKp;
  motorPidKiRight = motorPidKi; 
  motorPidKdRight = motorPidKd;
  motorPidKpLeft = motorPidKp;
  motorPidKiLeft = motorPidKi;
  motorPidKdLeft = motorPidKd;

  preferences.end();
  
  Serial.println("=== SETTINGS LOADED ===\n");

}

void setupTombol() {
  pinMode(upPin, INPUT_PULLDOWN);     // UP - active HIGH
  pinMode(downPin, INPUT_PULLDOWN);   // DOWN - active HIGH
  pinMode(leftPin, INPUT_PULLDOWN);   // LEFT - active HIGH
  pinMode(rightPin, INPUT_PULLDOWN);  // RIGHT - active HIGH
  pinMode(startPin, INPUT_PULLDOWN);  // START - active HIGH
  pinMode(stopPin, INPUT_PULLDOWN);   // STOP - active HIGH
}

void setupPreferences(){
  loadAllAGVStatesFromPreferences();
  loadTargetStationsListFromPreferences();
    // Load RFID stations
  loadRfidStations();
  
  // Load RFID Ujung and Warehouse data
  loadRfidUjungFromPreferences();
  loadRfidWarehouseFromPreferences();
  loadWarehouseUjungRfid();
  
  // Load Terminal RFID data
  loadTerminalRfid();
  
  // Load Ujung Slow Mode
  loadUjungSlowMode();
  
  // Load except error position flag
  loadExceptErrorFlag();

  // Load stations list from HTTP preferences
  loadTargetStationsListFromPreferences();
}
