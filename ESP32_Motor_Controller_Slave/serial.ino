void serialEvent() {
  // Batasi jumlah karakter yang dibaca setiap loop untuk mencegah WDT reset.
  int maxCharsPerLoop = 64;
  int charsRead = 0;

  while (Serial1.available() && charsRead < maxCharsPerLoop) {
    char inChar = (char)Serial1.read();
    charsRead++;

    if (inChar == '\n') {
      stringComplete = true;
      break; // Keluar dari loop setelah menemukan newline
    } else {
      inputString += inChar;
    }
  }
}

void processCommand(String command) {
  command.trim();  // Remove whitespace
  Serial.println("Received command: " + command);  // Debug message
  
  if (command.startsWith("PIDRIGHT")) {  // Handler untuk PID motor kanan
    String params = command.substring(8); // harus "PIDRIGHT<kp>,<ki>,<kd>" tanpa ':'
    int firstComma = params.indexOf(',');
    int secondComma = params.indexOf(',', firstComma + 1);
    
    Serial.println("PIDRIGHT Command Parsing:");
    Serial.println("Params: " + params);
    Serial.println("First comma at: " + String(firstComma));
    Serial.println("Second comma at: " + String(secondComma));
    
    if (firstComma > 0 && secondComma > firstComma) {
      double kp = params.substring(0, firstComma).toDouble();
      double ki = params.substring(firstComma + 1, secondComma).toDouble();
      double kd = params.substring(secondComma + 1).toDouble();
      
      Serial.println("Parsed Right Motor values - Kp: " + String(kp, 4) + " Ki: " + String(ki, 4) + " Kd: " + String(kd, 4));
      Serial.println("Validation check: " + String(kp >= 0 && kp <= 200 && ki >= 0 && ki <= 200 && kd >= 0 && kd <= 200));
      
      if (kp >= 0 && kp <= 200 && ki >= 0 && ki <= 200 && kd >= 0 && kd <= 200) {
        pidConfigRight.kp = kp; 
        pidConfigRight.ki = ki; 
        pidConfigRight.kd = kd;
        savePIDParametersRight();
        
        Serial1.println("PIDRIGHT_SAVED:" + String(kp, 3) + "," + String(ki, 3) + "," + String(kd, 3));
        Serial.println("Right Motor PID saved successfully");
      } else {
        Serial.println("ERROR: Right Motor PID values out of allowed range!");
      }
    }
  } else if (command.startsWith("PIDLEFT")) {  // Handler untuk PID motor kiri
    String params = command.substring(7); // harus "PIDLEFT<kp>,<ki>,<kd>" tanpa ':'
    int firstComma = params.indexOf(',');
    int secondComma = params.indexOf(',', firstComma + 1);
    
    Serial.println("PIDLEFT Command Parsing:");
    Serial.println("Params: " + params);
    Serial.println("First comma at: " + String(firstComma));
    Serial.println("Second comma at: " + String(secondComma));
    
    if (firstComma > 0 && secondComma > firstComma) {
      double kp = params.substring(0, firstComma).toDouble();
      double ki = params.substring(firstComma + 1, secondComma).toDouble();
      double kd = params.substring(secondComma + 1).toDouble();
      
      Serial.println("Parsed Left Motor values - Kp: " + String(kp, 4) + " Ki: " + String(ki, 4) + " Kd: " + String(kd, 4));
      Serial.println("Validation check: " + String(kp >= 0 && kp <= 200 && ki >= 0 && ki <= 200 && kd >= 0 && kd <= 200));
      
      if (kp >= 0 && kp <= 200 && ki >= 0 && ki <= 200 && kd >= 0 && kd <= 200) {
        pidConfigLeft.kp = kp; 
        pidConfigLeft.ki = ki; 
        pidConfigLeft.kd = kd;
        savePIDParametersLeft();
        
        Serial1.println("PIDLEFT_SAVED:" + String(kp, 3) + "," + String(ki, 3) + "," + String(kd, 3));
        Serial.println("Left Motor PID saved successfully");
      } else {
        Serial.println("ERROR: Left Motor PID values out of allowed range!");
      }
    }
  } else if(command.startsWith("RPMSHOW") || command.startsWith("RS")) {
    // Show current motor speeds in sendMotorStatusToMaster format (RS = shortcut)
    // Serial1.println("RPMSHOW:" + String(rpm_depan_kanan, 3) + "," + String(rpm_depan_kiri, 3));
    // Serial1.printfln("RPMSHOW:" + rpm_depan_kanan + "," + rpm_depan_kiri;  
    Serial1.printf("RPMSHOW:%d,%d\n", rpm_depan_kanan,rpm_depan_kiri);  
    Serial.printf("RPMSHOW:%d,%d\n", rpm_depan_kanan,rpm_depan_kiri);  
    Serial.println("RPM values sent to master via Serial1");  // Debug message
  } else if (command.startsWith("AUTOTUNE") || command.startsWith("TUNE")) {
    // Auto-tuning command dari master
    if (!isTuningActive()) {
      startAutoTuning();
      Serial1.println("AUTOTUNE:STARTED");
      Serial.println("Auto-tuning dimulai atas permintaan master");
    } else {
      Serial1.println("AUTOTUNE:ALREADY_RUNNING");
      Serial.println("Auto-tuning sudah berjalan");
    }
  } else if (command.startsWith("RIGHT_TUNE")) {
    // Auto-tuning khusus motor kanan
    if (!isTuningActive()) {
      startAutoTuningRight();
      Serial1.println("AUTOTUNE_RIGHT:STARTED");
      Serial.println("Auto-tuning motor kanan dimulai atas permintaan master");
    } else {
      Serial1.println("AUTOTUNE:ALREADY_RUNNING");
      Serial.println("Auto-tuning sudah berjalan");
    }
  } else if (command.startsWith("LEFT_TUNE")) {
    // Auto-tuning khusus motor kiri
    if (!isTuningActive()) {
      startAutoTuningLeft();
      Serial1.println("AUTOTUNE_LEFT:STARTED");
      Serial.println("Auto-tuning motor kiri dimulai atas permintaan master");
    } else {
      Serial1.println("AUTOTUNE:ALREADY_RUNNING");
      Serial.println("Auto-tuning sudah berjalan");
    }
  } else if (command.startsWith("TUNECANCEL") || command.startsWith("CANCEL")) {
    // Cancel auto-tuning command dari master
    if (isTuningActive()) {
      cancelAutoTuning();
      Serial1.println("AUTOTUNE:CANCELLED");
      Serial.println("Auto-tuning dibatalkan atas permintaan master");
    } else {
      Serial1.println("AUTOTUNE:NOT_RUNNING");
      Serial.println("Tidak ada auto-tuning yang sedang berjalan");
    }
  } else if (command.startsWith("TUNESTATUS")) {
    // Status auto-tuning untuk master
    if (isTuningActive()) {
      int progress = getTuningProgress();
      Serial1.printf("AUTOTUNE:PROGRESS:%d\n", progress);
      Serial.printf("Auto-tuning progress: %d%%\n", progress);
    } else {
      Serial1.println("AUTOTUNE:IDLE");
      Serial.println("Auto-tuning tidak aktif");
    }
  } else if (command.startsWith("RPM")) {
    // RPM Motor command: RPM30,25 (kanan, kiri)
    String params = command.substring(3);
    int commaPos = params.indexOf(',');
    
    if (commaPos > 0) {
      float rpmKanan = params.substring(0, commaPos).toFloat();
      float rpmKiri = params.substring(commaPos + 1).toFloat();
      
      Serial.printf("[SLAVE] RPM Command - Kanan: %.1f RPM, Kiri: %.1f RPM\n", rpmKanan, rpmKiri);
      
      // Gunakan fungsi rpmMotor untuk kontrol RPM yang konsisten
      rpmMotor(rpmKanan, rpmKiri);  
      
      // Send acknowledgment back to master via Serial1
      Serial1.printf("RPM_ACK:%.1f,%.1f\n", rpmKanan, rpmKiri);
    } else {
      Serial.printf("[SLAVE] ERROR: Invalid RPM format: %s\n", command.c_str());
      Serial1.println("ERROR:INVALID_RPM_CMD");
    }

  }else if (command.startsWith("PIDSHOW") || command.startsWith("PS")) {
    sendPIDToMaster();
    Serial.println("PID values sent to master via Serial1");  // Debug message
  } else if (command.startsWith("STOP") || command.startsWith("S")) {
    // Emergency stop (S = shortcut)
    stopAllMotors();
    // Reset PID integrals
    pidData[0].integral = 0;
    pidData[0].error = 0;
    pidData[1].integral = 0;
    pidData[1].error = 0;
  } else if (command.startsWith("L") && command.indexOf("R") > 0) {
    // Motor command format L<val>R<val> (dari sendMotorCommand atau manual input)
    // Contoh: L100R-50, L0R0, L-200R300
    if (parseCommand(command, leftSpeed, rightSpeed)) {
      setMotorSpeed(1, leftSpeed);   // Motor kiri
      setMotorSpeed(2, rightSpeed);  // Motor kanan
      Serial.printf("Motor Command Received - Left: %d, Right: %d\n", leftSpeed, rightSpeed);
      
      // Kirim konfirmasi kembali ke master
      Serial1.println("MOTOR_SET:L" + String(leftSpeed) + "R" + String(rightSpeed));
    } else {
      Serial.println("ERROR: Invalid motor command format!");
      Serial1.println("ERROR:INVALID_MOTOR_CMD");
    }
  } else {
    // stopAllMotors();
    // // Reset PID integrals
    // pidData[0].integral = 0;
    // pidData[0].error = 0;
    // pidData[1].integral = 0;
    // pidData[1].error = 0;
  }
}

bool parseCommand(String command, int &leftSpeed, int &rightSpeed) {
  // Find L and R positions
  int lPos = command.indexOf('L');
  int rPos = command.indexOf('R');
  
  if (lPos == -1 || rPos == -1 || lPos >= rPos) {
    return false;
  }
  
  // Extract speeds
  String leftStr = command.substring(lPos + 1, rPos);
  String rightStr = command.substring(rPos + 1);
  
  leftSpeed = leftStr.toInt();
  rightSpeed = rightStr.toInt();
  
  // Constrain speeds to 12-bit PWM range (-4095 to 4095)
  leftSpeed = constrain(leftSpeed, -4095, 4095);
  rightSpeed = constrain(rightSpeed, -4095, 4095);
  
  return true;
}

// Commented out to avoid duplication with preferences.ino
// Function to send current PID parameters to AGV_SAMI on startup
// void sendPIDToMaster() {
//   Serial1.println("PIDVALUES:" + String(pidConfig.kp, 3) + "," + String(pidConfig.ki, 3) + "," + String(pidConfig.kd, 3));
//   Serial.println("PID values sent to master via Serial1");  // Debug message
// }
