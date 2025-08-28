void loadPIDParameters() {
  // Nilai default yang konsisten untuk digunakan di seluruh kode
  const double DEFAULT_KP = 1.0;  // Konsisten dengan resetPIDParameters
  const double DEFAULT_KI = 0.15;
  const double DEFAULT_KD = 0.0;
  
  // Open preferences in read-only mode
  if (!preferences.begin("pid_config", true)) {
    Serial.println("ERROR: Failed to open preferences for reading");
    // Set default values jika tidak bisa membaca
    pidConfig.kp = DEFAULT_KP;
    pidConfig.ki = DEFAULT_KI;
    pidConfig.kd = DEFAULT_KD;
    Serial.println("Using default PID values due to preferences error");
    return;
  }
  
  delay(50);
  
  // Check if PID parameters exist
  bool hasKp = preferences.isKey("kp");
  bool hasKi = preferences.isKey("ki");
  bool hasKd = preferences.isKey("kd");
  
  // Load PID parameters dengan default values jika tidak ada
  pidConfig.kp = preferences.getDouble("kp", DEFAULT_KP);
  pidConfig.ki = preferences.getDouble("ki", DEFAULT_KI);
  pidConfig.kd = preferences.getDouble("kd", DEFAULT_KD);
  
  preferences.end();
  
  // Debug output lebih detail
  Serial.println("PID Parameters loaded from flash:");
  if (hasKp) {
    Serial.printf("  Kp: %.4f (stored in flash)\n", pidConfig.kp);
  } else {
    Serial.printf("  Kp: %.4f (using default, not found in flash)\n", pidConfig.kp);
  }
  
  if (hasKi) {
    Serial.printf("  Ki: %.4f (stored in flash)\n", pidConfig.ki);
  } else {
    Serial.printf("  Ki: %.4f (using default, not found in flash)\n", pidConfig.ki);
  }
  
  if (hasKd) {
    Serial.printf("  Kd: %.4f (stored in flash)\n", pidConfig.kd);
  } else {
    Serial.printf("  Kd: %.4f (using default, not found in flash)\n", pidConfig.kd);
  }
}

void savePIDParameters() {
  // Open preferences in write mode
  if (!preferences.begin("pid_config", false)) {
    Serial.println("ERROR: Failed to open preferences for writing");
    return;
  }
  
  delay(50);
  
  // Save PID parameters
  preferences.putDouble("kp", pidConfig.kp);
  preferences.putDouble("ki", pidConfig.ki);
  preferences.putDouble("kd", pidConfig.kd);
  
  // Flush untuk memastikan data ditulis ke flash
  preferences.end();
  
  // Verifikasi penyimpanan - buka kembali preferences dan baca nilai
  preferences.begin("pid_config", true);
  double verifyKp = preferences.getDouble("kp", -1.0);
  double verifyKi = preferences.getDouble("ki", -1.0);
  double verifyKd = preferences.getDouble("kd", -1.0);
  preferences.end();
  
  // Tampilkan hasil verifikasi
  Serial.println("PID Parameters saved to flash memory:");
  Serial.printf("  Kp: %.4f (Verified: %.4f)\n", pidConfig.kp, verifyKp);
  Serial.printf("  Ki: %.4f (Verified: %.4f)\n", pidConfig.ki, verifyKi);
  Serial.printf("  Kd: %.4f (Verified: %.4f)\n", pidConfig.kd, verifyKd);
  
  // Periksa apakah nilai tersimpan dengan benar
  if (verifyKp != pidConfig.kp || verifyKi != pidConfig.ki || verifyKd != pidConfig.kd) {
    Serial.println("WARNING: PID parameter verification failed! Values may not be saved correctly.");
  }
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
