void setupMotor() {
  // Set motor control pins as outputs
  pinMode(mdKananA, OUTPUT);
  // pinMode(mdKananB, OUTPUT);
  pinMode(mdKiriA, OUTPUT);
  // pinMode(mdKiriB, OUTPUT);
  // pinMode(ENKanan, OUTPUT);
  // pinMode(ENKiri, OUTPUT);

  // Setup PWM for enable pins
  ledcSetup(channelKanan, pwmFrequency, pwmResolution);
  ledcAttachPin(mdKananB, channelKanan);

  ledcSetup(channelKiri, pwmFrequency, pwmResolution);
  ledcAttachPin(mdKiriB, channelKiri);
}

void setupEncoder() {
  pinMode(encKananA, INPUT);
  pinMode(encKiriA, INPUT);

  attachInterrupt(digitalPinToInterrupt(encKananA), encKanan, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encKiriA), encKiri, CHANGE);

  for (int i = 0; i < numOutputs; ++i) {
    pidData[i].error = 0.0;
    pidData[i].integral = 0.0;
    pidData[i].derivative = 0.0;
    pidData[i].previousError = 0.0;
  }
}


// setup display
void initializeDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.clearDisplay();
  display.setTextSize(2);  // Ukuran tulisan  //Ukuran tulisan
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);  // Koordinat awal tulisan (x,y) dimulai dari atas-kiri
  display.print("Mulai Program AGV");
  delay(1000);
  display.setTextSize(1);
  display.display();
}

void setupDisplay() {
  // Initialize I2C
  Wire.begin();
  Wire.setClock(400000);
  delay(100);

  // Initialize display
  initializeDisplay();
}

void setupSensorMagnet() {
  Serial2.begin(BAUDRATE, SERIAL_8N1, RXD2, TXD2);
  pinMode(MAX485_RE, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE, 0);
  digitalWrite(MAX485_DE, 0);
  node.begin(1, Serial2);  // Slave ID = 1 (default pabrik adalah 1)
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  Serial.println(F("Inisialisasi Sensor Magnet selesai."));
}

void setupWebServer() {
  // Muat daftar stasiun dari Preferences saat startup
  loadStationsListFromPreferences();
  
  // Konfigurasi dan mulai koneksi Wi-Fi
  WiFi.begin(ssid, password);
  if (!WiFi.config(staticIP, gateway, subnet, dns)) {
    Serial.println("Error: Gagal mengkonfigurasi IP Statis");
  }
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nKoneksi Wi-Fi berhasil!");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());

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
}

// void setupUltrasonik() {
//   Serial1.begin(115200, SERIAL_8N1, rxPinUltrasonikA, txPinUltrasonikA);

//   Serial.println("\n\n--- Program Parser Sensor Ultrasonik ---");
//   Serial.println("Mencari paket data dari sensor...");
// }

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

void setupAll() {
  setupMotor();
  setupEncoder();
  setupDisplay();
  setupMenu();  // Initialize menu system
  setupTombol();
  setupSensorMagnet();
  setupWebServer();
  setupRfid();
  Serial.println("SETUP ALL SELESAI");
}
