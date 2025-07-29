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
  Wire.setClock(100000);
  delay(100);
  // Initialize display
  initializeDisplay();
}

void setupWebServer() {
  // Muat daftar stasiun dari Preferences saat startup
  loadStationsListFromPreferences();
  lcd.setCursor(0, 0);
  lcd.print("SETUP WIFI");

  // Konfigurasi dan mulai koneksi Wi-Fi
  WiFi.begin(ssid, password);
  if (!WiFi.config(staticIP, gateway, subnet, dns)) {
    Serial.println("Error: Gagal mengkonfigurasi IP Statis");
    error(ERROR_WIFI_CONNECTION, "Gagal config IP static");
  }

  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    // lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MENCARI WIFI");
    delay(500);

    wifiAttempts++;
    if (wifiAttempts > 60) {  // 30 seconds timeout
      error(ERROR_WIFI_CONNECTION, "WiFi timeout 30 detik");
    }
  }
  Serial.println("\nKoneksi Wi-Fi berhasil!");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wi-Fi Berhasil!");
  lcd.setCursor(0, 1);
  lcd.print("IP: ");
  lcd.print(WiFi.localIP());
  // Registrasi Endpoint HTTP yang diminta
  server.on("/updatestations", HTTP_POST, handleUpdateStations);  // Untuk menyimpan/menimpa daftar stasiun
  server.on("/showstations", HTTP_GET, handleShowStations);       // Untuk menampilkan daftar stasiun

  // Halaman utama server
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", "Server ESP32 Aktif. <br> Coba: <br> /updatestations (POST)<br> /showstations (GET)");
  });

  // Handler untuk endpoint tidak ditemukan
  server.onNotFound([]() {
    server.send(404, "text/plain", "Endpoint tidak ditemukan.");
  });

  server.begin();  // Memulai server HTTP
  Serial.println("Server HTTP telah dimulai.");
  delay(1000);
}

void setupUltrasonikWithParams(int slaveId) {
  // Initialize ultrasonic sensor with ModbusMaster
  initUltrasonicSensor(slaveId);
  Serial.printf("--- Setup Ultrasonik Slave ID: %d ---\n", slaveId);
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
  preferences.begin("agv-settings", false);

  // Load PID settings
  tempKp = preferences.getDouble("kpLinefollower", 70.0);
  tempKi = preferences.getDouble("kiLinefollower", 0.0);
  tempKd = preferences.getDouble("kdLinefollower", 0.0);

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

  // Load stations list from HTTP preferences
  loadStationsListFromPreferences();

  Serial.print("Loaded stationsList size: ");
  Serial.println(stationsList.size());
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
  setupSensorMagnet(SLAVEID_MAGNET_DEPAN);
  setupUltrasonikWithParams(SLAVEID_ULTRASONIK_DEPAN);
  setupHook();  // setupBuzzer();
  // setupWebServer();
  setupTombol();
  setupRfid();
  lcd.setCursor(0, 0);
  lcd.println("SETUP ALL SELESAI");
  delay(1000);
}
