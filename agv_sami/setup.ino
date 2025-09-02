void setupMotor() {
  // Setup komunikasi serial dengan ESP32 motor controller
  setupMotorSerial(); // Panggil fungsi setup motor serial yang benar
   
  // Test komunikasi - use RPM command for stopping
  sendRPM(0, 0); // Stop semua motor saat startup dengan RPM command
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
void initializeDisplay() {
  // I2C address scanner disabled for production
  byte count = 0;
  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      count++;
      if (i == LCD_ADDRESS) {
        // LCD found at expected address
      }
    }
  }
  
  if (count == 0) {
    // No I2C devices found - check wiring
  }

  // Initialize LCD with error handling
  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  
  // Test LCD communication
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("LCD Test OK        ");
  lcd.setCursor(0, 1);
  lcd.print("AGV System Ready   ");
  
  // Verify LCD is responding
  delay(500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Display Ready      ");
  // LCD initialization completed
}

void setupDisplay() {
  // Initialize I2C SDA 3, SCL 8
  Wire.begin(sdaPin, sclPin);
  delay(100);
  // Initialize display
  initializeDisplay();
}

void setupWebServer() {
  // Muat daftar stasiun dari Preferences saat startup
  loadTargetStationsListFromPreferences();

  lcd.setCursor(0, 0);
  lcd.print("SETUP WIFI");
 
  // Non-blocking WiFi setup - just initialize, don't wait for connection
  lcd.setCursor(0, 1);
  lcd.print("WiFi Initialized");
  delay(500); // Brief delay for display
  
  // Registrasi Endpoint HTTP yang diminta
  server.on("/updatestations", HTTP_POST, handleUpdateTargetStations);  // Untuk menyimpan/menimpa daftar stasiun
  server.on("/showstations", HTTP_GET, handleShowTargetStations);       // Untuk menampilkan daftar stasiun
  server.on("/showstationaddresses", HTTP_GET, handleShowStationAddresses); // Untuk menampilkan alamat station RFID
  server.on("/showujungstations", HTTP_GET, handleShowUjungStations);   // Untuk menampilkan data ujung station RFID
  server.on("/showwarehouserfid", HTTP_GET, handleShowWarehouseRfid);   // Untuk menampilkan data warehouse RFID
  server.on("/showterminalrfid", HTTP_GET, handleShowTerminalRfid);     // Untuk menampilkan data terminal RFID
  
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
  setupSensorUltrasonic(slaveId);
}

void setupRS485(int baudrate) {
  // Serial debug removed for production
  // Serial debug removed for production

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

  if (Serial1) {
    // Serial debug removed for production
  } else {
    // Serial debug removed for production
  }

  // Serial debug removed for production
}

void setupRS485_Serial2(int baudrate) {
  // Serial debug removed for production

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
    // Serial debug removed for production
  } else {
    // Serial debug removed for production
  }

  // Serial debug removed for production
}

void setupSensorMagnet(int slaveId) {
  // Serial debug removed for production

  // Check if Serial1 is available
  if (!Serial1) {
    // Serial debug removed for production
    return;
  }

  // Serial debug removed for production
  magnetNode.begin(slaveId, Serial1);
  magnetNode.preTransmission(preTransmissionMagnet);
  magnetNode.postTransmission(postTransmissionMagnet);

  // Serial debug removed for production
  setMagnetSlaveId(slaveId);

  // Test communication
  // Serial debug removed for production
  uint8_t testResult = magnetNode.readHoldingRegisters(0x0000, 2);
  if (testResult == magnetNode.ku8MBSuccess) {
    // Serial debug removed for production
  } else {
    // Serial debug removed for production
  }

  // Serial debug removed for production
}

void setupSensorUltrasonic(int slaveId) {
  // Serial debug removed for production

  // Check if Serial2 is available
  if (!Serial2) {
    // Serial debug removed for production
    return;
  }

  // Serial debug removed for production
  ultrasonicNode.begin(slaveId, Serial2);// Slave ID untuk sensor ultrasonik (Serial2)
  ultrasonicNode.preTransmission(preTransmissionUltrasonic);
  ultrasonicNode.postTransmission(postTransmissionUltrasonic);

  // Serial debug removed for production
  setUltrasonicSlaveId(slaveId);

  // Test communication
  // Serial debug removed for production
  uint8_t testResult = ultrasonicNode.readHoldingRegisters(0x0000, 5);
  if (testResult == ultrasonicNode.ku8MBSuccess) {
    // Serial debug removed for production
  } else {
    // Serial debug removed for production
  }

  // Serial debug removed for production
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

  // Load PID settings with proper default values
  tempKp = preferences.getDouble("kpLinefollower", 70.0);
  tempKi = preferences.getDouble("kiLinefollower", 0.0);
  tempKd = preferences.getDouble("kdLinefollower", 0.0);
  

  
  // Load Forward PID WithMassa settings with proper default values
  tempKpForwardWithMassa = preferences.getDouble("kpFwdMassa", 70.0);
  tempKiForwardWithMassa = preferences.getDouble("kiFwdMassa", 0.0);
  tempKdForwardWithMassa = preferences.getDouble("kdFwdMassa", 0.0);
  
  // Load Forward PID Default settings with proper default values
  tempKpForwardDefault = preferences.getDouble("kpFwdDefault", 70.0);
  tempKiForwardDefault = preferences.getDouble("kiFwdDefault", 0.0);
  tempKdForwardDefault = preferences.getDouble("kdFwdDefault", 0.0);
  
  // Load Backward PID WithMassa settings with proper default values
  tempKpBackwardWithMassa = preferences.getDouble("kpBwdMassa", 70.0);
  tempKiBackwardWithMassa = preferences.getDouble("kiBwdMassa", 0.0);
  tempKdBackwardWithMassa = preferences.getDouble("kdBwdMassa", 0.0);
  
  // Load Backward PID Default settings with proper default values
  tempKpBackwardDefault = preferences.getDouble("kpBwdDefault", 70.0);
  tempKiBackwardDefault = preferences.getDouble("kiBwdDefault", 0.0);
  tempKdBackwardDefault = preferences.getDouble("kdBwdDefault", 0.0);

  // Load Motor settings
  tempBaseSpeed = preferences.getInt("baseSpeed", 1000);

  // Load Motor invert settings
  tempInvertY = preferences.getBool("invertY", false);
  tempInvertX = preferences.getBool("invertX", false);
  tempInvertKanan = preferences.getBool("invertKanan", false);
  tempInvertKiri = preferences.getBool("invertKiri", false);
  tempInvertHook = preferences.getBool("invertHook", false);

  // Load Music mapping settings
  tempMusicOnPin = preferences.getInt("musicOn", 0);
  tempMusicObstaclePin = preferences.getInt("musicObstacle", 1);
  tempMusicStationPin = preferences.getInt("musicStation", 2);
  tempMusicOutOfLinePin = preferences.getInt("musicOutOfLine", 3);
  tempMusicWarningPin = preferences.getInt("musicWarning", 4);
  
  // Load Ultrasonic settings
  tempMinSafeDistanceFront = preferences.getUShort("SafeDistFront", 30);
  tempMinSafeDistanceBack = preferences.getUShort("SafeDistBack", 20);
  tempMinSafeDistanceFrontSerong = preferences.getUShort("SafeDistFrontS", 25);
  tempMinSafeDistanceBackSerong = preferences.getUShort("SafeDistBackS", 15);

  // Load Motor Control settings
  tempMaxMotorRpm = preferences.getInt("maxMotorRpm", 90);
  // PID parameters are managed by motor controller, set defaults
  tempMotorPidKp = 1.0;   // Default value, not loaded from preferences
  tempMotorPidKi = 0.15;  // Default value, not loaded from preferences  
  tempMotorPidKd = 0.0;   // Default value, not loaded from preferences

  // Apply PID values
  kpLinefollower = tempKp;
  kiLinefollower = tempKi;
  kdLinefollower = tempKd;
  

  
  // Apply Forward PID WithMassa values
  kpLinefollowerForwardWithMassa = tempKpForwardWithMassa;
  kiLinefollowerForwardWithMassa = tempKiForwardWithMassa;
  kdLinefollowerForwardWithMassa = tempKdForwardWithMassa;
  
  // Apply Forward PID Default values
  kpLinefollowerForwardDefault = tempKpForwardDefault;
  kiLinefollowerForwardDefault = tempKiForwardDefault;
  kdLinefollowerForwardDefault = tempKdForwardDefault;
  
  // Apply Backward PID WithMassa values
  kpLinefollowerBackwardWithMassa = tempKpBackwardWithMassa;
  kiLinefollowerBackwardWithMassa = tempKiBackwardWithMassa;
  kdLinefollowerBackwardWithMassa = tempKdBackwardWithMassa;
  
  // Apply Backward PID Default values
  kpLinefollowerBackwardDefault = tempKpBackwardDefault;
  kiLinefollowerBackwardDefault = tempKiBackwardDefault;
  kdLinefollowerBackwardDefault = tempKdBackwardDefault;
  
  // Apply Motor values
  baseSpeed = tempBaseSpeed;

  // Apply Motor invert values
  invertMotorY = tempInvertY;
  invertMotorX = tempInvertX;
  invertMotorKanan = tempInvertKanan;
  invertMotorKiri = tempInvertKiri;
  invertHook = tempInvertHook;

  // Apply Music mapping values
  musicOnPin = tempMusicOnPin;
  musicObstaclePin = tempMusicObstaclePin;
  musicStationPin = tempMusicStationPin;
  musicOutOfLinePin = tempMusicOutOfLinePin;
  musicWarningPin = tempMusicWarningPin;
  
  // Apply Ultrasonic settings
  minSafeDistanceFront = tempMinSafeDistanceFront;
  minSafeDistanceBack = tempMinSafeDistanceBack;
  minSafeDistanceFrontSerong = tempMinSafeDistanceFrontSerong;
  minSafeDistanceBackSerong = tempMinSafeDistanceBackSerong;

  // Apply Motor Control settings
  maxMotorRpm = tempMaxMotorRpm;
  // PID parameters are managed by motor controller
  motorPidKp = tempMotorPidKp;     // Default values only
  motorPidKi = tempMotorPidKi;     // Default values only
  motorPidKd = tempMotorPidKd;     // Default values only

  preferences.end();

  // Load RFID stations
  loadRfidStations();
  
  // Load RFID Ujung and Warehouse data
  loadRfidUjungFromPreferences();
  loadRfidWarehouseFromPreferences();
  loadWarehouseUjungRfid();
  
  // Load Terminal RFID data
  loadTerminalRfid();
  

  
  // Load except error position flag
  loadExceptErrorFlag();

  // Load stations list from HTTP preferences
  loadTargetStationsListFromPreferences();

  // Serial debug removed for production
  // Serial debug removed for production
}

void setupTombol() {
  pinMode(upPin, INPUT_PULLDOWN);     // UP - active HIGH
  pinMode(downPin, INPUT_PULLDOWN);   // DOWN - active HIGH
  pinMode(leftPin, INPUT_PULLDOWN);   // LEFT - active HIGH
  pinMode(rightPin, INPUT_PULLDOWN);  // RIGHT - active HIGH
  pinMode(startPin, INPUT_PULLDOWN);  // START - active HIGH
  pinMode(stopPin, INPUT_PULLDOWN);   // STOP - active HIGH
}

void setupAll() {
  setupMotor();
  setuplamp();
  setupMusic();
  setupDisplay();
  setupMenu();  // Initialize menu system
  // Setup RS485 communication for both Serial1 and Serial2
  setupRS485(BAUDRATE);        // Serial1 untuk sensor magnet
  setupRS485_Serial2(BAUDRATE); // Serial2 untuk sensor ultrasonik
  delay(200);
  setupSensorMagnet(SLAVEID_MAGNET_DEPAN);  
  setupSensorUltrasonic(SLAVEID_ULTRASONIK_DEPAN);
  setupHook();  // setupBuzzer();
  setupWifi();  // Setup WiFi configuration
  // Web server setup now handled by HTTPManager in main agv_sami.ino
  setupTombol();
  setupRfid();
  // Initialize performance optimization
  resetSensorTimers();
  
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.println("SETUP ALL SELESAI");
  delay(1000);
}
