
// PID RPM Control Function untuk 2 motor
void rpmMotor(float rpm1, float rpm2) {
  // Gunakan PID parameters dari preferences
  double kp = pidConfig.kp;
  double ki = pidConfig.ki;
  double kd = pidConfig.kd;

  // Hitung integral limits berdasarkan Ki - dengan safety check
  double minintegral, maxintegral;
  if (ki > 0.001) {  // Prevent division by very small numbers
    minintegral = -4000.0/ki;
    maxintegral = 4000.0/ki;
  } else {
    minintegral = -1000.0;  // Safe fallback values
    maxintegral = 1000.0;
  }

  // Constrain RPM to valid range
  rpm1 = constrain(rpm1, minrpm, maxrpm);
  rpm2 = constrain(rpm2, minrpm, maxrpm);
  
  // Debug outputcomputePID
  
  // ===== MOTOR KANAN (Motor 1) PID Control =====
  if (rpm1 > 0) {
    // Forward direction for motor kanan
    pwmKanan = computePID(0, rpm1, rpm_depan_kanan, kp, ki, kd, minintegral, maxintegral);
    pwmKanan = constrain(pwmKanan, pwm_zero, pwm_max);
    
  } else if (rpm1 < 0) {
    // Reverse direction for motor kanan
    pwmKanan = computePID(1, rpm1, rpm_depan_kanan, kp, ki, kd, minintegral, maxintegral);
    // pwmKanan = -pwmKanan;  // Make negative for reverse
    pwmKanan = constrain(pwmKanan, pwm_min, pwm_zero);
    
  } else if (rpm1 == 0) {
    // Stop motor kanan
    pwmKanan = 0;
    pidData[0].error = 0;
    pidData[0].integral = 0;
  }
  
  // ===== MOTOR KIRI (Motor 2) PID Control =====
  if (rpm2 > 0) {
    // Forward direction for motor kiri
    pwmKiri = computePID(2, rpm2, rpm_depan_kiri, kp, ki, kd, minintegral, maxintegral);
    pwmKiri = constrain(pwmKiri, pwm_zero, pwm_max);
    
  } else if (rpm2 < 0) {
    // Reverse direction for motor kiri
    pwmKiri = computePID(3, rpm2, rpm_depan_kiri, kp, ki, kd, minintegral, maxintegral);
    // pwmKiri = -pwmKiri;  // Make negative for reverse
    pwmKiri = constrain(pwmKiri, pwm_min,pwm_zero);
    
  } else if (rpm2 == 0) {
    // Stop motor kiri
    pwmKiri = 0;
    pidData[1].error = 0;
    pidData[1].integral = 0;
  }
  
  // Apply PWM values to motors
  Serial.printf("Error RPM (double) - Kanan: %.2f, Kiri: %.2f\n", pidData[0].error, pidData[1].error);
  // Validasi akhir PWM output
  pwmKanan = constrain(pwmKanan, pwm_min, pwm_max);
  pwmKiri = constrain(pwmKiri, pwm_min, pwm_max);
  
  setMotorSpeed(1, pwmKiri);  // Motor 1 = Kanan
  setMotorSpeed(2, pwmKanan);   // Motor 2 = Kiri

}

// Helper function untuk set target RPM dengan validasi
void setTargetRPM(int rpmKanan, int rpmKiri) {
  // Validasi input RPM
  rpmKanan = constrain(rpmKanan, -maxrpm, maxrpm);
  rpmKiri = constrain(rpmKiri, -maxrpm, maxrpm);
  
  // Panggil PID RPM control
  rpmMotor(rpmKanan, rpmKiri);
}

// Function untuk stop semua motor dan reset PID
void stopAllMotorsRPM() {
  rpmMotor(0, 0);
  
  // Reset PID data
  for (int i = 0; i < numOutputs; i++) {
    pidData[i].error = 0;
    pidData[i].integral = 0;
    pidData[i].previousError = 0;
    pidData[i].derivative = 0;
  }
  
  Serial.println("All motors stopped and PID reset");
}
