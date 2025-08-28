void loadPIDParameters() {
  // Nilai default yang konsisten untuk digunakan di seluruh kode
  const double DEFAULT_KP = 1.0;  // Konsisten dengan resetPIDParameters
  const double DEFAULT_KI = 0.15;
  const double DEFAULT_KD = 0.0;
  
  // Open preferences in read-only mode
  preferences.begin("pid_config", true);
  
  // Load PID parameters dengan default values jika tidak ada
  pidConfig.kp = preferences.getDouble("kp", DEFAULT_KP);
  pidConfig.ki = preferences.getDouble("ki", DEFAULT_KI);
  pidConfig.kd = preferences.getDouble("kd", DEFAULT_KD);
  
  preferences.end();
  
  // Debug output lebih detail
  Serial.println("PID Parameters loaded from flash:" + String(pidConfig.kp, 4) + "," + String(pidConfig.ki, 4) + "," + String(pidConfig.kd, 4));
}

void savePIDParameters() {
  static double tempKp = pidConfig.kp;
  static double tempKi = pidConfig.ki;
  static double tempKd = pidConfig.kd;

  // Open preferences in write mode
  if (!preferences.begin("pid_config", false)) {
    Serial.println("ERROR: Failed to open preferences for writing");
    return;
  }
  
  delay(50);
  
  // Save PID parameters
  tempKp = preferences.putDouble("kp", tempKp);
  tempKi = preferences.putDouble("ki", tempKi);
  tempKd = preferences.putDouble("kd", tempKd);

  // Flush untuk memastikan data ditulis ke flash
  preferences.end();

  tempKp = pidConfig.kp;
  tempKi = pidConfig.ki;
  tempKd = pidConfig.kd;
}

void resetPIDParameters() {
  // Reset to default values - harus konsisten dengan default di loadPIDParameters
  const double DEFAULT_KP = 1.0;
  const double DEFAULT_KI = 0.15;
  const double DEFAULT_KD = 0.0;
  
  // Set nilai default
  pidConfig.kp = DEFAULT_KP;
  pidConfig.ki = DEFAULT_KI;
  pidConfig.kd = DEFAULT_KD;
  
  // Save default values
  savePIDParameters();
  
  // Debug output
  Serial.println("PID Parameters reset to defaults:");
  Serial.printf("  Kp: %.4f, Ki: %.4f, Kd: %.4f\n", 
                pidConfig.kp, pidConfig.ki, pidConfig.kd);
                
  // Kirim konfirmasi ke AGV SAMI bahwa nilai telah direset
  Serial1.println("PIDVALUES:" + String(pidConfig.kp, 3) + "," + 
                 String(pidConfig.ki, 3) + "," + String(pidConfig.kd, 3));
  Serial.println("Reset PID values sent to master via Serial1");
}
