// =============================================
// RPM MOTOR CONTROL with ADAPTIVE SAMPLING & PERFORMANCE METRICS
// =============================================
// Best Practices Applied:
// 1. Adaptive Sampling Rate (adjust based on transient vs steady-state)
// 2. Performance Metrics Collection (ISE, IAE, ITAE)

// PID RPM Control Function untuk 2 motor
void rpmMotor(float rpm1, float rpm2) {
  // Gunakan PID parameters dari preferences
  double kpRight = pidConfigRight.kp;
  double kiRight = pidConfigRight.ki;
  double kdRight = pidConfigRight.kd;

  double kpLeft = pidConfigLeft.kp;
  double kiLeft = pidConfigLeft.ki;
  double kdLeft = pidConfigLeft.kd;
  
  // ✅ FIXED: Gunakan integral limits yang SAMA seperti saat tuning
  // Integral limits harus CUKUP BESAR untuk Ki bekerja optimal
  // Auto-tuner menggunakan range yang lebih besar untuk performa terbaik
  double minintegralRight = -2000.0;  // Fixed range untuk konsistensi
  double maxintegralRight = 2000.0;
  
  double minintegralLeft = -2000.0;   // Fixed range untuk konsistensi
  double maxintegralLeft = 2000.0;

  // Constrain RPM to valid range
  rpm1 = constrain(rpm1, minrpm, maxrpm);
  rpm2 = constrain(rpm2, minrpm, maxrpm);
  
  // =============================================
  // BEST PRACTICE 5: ADAPTIVE SAMPLING RATE
  // =============================================
  // Reference: Control Theory - "Sampling rate should adapt to system dynamics"
  // Fast sampling during transients, slower during steady-state (saves CPU)
  
  static unsigned long lastAdaptiveUpdate = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastAdaptiveUpdate >= 500) {  // Update every 500ms
    // Calculate RPM difference (error magnitude)
    int rpmDiffRight = abs((int)rpm1 - rpm_depan_kanan);
    int rpmDiffLeft = abs((int)rpm2 - rpm_depan_kiri);
    int maxRpmDiff = max(rpmDiffRight, rpmDiffLeft);
    
    // Adaptive sampling logic
    if (maxRpmDiff > 10) {
      // Large error - FAST sampling for quick response
      adaptiveSampling.currentInterval = adaptiveSampling.minInterval;
    } else if (maxRpmDiff < 3) {
      // Near steady-state - SLOW sampling to save CPU
      adaptiveSampling.currentInterval = adaptiveSampling.maxInterval;
    } else {
      // Medium transient - MEDIUM sampling
      adaptiveSampling.currentInterval = 100;
    }
    
    adaptiveSampling.lastRPMDifference = maxRpmDiff;
    lastAdaptiveUpdate = currentTime;
    
    // Debug adaptive sampling
    Serial.printf("📊 Adaptive Sampling: RPM_diff=%d, Interval=%lu ms\n", 
                  maxRpmDiff, adaptiveSampling.currentInterval);
  }
  
  // =============================================
  // BEST PRACTICE 7: PERFORMANCE METRICS COLLECTION
  // =============================================
  // Reference: "Optimal Control" - ISE, IAE, ITAE standard indices
  
  static unsigned long metricsStartTime = 0;
  if (metricsStartTime == 0) {
    metricsStartTime = millis();
  }
  
  // Calculate time elapsed (in seconds)
  double elapsedTime = (currentTime - metricsStartTime) / 1000.0;
  double dt = 0.1;  // Approximate sampling time in seconds
  
  // Calculate errors
  double errorRight = rpm1 - rpm_depan_kanan;
  double errorLeft = rpm2 - rpm_depan_kiri;
  
  // Update metrics for right motor
  performanceMetricsRight.ISE += errorRight * errorRight * dt;  // Integral Square Error
  performanceMetricsRight.IAE += abs(errorRight) * dt;          // Integral Absolute Error
  performanceMetricsRight.ITAE += elapsedTime * abs(errorRight) * dt;  // Time-weighted
  performanceMetricsRight.peakError = max(performanceMetricsRight.peakError, (float)abs(errorRight));
  performanceMetricsRight.steadyStateError = errorRight;
  performanceMetricsRight.sampleCount++;
  
  // Update metrics for left motor
  performanceMetricsLeft.ISE += errorLeft * errorLeft * dt;
  performanceMetricsLeft.IAE += abs(errorLeft) * dt;
  performanceMetricsLeft.ITAE += elapsedTime * abs(errorLeft) * dt;
  performanceMetricsLeft.peakError = max(performanceMetricsLeft.peakError, (float)abs(errorLeft));
  performanceMetricsLeft.steadyStateError = errorLeft;
  performanceMetricsLeft.sampleCount++;
  
  // Debug outputcomputePID

  
  // ===== MOTOR KANAN (Motor 1) PID Control =====
  if (rpm1 > 0) {
    // Forward direction for motor kanan
    pwmKanan = computePID(0, rpm1, rpm_depan_kanan, kpRight, kiRight, kdRight, minintegralRight, maxintegralRight);
    // Untuk target positif, pastikan PWM tidak negatif (tidak mundur)
    pwmKanan = constrain(pwmKanan, 0, pwm_max);
    
  } else if (rpm1 < 0) {
    // Reverse direction for motor kanan
    pwmKanan = computePID(0, rpm1, rpm_depan_kanan, kpRight, kiRight, kdRight, minintegralRight, maxintegralRight);
    // Untuk target negatif, pastikan PWM tidak positif (tidak maju)
    pwmKanan = constrain(pwmKanan, pwm_min, 0);
    
  } else if (rpm1 == 0) {
    // Stop motor kanan
    pwmKanan = 0;
    pidData[0].error = 0;
    pidData[0].integral = 0;
  }
  
  // ===== MOTOR KIRI (Motor 2) PID Control =====
  if (rpm2 > 0) {
    // Forward direction for motor kiri
    pwmKiri = computePID(1, rpm2, rpm_depan_kiri, kpLeft, kiLeft, kdLeft, minintegralLeft, maxintegralLeft);
    // Untuk target positif, pastikan PWM tidak negatif (tidak mundur)
    pwmKiri = constrain(pwmKiri, 0, pwm_max);
    
  } else if (rpm2 < 0) {
    // Reverse direction for motor kiri
    pwmKiri = computePID(1, rpm2, rpm_depan_kiri, kpLeft, kiLeft, kdLeft, minintegralLeft, maxintegralLeft);
    // Untuk target negatif, pastikan PWM tidak positif (tidak maju)
    pwmKiri = constrain(pwmKiri, pwm_min, 0);
    
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
  
  setMotorSpeed(1, pwmKiri);   // Motor 1 = Kiri (berdasarkan motor.ino)
  setMotorSpeed(2, pwmKanan);  // Motor 2 = Kanan (berdasarkan motor.ino)

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
