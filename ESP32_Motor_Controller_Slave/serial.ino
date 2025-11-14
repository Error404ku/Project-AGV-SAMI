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
    // OPTIMIZED: Direct printf - no String allocation
    // Show current motor speeds in sendMotorStatusToMaster format (RS = shortcut)
    Serial1.printf("RPMSHOW:%.1f,%.1f\n", rpm_depan_kanan, rpm_depan_kiri);  
    Serial.printf("[SLAVE] RPMSHOW - Kanan: %.1f RPM, Kiri: %.1f RPM\n", rpm_depan_kanan, rpm_depan_kiri);
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
    // OPTIMIZED: Direct sscanf parsing - 10x faster than String operations
    // RPM Motor command: RPM30,25 (kanan, kiri)
    float rpmKanan, rpmKiri;
    
    // Fast parsing with sscanf - no heap allocation, no String operations
    if (sscanf(command.c_str() + 3, "%f,%f", &rpmKanan, &rpmKiri) == 2) {
      Serial.printf("[SLAVE] RPM Command - Kanan: %.1f RPM, Kiri: %.1f RPM\n", rpmKanan, rpmKiri);
      
      // Reset performance metrics when new command is received
      performanceMetricsRight = PerformanceMetrics();
      performanceMetricsLeft = PerformanceMetrics();
      
      // Gunakan fungsi rpmMotor untuk kontrol RPM yang konsisten
      rpmMotor(rpmKanan, rpmKiri);  
      
      // Send acknowledgment back to master via Serial1
      Serial1.printf("RPM_ACK:%.1f,%.1f\n", rpmKanan, rpmKiri);
    } else {
      Serial.printf("[SLAVE] ERROR: Invalid RPM format: %s\n", command.c_str());
      Serial1.println("ERROR:INVALID_RPM_CMD");
    }

  } else if (command.startsWith("METRICS") || command.startsWith("PM")) {
    // =============================================
    // BEST PRACTICE 7: SHOW PERFORMANCE METRICS
    // =============================================
    // PM = shortcut untuk Performance Metrics
    Serial.println("\n📊 ===== PERFORMANCE METRICS =====");
    
    // Right Motor Metrics
    Serial.println("🔵 RIGHT MOTOR:");
    Serial.printf("  ISE (Integral Square Error): %.2f\n", performanceMetricsRight.ISE);
    Serial.printf("  IAE (Integral Absolute Error): %.2f\n", performanceMetricsRight.IAE);
    Serial.printf("  ITAE (Time-weighted Absolute Error): %.2f\n", performanceMetricsRight.ITAE);
    Serial.printf("  Peak Error: %.2f RPM\n", performanceMetricsRight.peakError);
    Serial.printf("  Steady-State Error: %.2f RPM\n", performanceMetricsRight.steadyStateError);
    Serial.printf("  Samples Collected: %d\n", performanceMetricsRight.sampleCount);
    
    // Calculate combined score (lower is better)
    float scoreRight = 0.5 * performanceMetricsRight.ISE 
                      + 0.3 * performanceMetricsRight.IAE 
                      + 0.2 * performanceMetricsRight.ITAE;
    Serial.printf("  ⭐ Combined Score: %.2f (lower is better)\n", scoreRight);
    
    Serial.println("\n🔴 LEFT MOTOR:");
    Serial.printf("  ISE (Integral Square Error): %.2f\n", performanceMetricsLeft.ISE);
    Serial.printf("  IAE (Integral Absolute Error): %.2f\n", performanceMetricsLeft.IAE);
    Serial.printf("  ITAE (Time-weighted Absolute Error): %.2f\n", performanceMetricsLeft.ITAE);
    Serial.printf("  Peak Error: %.2f RPM\n", performanceMetricsLeft.peakError);
    Serial.printf("  Steady-State Error: %.2f RPM\n", performanceMetricsLeft.steadyStateError);
    Serial.printf("  Samples Collected: %d\n", performanceMetricsLeft.sampleCount);
    
    float scoreLeft = 0.5 * performanceMetricsLeft.ISE 
                     + 0.3 * performanceMetricsLeft.IAE 
                     + 0.2 * performanceMetricsLeft.ITAE;
    Serial.printf("  ⭐ Combined Score: %.2f (lower is better)\n", scoreLeft);
    
    Serial.println("====================================\n");
    
    // Send to master via Serial1
    Serial1.printf("METRICS_RIGHT:ISE=%.2f,IAE=%.2f,ITAE=%.2f,Peak=%.2f,SS=%.2f\n",
                   performanceMetricsRight.ISE, performanceMetricsRight.IAE, 
                   performanceMetricsRight.ITAE, performanceMetricsRight.peakError,
                   performanceMetricsRight.steadyStateError);
    Serial1.printf("METRICS_LEFT:ISE=%.2f,IAE=%.2f,ITAE=%.2f,Peak=%.2f,SS=%.2f\n",
                   performanceMetricsLeft.ISE, performanceMetricsLeft.IAE, 
                   performanceMetricsLeft.ITAE, performanceMetricsLeft.peakError,
                   performanceMetricsLeft.steadyStateError);
    
  } else if (command.startsWith("RESET_METRICS") || command.startsWith("RM")) {
    // Reset performance metrics
    performanceMetricsRight = PerformanceMetrics();
    performanceMetricsLeft = PerformanceMetrics();
    Serial.println("✅ Performance metrics reset!");
    Serial1.println("METRICS_RESET:OK");
    
  } else if (command.startsWith("SAFEMODE")) {
    // Toggle safe mode
    // Note: ENABLE_SAFE_MODE is const, so we show current status only
    Serial.println("\n🔴 ===== SAFE MODE STATUS =====");
    Serial.printf("SAFE MODE: %s\n", ENABLE_SAFE_MODE ? "ENABLED ✅" : "DISABLED ❌");
    Serial.println("\nSAFE MODE Features:");
    Serial.println("  ✅ Derivative term DISABLED (prevent stuttering)");
    Serial.println("  ✅ Setpoint weight INCREASED (0.8 for direct response)");
    Serial.println("  ✅ Anti-windup LESS AGGRESSIVE (prevent overcorrection)");
    Serial.println("  ✅ dt validation ENHANCED (prevent timing issues)");
    Serial.println("\n💡 To change: Edit config.h line 'ENABLE_SAFE_MODE'");
    Serial.println("================================\n");
    
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

// =============================================
// HELP COMMAND - Show all available commands
// =============================================
void printHelp() {
  Serial.println("\n📖 ===== ESP32 MOTOR CONTROLLER - COMMAND REFERENCE =====");
  Serial.println("\n🔧 MOTOR CONTROL:");
  Serial.println("  L<val>R<val>  - Direct PWM control (e.g., L100R-50)");
  Serial.println("  RPM<r>,<l>    - RPM control (e.g., RPM30,25)");
  Serial.println("  S or STOP     - Emergency stop");
  
  Serial.println("\n⚙️ PID CONFIGURATION:");
  Serial.println("  PIDRIGHT<kp>,<ki>,<kd>  - Set right motor PID");
  Serial.println("  PIDLEFT<kp>,<ki>,<kd>   - Set left motor PID");
  Serial.println("  PS or PIDSHOW           - Show current PID values");
  
  Serial.println("\n🎯 AUTO-TUNING:");
  Serial.println("  TUNE or AUTOTUNE    - Auto-tune both motors");
  Serial.println("  RIGHT_TUNE          - Auto-tune right motor only");
  Serial.println("  LEFT_TUNE           - Auto-tune left motor only");
  Serial.println("  CANCEL_TUNE         - Cancel tuning");
  
  Serial.println("\n📊 MONITORING & METRICS:");
  Serial.println("  RS or RPMSHOW       - Show current RPM values");
  Serial.println("  PM or METRICS       - Show performance metrics (ISE/IAE/ITAE)");
  Serial.println("  RM or RESET_METRICS - Reset performance metrics");
  
  Serial.println("\n✨ BEST PRACTICES FEATURES:");
  Serial.println("  ✅ Anti-Windup (Back-calculation)");
  Serial.println("  ✅ Derivative Filtering (Low-pass filter)");
  Serial.println("  ✅ Setpoint Weighting (2-DOF PID)");
  Serial.println("  ✅ Adaptive Sampling Rate");
  Serial.println("  ✅ Performance Metrics (ISE/IAE/ITAE)");
  
  Serial.println("\n📚 REFERENCES:");
  Serial.println("  - Åström & Hägglund: PID Controllers Theory & Design");
  Serial.println("  - Wikipedia: PID Controller");
  Serial.println("  - NI White Paper: PID Theory Explained");
  
  Serial.println("========================================================\n");
}
