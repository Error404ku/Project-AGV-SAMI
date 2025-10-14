// Motor Serial Communication - ESP32 Master
// RPM-based motor control via Serial communication to ESP32 motor controller

// Forward declaration for safeDelay function from menu.ino
extern void safeDelay(unsigned long ms);

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
  
  // Debug output untuk monitoring
  // Serial.printf("[MASTER] Sending RPM command: %s\n", perintah.c_str());
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

// Function to send PID values TO right motor specifically
void sendPidValuesRight(double kp, double ki, double kd) {
  // Kosongkan buffer serial dulu
  while (Serial.available()) {
    Serial.read();
  }
  
  // Batasi nilai sesuai permintaan (maksimal 200)
  kp = constrain(kp, 0, 200);
  ki = constrain(ki, 0, 200);
  kd = constrain(kd, 0, 200);
  
  // Format pesan PID untuk motor kanan
  String pidCommand = "PIDRIGHT" + String(kp, 3) + "," + String(ki, 3) + "," + String(kd, 3);
  
  // Kirim perintah PID
  Serial.println(pidCommand);
  
  // Simpan juga secara lokal sebagai cadangan
  motorPidKpRight = kp;
  motorPidKiRight = ki;
  motorPidKdRight = kd;
  
  // Tambahkan delay kecil untuk memastikan pengiriman
  delay(50);
}

// Function to send PID values TO left motor specifically
void sendPidValuesLeft(double kp, double ki, double kd) {
  // Kosongkan buffer serial dulu
  while (Serial.available()) {
    Serial.read();
  }
  
  // Batasi nilai sesuai permintaan (maksimal 200)
  kp = constrain(kp, 0, 200);
  ki = constrain(ki, 0, 200);
  kd = constrain(kd, 0, 200);
  
  // Format pesan PID untuk motor kiri
  String pidCommand = "PIDLEFT" + String(kp, 3) + "," + String(ki, 3) + "," + String(kd, 3);
  
  // Kirim perintah PID
  Serial.println(pidCommand);
  
  // Simpan juga secara lokal sebagai cadangan
  motorPidKpLeft = kp;
  motorPidKiLeft = ki;
  motorPidKdLeft = kd;
  
  // Tambahkan delay kecil untuk memastikan pengiriman
  delay(50);
}


// Function to process messages FROM motor controller (including startup PIDVALUES)
void processMotorControllerMessage(String message) {
  message.trim();
  
  // Handle RPM acknowledgment from motor controller
  if (message.startsWith("RPM_ACK:")) {
    String rpmData = message.substring(8); // Remove "RPM_ACK:"
    int commaPos = rpmData.indexOf(',');
    
    if (commaPos > 0) {
      float rpmKanan = rpmData.substring(0, commaPos).toFloat();
      float rpmKiri = rpmData.substring(commaPos + 1).toFloat();
      
      // Update current RPM status for monitoring
      // Serial.printf("RPM Command Confirmed - Kanan: %.1f, Kiri: %.1f\n", rpmKanan, rpmKiri);
    }
  }
  // Handle RPM errors from motor controller
  else if (message.startsWith("ERROR:INVALID_RPM_CMD")) {
    // Log RPM command error
    // Serial.println("[MASTER] Received RPM error from slave");
  }
  // Handle PIDRIGHT command from motor controller
  else if (message.startsWith("PIDRIGHT:")) {
    String pidData = message.substring(9); // Remove "PIDRIGHT:"
    
    int firstComma = pidData.indexOf(',');
    int secondComma = pidData.indexOf(',', firstComma + 1);
    
    if (firstComma > 0 && secondComma > firstComma) {
      double kp = pidData.substring(0, firstComma).toDouble();
      double ki = pidData.substring(firstComma + 1, secondComma).toDouble(); 
      double kd = pidData.substring(secondComma + 1).toDouble();
      
      // Update Right Motor PID variables
      motorPidKpRight = kp;
      motorPidKiRight = ki;
      motorPidKdRight = kd;
      tempMotorPidKpRight = kp;  // Update temp variables for menu display
      tempMotorPidKiRight = ki;
      tempMotorPidKdRight = kd;
    }
  } else if (message.startsWith("PIDRIGHT_VALUES:")) {
    // Handle startup PID values from motor controller slave
    String pidData = message.substring(16); // Remove "PIDRIGHT_VALUES:"
    
    int firstComma = pidData.indexOf(',');
    int secondComma = pidData.indexOf(',', firstComma + 1);
    
    if (firstComma > 0 && secondComma > firstComma) {
      double kp = pidData.substring(0, firstComma).toDouble();
      double ki = pidData.substring(firstComma + 1, secondComma).toDouble(); 
      double kd = pidData.substring(secondComma + 1).toDouble();
      
      // Update Right Motor PID variables
      motorPidKpRight = kp;
      motorPidKiRight = ki;
      motorPidKdRight = kd;
      tempMotorPidKpRight = kp;  // Update temp variables for menu display
      tempMotorPidKiRight = ki;
      tempMotorPidKdRight = kd;
      
      // Mark that we received right motor PID data
      pidDataReceivedRight = true;
      Serial.println("Received Right Motor PID: Kp=" + String(kp) + " Ki=" + String(ki) + " Kd=" + String(kd));
    }
  } else if (message.startsWith("PIDLEFT:")) {
    String pidData = message.substring(8); // Remove "PIDLEFT:"
    
    int firstComma = pidData.indexOf(',');
    int secondComma = pidData.indexOf(',', firstComma + 1);
    
    if (firstComma > 0 && secondComma > firstComma) {
      double kp = pidData.substring(0, firstComma).toDouble();
      double ki = pidData.substring(firstComma + 1, secondComma).toDouble(); 
      double kd = pidData.substring(secondComma + 1).toDouble();
      
      // Update Left Motor PID variables
      motorPidKpLeft = kp;
      motorPidKiLeft = ki;
      motorPidKdLeft = kd;
      tempMotorPidKpLeft = kp;  // Update temp variables for menu display
      tempMotorPidKiLeft = ki;
      tempMotorPidKdLeft = kd;
    }
  } else if (message.startsWith("PIDLEFT_VALUES:")) {
    // Handle startup PID values from motor controller slave
    String pidData = message.substring(15); // Remove "PIDLEFT_VALUES:"
    
    int firstComma = pidData.indexOf(',');
    int secondComma = pidData.indexOf(',', firstComma + 1);
    
    if (firstComma > 0 && secondComma > firstComma) {
      double kp = pidData.substring(0, firstComma).toDouble();
      double ki = pidData.substring(firstComma + 1, secondComma).toDouble(); 
      double kd = pidData.substring(secondComma + 1).toDouble();
      
      // Update Left Motor PID variables
      motorPidKpLeft = kp;
      motorPidKiLeft = ki;
      motorPidKdLeft = kd;
      tempMotorPidKpLeft = kp;  // Update temp variables for menu display
      tempMotorPidKiLeft = ki;
      tempMotorPidKdLeft = kd;
      
      // Mark that we received left motor PID data
      pidDataReceivedLeft = true;
      Serial.println("Received Left Motor PID: Kp=" + String(kp) + " Ki=" + String(ki) + " Kd=" + String(kd));
    }
  } else if (message.startsWith("RPMSHOW:")) {
    String rpmData = message.substring(8); // Remove "RPMSHOW:"
    
    int commaPos = rpmData.indexOf(',');
    if (commaPos > 0) {
      int rpmKanan = rpmData.substring(0, commaPos).toInt();
      int rpmKiri = rpmData.substring(commaPos + 1).toInt();
      
      // Update current RPM values
      currentRpmKanan = rpmKanan;
      currentRpmKiri = rpmKiri;
    }
  } else if (message.startsWith("PIDRIGHT_SAVED:")) {
    // Handle PID Right Motor save confirmation
    String pidData = message.substring(15); // Remove "PIDRIGHT_SAVED:"
    
    // Optional: Show confirmation on LCD briefly
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Right PID Saved");
    lcd.setCursor(0, 1);
    lcd.print(pidData);
    delay(500);
  } else if (message.startsWith("PIDLEFT_SAVED:")) {
    // Handle PID Left Motor save confirmation
    String pidData = message.substring(14); // Remove "PIDLEFT_SAVED:"
    
    // Optional: Show confirmation on LCD briefly
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Left PID Saved");
    lcd.setCursor(0, 1);
    lcd.print(pidData);
    delay(500);
  } else if (message.startsWith("AUTOTUNE:")) {
    // Handle auto-tuning responses from motor controller slave
    parseTuningResponse(message);
  }
}

// Fungsi untuk meminta data PID dari motor controller slave
void requestPidDataFromSlave() {
  pidDataReceived = false;
  pidDataReceivedRight = false;
  pidDataReceivedLeft = false;
  systemReadyToRun = false;
  pidRequestStartTime = millis();
  
  // Display status di LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Requesting PID...");
  lcd.setCursor(0, 1);
  lcd.print("Right & Left Motors");
  
  // Kirim perintah PIDSHOW ke motor controller
  Serial.println("PIDSHOW");
}

// Fungsi untuk mengecek apakah sistem siap untuk running
bool checkSystemReadyStatus() {
  // Cek apakah kedua PID data (kanan dan kiri) sudah diterima
  if (pidDataReceivedRight && pidDataReceivedLeft && systemReadyToRun) {
    return true;
  }
  
  // Update legacy flag untuk backward compatibility
  if (pidDataReceivedRight && pidDataReceivedLeft) {
    pidDataReceived = true;
    systemReadyToRun = true;
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
    safeDelay(1000);
    
    // Set default PID values jika timeout untuk motor kanan
    if (!pidDataReceivedRight) {
      motorPidKpRight = 1.0;
      motorPidKiRight = 0.15;
      motorPidKdRight = 0.0;
      tempMotorPidKpRight = 1.0;
      tempMotorPidKiRight = 0.15;
      tempMotorPidKdRight = 0.0;
      Serial.println("Using default Right Motor PID values");
    }
    
    // Set default PID values jika timeout untuk motor kiri
    if (!pidDataReceivedLeft) {
      motorPidKpLeft = 1.0;
      motorPidKiLeft = 0.15;
      motorPidKdLeft = 0.0;
      tempMotorPidKpLeft = 1.0;
      tempMotorPidKiLeft = 0.15;
      tempMotorPidKdLeft = 0.0;
      Serial.println("Using default Left Motor PID values");
    }
    
    // Set legacy values
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
    Serial.println("PS"); // Retry request
    lastRetry = millis();
    
    // Update display dengan status PID yang sudah diterima
    lcd.setCursor(0, 1);
    String status = "";
    if (pidDataReceivedRight) status += "R✓ ";
    else status += "R✗ ";
    if (pidDataReceivedLeft) status += "L✓";
    else status += "L✗";
    status += " Waiting...";
    lcd.print(status + "          "); // Padding untuk clear line
    
    Serial.println("PID Status - Right: " + String(pidDataReceivedRight ? "OK" : "WAIT") + 
                   " Left: " + String(pidDataReceivedLeft ? "OK" : "WAIT"));
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

