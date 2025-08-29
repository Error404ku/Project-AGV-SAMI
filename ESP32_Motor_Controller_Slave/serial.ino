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

  // Reset watchdog timer jika ada aktivitas serial untuk mencegah timeout.
  if (charsRead > 0) {
    esp_task_wdt_reset();
  }
}

void processCommand(String command) {
  command.trim();  // Remove whitespace
  Serial.println("Received command: " + command);  // Debug message
  
  if (command.startsWith("PID")) {  // TERAKHIR untuk set nilai PID
    String params = command.substring(3); // harus "PID<kp>,<ki>,<kd>" tanpa ':'
    int firstComma = params.indexOf(',');
    int secondComma = params.indexOf(',', firstComma + 1);
    
    // Debug info tambahan
    Serial.println("PID Command Parsing:");
    Serial.println("Params: " + params);
    Serial.println("First comma at: " + String(firstComma));
    Serial.println("Second comma at: " + String(secondComma));
    
    if (firstComma > 0 && secondComma > firstComma) {
      double kp = params.substring(0, firstComma).toDouble();
      double ki = params.substring(firstComma + 1, secondComma).toDouble();
      double kd = params.substring(secondComma + 1).toDouble();
      
      // Debug info tambahan
      Serial.println("Parsed values - Kp: " + String(kp, 4) + " Ki: " + String(ki, 4) + " Kd: " + String(kd, 4));
      Serial.println("Validation check: " + String(kp >= 0 && kp <= 100 && ki >= 0 && ki <= 10 && kd >= 0 && kd <= 10));
      
      if (kp >= 0 && kp <= 100 && ki >= 0 && ki <= 10 && kd >= 0 && kd <= 10) {
        pidConfig.kp = kp; pidConfig.ki = ki; pidConfig.kd = kd;
        savePIDParameters();
        
        // Tambahkan echo balik ke master untuk konfirmasi
        Serial1.println("PID_SAVED:" + String(kp, 3) + "," + String(ki, 3) + "," + String(kd, 3));
      } else {
        Serial.println("ERROR: PID values out of allowed range!");
      }

    }
  } else if(command.startsWith("RPMSHOW") || command.startsWith("RS")) {
    // Show current motor speeds in sendMotorStatusToMaster format (RS = shortcut)
    Serial1.println("RPMSHOW:" + String(rpm_depan_kanan, 1) + "," + String(rpm_depan_kiri, 1));
    Serial.println("RPM values sent to master via Serial1");  // Debug message
  } else if (command.startsWith("RPM")) {
    // RPM Motor command: RPM30,25 (kanan, kiri)
    String params = command.substring(3);
    int commaPos = params.indexOf(',');
    
    if (commaPos > 0) {
      int rpmKanan = params.substring(0, commaPos).toInt();
      int rpmKiri = params.substring(commaPos + 1).toInt();
      
      Serial.printf("RPM Command Received - Kanan: %d RPM, Kiri: %d RPM\n", rpmKanan, rpmKiri);
      
      // Gunakan fungsi setMotorSpeedRPM untuk kontrol yang konsisten
      rpmMotor(rpmKanan, rpmKiri);  
    } else {
      Serial.println("ERROR: Invalid RPM command format! Use RPM<kanan>,<kiri>");
      Serial1.println("ERROR:INVALID_RPM_CMD");
    }

  }else if (command.startsWith("PIDSHOW") || command.startsWith("PS")) {
    // Show current PID parameters in sendPIDToMaster format (PS = shortcut)
    Serial1.println("PIDVALUES:" + String(pidConfig.kp, 3) + "," + String(pidConfig.ki, 3) + "," + String(pidConfig.kd, 3));
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
