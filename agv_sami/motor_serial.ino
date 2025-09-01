// Motor Serial Communication - ESP32 Master
// RPM-based motor control via Serial communication to ESP32 motor controller

void setupMotorSerial() {
  // Inisialisasi Serial0 untuk komunikasi dengan ESP32 kedua
  Serial.begin(115200);
  
  // Request PID data dari motor controller saat startup
  requestPidDataFromSlave();
}

// =============== NEW RPM-BASED FUNCTIONS ===============


void sendRPM(int rpmKiri, int rpmKanan) {
  // Batasi RPM dalam range yang aman (10-90)
  rpmKiri = constrain(rpmKiri, -90, 90);
  rpmKanan = constrain(rpmKanan, -90, 90);
  
  String perintah = "RPM" + String(rpmKanan) + "," + String(rpmKiri);
  Serial.println(perintah);
}

// =============== ESSENTIAL LEGACY PWM SUPPORT ===============
// Only keep essential functions for AGV operations compatibility

void sendMotorCommand(int speedKiri, int speedKanan) {
  speedKiri = constrain(speedKiri, -4095, 4095);
  speedKanan = constrain(speedKanan, -4095, 4095);
  
  String perintah = "L" + String(speedKiri) + "R" + String(speedKanan);
  Serial.println(perintah);
}

// Essential stop function for error handling
void motorStop() {
  pwmMotor(0,0);  // Use RPM stop command instead of PWM
}

void handleMotorControllerSerial() {
  // Check for incoming data from motor controller
  esp_task_wdt_reset();

  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    if (inChar == '\n') {
      motorControllerStringComplete = true;
    } else {
      motorControllerBuffer += inChar;
    }
  }
  
  // Process complete message
  if (motorControllerStringComplete) {
    processMotorControllerMessage(motorControllerBuffer);
    motorControllerBuffer = "";
    motorControllerStringComplete = false;
  }
}

// Function to send PID values TO motor controller
void sendPidValues(double kp, double ki, double kd) {
  // Kosongkan buffer serial dulu
  while (Serial.available()) {
    Serial.read();
  }
  
  // Pastikan dalam range yang valid di sisi slave (kp <= 100, ki <= 50, kd <= 50)
  kp = constrain(kp, 0, 100);
  ki = constrain(ki, 0, 50);
  kd = constrain(kd, 0, 50);
  
  // Format pesan PID dengan presisi yang tepat
  String pidCommand = "PID" + String(kp, 3) + "," + String(ki, 3) + "," + String(kd, 3);
  
  // Kirim perintah PID
  Serial.println(pidCommand);
  
  // Tampilkan informasi di Serial Monitor
  Serial.println("Sending PID command: " + pidCommand);
  
  // Simpan juga secara lokal sebagai cadangan
  motorPidKp = kp;
  motorPidKi = ki;
  motorPidKd = kd;
  
  // Tambahkan delay kecil untuk memastikan pengiriman
  delay(50);
}


// Function to process messages FROM motor controller (including startup PIDVALUES)
void processMotorControllerMessage(String message) {
  message.trim();
  
  // Handle PIDVALUES command from motor controller
  if (message.startsWith("PIDVALUES:")) {
    String pidData = message.substring(10); // Remove "PIDVALUES:"
    
    int firstComma = pidData.indexOf(',');
    int secondComma = pidData.indexOf(',', firstComma + 1);
    
    if (firstComma > 0 && secondComma > firstComma) {
      double kp = pidData.substring(0, firstComma).toDouble();
      double ki = pidData.substring(firstComma + 1, secondComma).toDouble(); 
      double kd = pidData.substring(secondComma + 1).toDouble();
      
      // Update AGV_SAMI variables with values from motor controller
      motorPidKp = kp;
      motorPidKi = ki;
      motorPidKd = kd;
      tempMotorPidKp = kp;  // Update temp variables for menu display
      tempMotorPidKi = ki;
      tempMotorPidKd = kd;
      
      // Set flag bahwa PID data telah diterima
      pidDataReceived = true;
      systemReadyToRun = true;
      
      // Display konfirmasi di LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("PID Data Received");
      lcd.setCursor(0, 1);
      lcd.print("System Ready");
      delay(1000);
    }
  }else if (message.startsWith("RPMSHOW:")) {
    String rpmData = message.substring(8); // Remove "RPMSHOW:"
    
    int commaPos = rpmData.indexOf(',');
    if (commaPos > 0) {
      int rpmKanan = rpmData.substring(0, commaPos).toInt();
      int rpmKiri = rpmData.substring(commaPos + 1).toInt();
      
      // Update current RPM values
      currentRpmKanan = rpmKanan;
      currentRpmKiri = rpmKiri;
    }
  }
}

// Fungsi untuk meminta data PID dari motor controller slave
void requestPidDataFromSlave() {
  pidDataReceived = false;
  systemReadyToRun = false;
  pidRequestStartTime = millis();
  
  // Display status di LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Requesting PID...");
  lcd.setCursor(0, 1);
  lcd.print("From Motor Slave");
  
  // Kirim perintah PIDSHOW ke motor controller
  Serial.println("PIDSHOW");
}

// Fungsi untuk mengecek apakah sistem siap untuk running
bool checkSystemReadyStatus() {
  if (pidDataReceived && systemReadyToRun) {
    return true;
  }
  
  // Cek timeout
  if (millis() - pidRequestStartTime > PID_REQUEST_TIMEOUT) {
    // Timeout - tampilkan error dan set default values
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PID Request");
    lcd.setCursor(0, 1);
    lcd.print("TIMEOUT - Default");
    delay(1000);
    
    // Set default PID values jika timeout
    motorPidKp = 1.0;
    motorPidKi = 0.15;
    motorPidKd = 0.0;
    tempMotorPidKp = 1.0;
    tempMotorPidKi = 0.15;
    tempMotorPidKd = 0.0;
    
    systemReadyToRun = true; // Allow system to proceed with defaults
    return true;
  }
  
  // Retry request setiap 1 detik
  static unsigned long lastRetry = 0;
  if (millis() - lastRetry > 1000) {
    Serial.println("PIDSHOW"); // Retry request
    lastRetry = millis();
    
    // Update display
    lcd.setCursor(0, 1);
    lcd.print("Waiting...      ");
  }
  
  return false;
}

// Fungsi untuk meminta data RPM current dari motor controller slave
void requestRpmDataFromSlave() {
  // Kosongkan buffer serial dulu untuk menghindari buffer overflow
  while (Serial.available()) {
    Serial.read();
  }
  Serial.println("RS");
}

