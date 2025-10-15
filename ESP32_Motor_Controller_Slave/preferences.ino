void loadPIDParameters() {
  // Nilai default yang lebih realistis untuk RPM control
  const double DEFAULT_KP = 20.0;  // Proportional gain yang lebih tinggi untuk responsivitas
  const double DEFAULT_KI = 0.5;   // Integral gain yang wajar untuk eliminasi error steady-state
  const double DEFAULT_KD = 0.1;   // Derivative gain kecil untuk stabilitas
  
  // Open preferences in read-only mode
  preferences.begin("pid_config", true);

  // Load individual motor PID parameters
  pidConfigRight.kp = preferences.getDouble("kp_right", DEFAULT_KP);
  pidConfigRight.ki = preferences.getDouble("ki_right", DEFAULT_KI);
  pidConfigRight.kd = preferences.getDouble("kd_right", DEFAULT_KD);
  
  pidConfigLeft.kp = preferences.getDouble("kp_left", DEFAULT_KP);
  pidConfigLeft.ki = preferences.getDouble("ki_left", DEFAULT_KI);
  pidConfigLeft.kd = preferences.getDouble("kd_left", DEFAULT_KD);
  
  // Load general PID parameters (for auto-tuner backward compatibility)
  pidConfig.kp = preferences.getDouble("kp", DEFAULT_KP);
  pidConfig.ki = preferences.getDouble("ki", DEFAULT_KI);
  pidConfig.kd = preferences.getDouble("kd", DEFAULT_KD);
  
  preferences.end();
  
  // Debug output lebih detail
}

// Fungsi untuk menyimpan PID parameters motor kanan
void savePIDParametersRight() {
  // Batasi nilai PID untuk keamanan sebelum menyimpan
  pidConfigRight.kp = constrain(pidConfigRight.kp, 0.0, 200.0);
  pidConfigRight.ki = constrain(pidConfigRight.ki, 0.0, 200.0);
  pidConfigRight.kd = constrain(pidConfigRight.kd, 0.0, 200.0);

  // Open preferences in write mode
  preferences.begin("pid_config", false);
  
  delay(50); // Berikan waktu untuk flash operation
  
  // Save Right Motor PID parameters
  preferences.putDouble("kp_right", pidConfigRight.kp);
  preferences.putDouble("ki_right", pidConfigRight.ki);
  preferences.putDouble("kd_right", pidConfigRight.kd);

  // Flush untuk memastikan data ditulis ke flash
  preferences.end();

  Serial.println("Right Motor PID saved to flash: Kp=" + String(pidConfigRight.kp, 4) + ", Ki=" + String(pidConfigRight.ki, 4) + ", Kd=" + String(pidConfigRight.kd, 4));
}

// Fungsi untuk menyimpan PID parameters motor kiri
void savePIDParametersLeft() {
  // Batasi nilai PID untuk keamanan sebelum menyimpan
  pidConfigLeft.kp = constrain(pidConfigLeft.kp, 0.0, 200.0);
  pidConfigLeft.ki = constrain(pidConfigLeft.ki, 0.0, 200.0);
  pidConfigLeft.kd = constrain(pidConfigLeft.kd, 0.0, 200.0);

  // Open preferences in write mode
  preferences.begin("pid_config", false);
  
  delay(50); // Berikan waktu untuk flash operation
  
  // Save Left Motor PID parameters
  preferences.putDouble("kp_left", pidConfigLeft.kp);
  preferences.putDouble("ki_left", pidConfigLeft.ki);
  preferences.putDouble("kd_left", pidConfigLeft.kd);

  // Flush untuk memastikan data ditulis ke flash
  preferences.end();

  Serial.println("Left Motor PID saved to flash: Kp=" + String(pidConfigLeft.kp, 4) + ", Ki=" + String(pidConfigLeft.ki, 4) + ", Kd=" + String(pidConfigLeft.kd, 4));
}

// Fungsi untuk menyimpan PID parameters umum (untuk auto-tuner backward compatibility)
void savePIDParameters() {
  // Batasi nilai PID untuk keamanan sebelum menyimpan
  pidConfig.kp = constrain(pidConfig.kp, 0.0, 200.0);
  pidConfig.ki = constrain(pidConfig.ki, 0.0, 200.0);
  pidConfig.kd = constrain(pidConfig.kd, 0.0, 200.0);

  // Open preferences in write mode
  preferences.begin("pid_config", false);
  
  delay(50); // Berikan waktu untuk flash operation
  
  // Save General PID parameters (for auto-tuner)
  preferences.putDouble("kp", pidConfig.kp);
  preferences.putDouble("ki", pidConfig.ki);
  preferences.putDouble("kd", pidConfig.kd);

  // Flush untuk memastikan data ditulis ke flash
  preferences.end();

  Serial.println("General PID saved to flash: Kp=" + String(pidConfig.kp, 4) + ", Ki=" + String(pidConfig.ki, 4) + ", Kd=" + String(pidConfig.kd, 4));
}

// Fungsi untuk reset PID parameters ke nilai default
void resetPIDParameters() {
  // Reset PID parameters dengan nilai yang lebih realistis untuk kedua motor
  pidConfigRight.kp = 20.0;  // Nilai awal yang realistic untuk motor
  pidConfigRight.ki = 0.05;  // Nilai yang adequate untuk integral
  pidConfigRight.kd = 0.01;  // Sedikit damping untuk stabilitas
  
  pidConfigLeft.kp = 20.0;   // Sama untuk motor kiri - nilai realistis
  pidConfigLeft.ki = 0.05;
  pidConfigLeft.kd = 0.01;
  
  // Save ke flash memory untuk kedua motor
  savePIDParametersRight();
  savePIDParametersLeft();
  
  Serial.println("PID Parameters reset to default values for both motors");
  Serial.println("Right Motor - KP: " + String(pidConfigRight.kp, 4) + 
                 ", KI: " + String(pidConfigRight.ki, 4) + 
                 ", KD: " + String(pidConfigRight.kd, 4));
  Serial.println("Left Motor - KP: " + String(pidConfigLeft.kp, 4) + 
                 ", KI: " + String(pidConfigLeft.ki, 4) + 
                 ", KD: " + String(pidConfigLeft.kd, 4));
}

// Fungsi untuk reset spesifik PID controller
void resetPID(int index) {
  if (index >= 0 && index < numOutputs) {
    pidData[index].error = 0;
    pidData[index].integral = 0;
    pidData[index].previousError = 0;
    pidData[index].derivative = 0;
    Serial.printf("Reset PID controller %d\n", index);
  }
}

// Fungsi untuk kirim PID parameters ke master ESP32
void sendPIDToMaster() {
  // Kirim PID untuk motor kanan
  String pidRightString = "PIDRIGHT_VALUES:" + String(pidConfigRight.kp, 3) + "," + 
                         String(pidConfigRight.ki, 3) + "," + String(pidConfigRight.kd, 3);
  
  // Kirim PID untuk motor kiri
  String pidLeftString = "PIDLEFT_VALUES:" + String(pidConfigLeft.kp, 3) + "," + 
                        String(pidConfigLeft.ki, 3) + "," + String(pidConfigLeft.kd, 3);
  
  // Kirim kedua command ke master
  Serial1.println(pidRightString);
  delay(100); // Delay kecil antar pengiriman
  Serial1.println(pidLeftString);
  
  Serial.println("Sent Right Motor PID to master: " + pidRightString);
  Serial.println("Sent Left Motor PID to master: " + pidLeftString);
}