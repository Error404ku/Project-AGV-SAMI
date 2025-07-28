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

void setupMusicAndLed() {
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
// void setupEncoder() {
//   pinMode(encKananA, INPUT);
//   pinMode(encKiriA, INPUT);

//   attachInterrupt(digitalPinToInterrupt(encKananA), encKanan, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(encKiriA), encKiri, CHANGE);

//   for (int i = 0; i < numOutputs; ++i) {
//     pidData[i].error = 0.0;
//     pidData[i].integral = 0.0;
//     pidData[i].derivative = 0.0;
//     pidData[i].previousError = 0.0;
//   }
// }


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
  Wire.begin(3, 8);
  // Wire.setClock(200000);
  delay(100);

  // Initialize display
  initializeDisplay();
}
void setupWebServer() {
  // Muat daftar stasiun dari Preferences saat startup
  loadStationsListFromPreferences();
  // lcd.clear();
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
    if (wifiAttempts > 60) { // 30 seconds timeout
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
  server.on("/updatestations", HTTP_POST, handleUpdateStations); // Untuk menyimpan/menimpa daftar stasiun
  server.on("/showstations", HTTP_GET, handleShowStations);       // Untuk menampilkan daftar stasiun
  
  // Halaman utama server
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", "Server ESP32 Aktif. <br> Coba: <br> /updatestations (POST)<br> /showstations (GET)");
  });

  // Handler untuk endpoint tidak ditemukan
  server.onNotFound([]() {
    server.send(404, "text/plain", "Endpoint tidak ditemukan.");
  });
  
  server.begin(); // Memulai server HTTP
  Serial.println("Server HTTP telah dimulai.");
  delay(1000);
}

void setupUltrasonikWithParams(int slaveId, int baudrate = 9600) {
  // Initialize ultrasonic sensor with ModbusMaster
  initUltrasonicSensor(slaveId);
  Serial.printf("--- Setup Ultrasonik Slave ID: %d ---\n", slaveId);
}

void setupSensorMagnet(int slaveId, int baudrate = 9600) {
  Serial1.begin(baudrate, SERIAL_8N1, RS485_RX, RS485_TX);
  pinMode(MAX485_RE, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE, 0);
  digitalWrite(MAX485_DE, 0);
  node.begin(slaveId, Serial1);  // Slave ID
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  Serial.printf("Inisialisasi Sensor Magnet selesai. Slave ID: %d\n", slaveId);
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

// void setupBuzzer() {
//   pinMode(BUZZER_PIN, OUTPUT);
//   digitalWrite(BUZZER_PIN, LOW); // Ensure buzzer is off initially
//   Serial.println("Buzzer initialized");
// }



void setupAll() {
  setupMotor();
  // setupEncoder();
  setupMusicAndLed();
  setupDisplay();
  setupMenu();  // Initialize menu system
  setupSensorMagnet(SLAVEID_MAGNET_DEPAN, BAUDRATE);
  setupUltrasonikWithParams(SLAVEID_ULTRASONIK_DEPAN, BAUDRATE);
  setupHook();// setupBuzzer();
  // setupWebServer();
  setupTombol();
  setupRfid();
  Serial.println("SETUP ALL SELESAI");
}
