void setupMotor()
{
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

void setupEncoder()
{
  pinMode(encKananA, INPUT);
  pinMode(encKiriA, INPUT);

  attachInterrupt(digitalPinToInterrupt(encKananA), encKanan, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encKiriA), encKiri, CHANGE);

  for (int i = 0; i < numOutputs; ++i)
  {
    pidData[i].error = 0.0;
    pidData[i].integral = 0.0;
    pidData[i].derivative = 0.0;
    pidData[i].previousError = 0.0;
  }
}


// setup display
void initializeDisplay()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.clearDisplay();
  display.setTextSize(2); // Ukuran tulisan  //Ukuran tulisan
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0); // Koordinat awal tulisan (x,y) dimulai dari atas-kiri
  display.print("Mulai Program AGV");
  delay(1000);
  display.setTextSize(1);
  display.display();
}

void setupDisplay()
{
  // Initialize I2C
  Wire.begin();
  Wire.setClock(400000);
  delay(100);
  
  // Initialize display
  initializeDisplay();
}

void setupSensorMagnet()
{
    Serial2.begin(BAUDRATE, SERIAL_8N1, RXD2, TXD2);
    pinMode(MAX485_RE, OUTPUT);
    pinMode(MAX485_DE, OUTPUT);
    digitalWrite(MAX485_RE, 0);
    digitalWrite(MAX485_DE, 0);
    node.begin(1, Serial2); // Slave ID = 1 (default pabrik adalah 1)
    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);
    Serial.println(F("Inisialisasi Sensor Magnet selesai."));
}

void setupAll(){
    setupMotor();
    setupEncoder();
    setupDisplay();
    setupSensorMagnet();
    Serial.println("SETUP ALL SELESAI");
}

