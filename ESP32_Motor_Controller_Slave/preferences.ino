void loadPIDParameters() {
  // Nilai default yang lebih realistis untuk RPM control
  const double DEFAULT_KP = 20.0;  // Proportional gain yang lebih tinggi untuk responsivitas
  const double DEFAULT_KI = 0.5;   // Integral gain yang wajar untuk eliminasi error steady-state
  const double DEFAULT_KD = 0.1;   // Derivative gain kecil untuk stabilitas
  
  // Open preferences in read-only mode
  preferences.begin("pid_config", true);
  
  // Load PID parameters dengan default values jika tidak ada
  pidConfig.kp = preferences.getDouble("kp", DEFAULT_KP);
  pidConfig.ki = preferences.getDouble("ki", DEFAULT_KI);
  pidConfig.kd = preferences.getDouble("kd", DEFAULT_KD);
  
  preferences.end();
  
  // Batasi nilai-nilai PID untuk keamanan
  pidConfig.kp = constrain(pidConfig.kp, 0.0, 100.0);
  pidConfig.ki = constrain(pidConfig.ki, 0.0, 50.0);
  pidConfig.kd = constrain(pidConfig.kd, 0.0, 50.0);
  
  // Debug output lebih detail
  Serial.println("PID Parameters loaded from flash:" + String(pidConfig.kp, 4) + "," + String(pidConfig.ki, 4) + "," + String(pidConfig.kd, 4));
}

void savePIDParameters() {
  // Batasi nilai PID untuk keamanan sebelum menyimpan
  pidConfig.kp = constrain(pidConfig.kp, 0.0, 100.0);
  pidConfig.ki = constrain(pidConfig.ki, 0.0, 50.0);
  pidConfig.kd = constrain(pidConfig.kd, 0.0, 50.0);

  // Open preferences in write mode
  preferences.begin("pid_config", false);
  
  delay(50); // Berikan waktu untuk flash operation
  
  // Save PID parameters
  preferences.putDouble("kp", pidConfig.kp);
  preferences.putDouble("ki", pidConfig.ki);
  preferences.putDouble("kd", pidConfig.kd);

  // Flush untuk memastikan data ditulis ke flash
  preferences.end();

  Serial.println("PID Parameters saved to flash:" + String(pidConfig.kp, 4) + "," + String(pidConfig.ki, 4) + "," + String(pidConfig.kd, 4));
  Serial.println("PIDVALUES:" + String(pidConfig.kp, 3) + "," + String(pidConfig.ki, 3) + "," + String(pidConfig.kd, 3));
}

// Fungsi untuk reset PID parameters ke nilai default
void resetPIDParameters() {
  // Reset PID parameters dengan nilai konservatif yang lebih aman
  pidConfig.kp = 0.5;  // Nilai lebih kecil untuk kp
  pidConfig.ki = 0.01; // Nilai lebih kecil untuk ki
  pidConfig.kd = 0.0;  // Biasanya kd tidak dibutuhkan untuk kontrol motor sederhana
  
  // Save ke flash memory
  savePIDParameters();
  
  Serial.println("PID Parameters reset to default values");
  Serial.println("KP: " + String(pidConfig.kp, 4) + 
                 ", KI: " + String(pidConfig.ki, 4) + 
                 ", KD: " + String(pidConfig.kd, 4));
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
  // Format string: PIDVALUES:KP,KI,KD (sesuai dengan format penerimaan di master)
  String pidString = "PIDVALUES:" + String(pidConfig.kp, 3) + "," + 
                    String(pidConfig.ki, 3) + "," + String(pidConfig.kd, 3);
  
  // Kirim ke master
  Serial1.println(pidString);
  Serial.println("Sent PID to master: " + pidString);
}
