void setupMotor() {
  // Set motor control pins as outputs untuk L298N
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Setup PWM for enable pins (ENA dan ENB)
  ledcSetup(channelKanan, pwmFrequency, pwmResolution);
  ledcAttachPin(ENA, channelKanan);

  ledcSetup(channelKiri, pwmFrequency, pwmResolution);
  ledcAttachPin(ENB, channelKiri);

  // Matikan motor saat startup
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  ledcWrite(channelKanan, 0);
  ledcWrite(channelKiri, 0);
}

void setupMusic() {
  pinMode(pinMusic1, OUTPUT);
  pinMode(pinMusic2, OUTPUT);
  pinMode(pinMusic3, OUTPUT);
  pinMode(pinMusic4, OUTPUT);
  // digital high semua
  digitalWrite(pinMusic1, HIGH);
  digitalWrite(pinMusic2, HIGH);
  digitalWrite(pinMusic3, HIGH);
  digitalWrite(pinMusic4, HIGH);
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
  lcd.begin(LCD_COLUMNS, LCD_ROWS);  // Inisialisasi LCD
  lcd.backlight();                   // Nyalakan backlight
  // Test LCD communication
  lcd.setCursor(0, 0);
  lcd.print("Mulai Program");
  lcd.setCursor(0, 1);
  lcd.print("AGV System");
  // Simple LCD test - try to set cursor and check if it works
  delay(100);
  lcd.setCursor(0, 0);
  // If LCD is not responding, this will be detected in normal operation
  lcd.clear();
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
 
  int wifiAttempts = 0;
  bool wifiConnected = false;
  
  // Try to connect to WiFi with saved credentials
  if (strlen(ssid) > 0 && strlen(password) > 0) {
    Serial.printf("Connecting to WiFi: %s\n", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && wifiAttempts < 5) { // Increased attempts for initial connection
      lcd.setCursor(0, 0);
      lcd.print("MENCARI WIFI");
      Serial.print(".");
      delay(1000);
      wifiAttempts++;
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\nKoneksi Wi-Fi berhasil!");
    Serial.print("Alamat IP: ");
    Serial.println(WiFi.localIP());
    lcd.setCursor(0, 1);
    lcd.print("Wi-Fi Berhasil!");
    lcd.setCursor(0, 2);
    lcd.print("IP: ");
    lcd.print(WiFi.localIP());
  } else {
    Serial.println("\nKoneksi Wi-Fi gagal. Memulai sebagai Access Point.");
    lcd.setCursor(0, 1);
    lcd.print("Wi-Fi Gagal!");
    delay(1000);
   }
  
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
  Serial.println("Server HTTP telah dimulai.");
  delay(500);
}

void setupUltrasonikWithParams(int slaveId) {
  // Initialize ultrasonic sensor with ModbusMaster
  initUltrasonicSensor(slaveId);
}

void setupRS485(int baudrate) {
  Serial.printf("[DEBUG] setupRS485 dimulai dengan baudrate: %d\n", baudrate);
  Serial.printf("[DEBUG] RS485 pins - RX: %d, TX: %d, RE: %d, DE: %d\n", RS485_RX, RS485_TX, MAX485_RE, MAX485_DE);

  // Initialize Serial1 for RS485 communication
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
    Serial.println("[SUCCESS] RS485 komunikasi berhasil diinisialisasi!");
  } else {
    Serial.println("[ERROR] Gagal menginisialisasi RS485 komunikasi!");
  }

  Serial.printf("[INFO] setupRS485 selesai dengan baudrate: %d\n", baudrate);
}

void setupSensorMagnet(int slaveId) {
  Serial.printf("[DEBUG] setupSensorMagnet dimulai dengan Slave ID: %d\n", slaveId);

  // Check if Serial1 is available
  if (!Serial1) {
    Serial.println("[ERROR] Serial1 tidak tersedia untuk setup sensor magnet!");
    return;
  }

  Serial.printf("[DEBUG] Menginisialisasi ModbusMaster dengan Slave ID: %d\n", slaveId);
  node.begin(slaveId, Serial1);  // Slave ID
  node.preTransmission(preTransmissionMagnet);
  node.postTransmission(postTransmissionMagnet);

  Serial.printf("[DEBUG] Mengatur magnet slave ID ke: %d\n", slaveId);
  setMagnetSlaveId(slaveId);

  // Test communication
  Serial.println("[DEBUG] Testing komunikasi dengan sensor magnet...");
  uint8_t testResult = node.readHoldingRegisters(0x0000, 2);
  if (testResult == node.ku8MBSuccess) {
    Serial.println("[SUCCESS] Test komunikasi sensor magnet berhasil!");
  } else {
    Serial.printf("[WARNING] Test komunikasi sensor magnet gagal. Error: 0x%02X\n", testResult);
  }

  Serial.printf("[INFO] Inisialisasi Sensor Magnet selesai. Slave ID: %d\n", slaveId);
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

  // Load PID settings
  tempKp = preferences.getDouble("kpLinefollower", 0.0);
  tempKi = preferences.getDouble("kiLinefollower", 0.0);
  tempKd = preferences.getDouble("kdLinefollower", 0.0);
  
  // Load Forward PID settings
  tempKpForward = preferences.getDouble("kpForward", 0.0);
  tempKiForward = preferences.getDouble("kiForward", 0.0);
  tempKdForward = preferences.getDouble("kdForward", 0.0);
  
  // Load Backward PID settings
  tempKpBackward = preferences.getDouble("kpBackward", 0.0);
  tempKiBackward = preferences.getDouble("kiBackward", 0.0);
  tempKdBackward = preferences.getDouble("kdBackward", 0.0);
  
  // Debug: Print loaded values
  Serial.println("=== PID Values Loaded from Preferences ===");
  Serial.println("Forward PID - Kp: " + String(tempKpForward) + ", Ki: " + String(tempKiForward) + ", Kd: " + String(tempKdForward));
  Serial.println("Backward PID - Kp: " + String(tempKpBackward) + ", Ki: " + String(tempKiBackward) + ", Kd: " + String(tempKdBackward));

  // Load Motor settings
  tempBaseSpeed = preferences.getInt("baseSpeed", 1000);

  // Load Motor invert settings
  tempInvertY = preferences.getBool("invertY", false);
  tempInvertX = preferences.getBool("invertX", false);
  tempInvertKanan = preferences.getBool("invertKanan", false);
  tempInvertKiri = preferences.getBool("invertKiri", false);
  tempInvertHook = preferences.getBool("invertHook", false);

  // Load Music mapping settings
  tempMusicStationPin = preferences.getInt("musicStation", 0);
  tempMusicErrorPin = preferences.getInt("musicError", 1);
  tempMusicDetectPin = preferences.getInt("musicDetect", 2);
  tempMusicKomputerPin = preferences.getInt("musicKomputer", 3);

  // Apply PID values
  kpLinefollower = tempKp;
  kiLinefollower = tempKi;
  kdLinefollower = tempKd;
  
  // Apply Forward PID values
  kpLinefollowerForward = tempKpForward;
  kiLinefollowerForward = tempKiForward;
  kdLinefollowerForward = tempKdForward;
  
  // Apply Backward PID values
  kpLinefollowerBackward = tempKpBackward;
  kiLinefollowerBackward = tempKiBackward;
  kdLinefollowerBackward = tempKdBackward;
  
  // Debug: Print applied values
  Serial.println("=== PID Values Applied to Global Variables ===");
  Serial.println("Forward PID Global - Kp: " + String(kpLinefollowerForward) + ", Ki: " + String(kiLinefollowerForward) + ", Kd: " + String(kdLinefollowerForward));
  Serial.println("Backward PID Global - Kp: " + String(kpLinefollowerBackward) + ", Ki: " + String(kiLinefollowerBackward) + ", Kd: " + String(kdLinefollowerBackward));

  // Apply Motor values
  baseSpeed = tempBaseSpeed;

  // Apply Motor invert values
  invertMotorY = tempInvertY;
  invertMotorX = tempInvertX;
  invertMotorKanan = tempInvertKanan;
  invertMotorKiri = tempInvertKiri;
  invertHook = tempInvertHook;

  // Apply Music mapping values
  musicStationPin = tempMusicStationPin;
  musicErrorPin = tempMusicErrorPin;
  musicDetectPin = tempMusicDetectPin;
  musicKomputerPin = tempMusicKomputerPin;

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

  Serial.print("Loaded targetStationsList size: ");
  Serial.println(targetStationsList.size());
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
  setupRS485(BAUDRATE);
  delay(200);
  setupSensorMagnet(SLAVEID_MAGNET_DEPAN);  
  setupUltrasonikWithParams(SLAVEID_ULTRASONIK_DEPAN);
  setupHook();  // setupBuzzer();
  setupWifi();  // Setup WiFi configuration
  setupWebServer();
  setupTombol();
  setupRfid();
  
  // Load RFID data from preferences
  loadRfidUjungFromPreferences();
  loadRfidWarehouseFromPreferences();
  // loadAutoStationsFromPreferences(); // Function removed - using existing RFID station management
  loadTerminalRfid();
  
  // Initialize performance optimization
  resetSensorTimers();
  
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.println("SETUP ALL SELESAI");
  delay(1000);
}
