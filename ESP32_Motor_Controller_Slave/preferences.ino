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
  preferences.begin("pid_config", false);
  
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
  Serial.println("PID Parameters saved to flash:" + String(tempKp, 4) + "," + String(tempKi, 4) + "," + String(tempKd, 4));
  Serial.println("PIDVALUES:" + String(pidConfig.kp, 3) + "," + String(pidConfig.ki, 3) + "," + String(pidConfig.kd, 3));
}
