// Versi yang diperbaiki dari fungsi computePID untuk menghindari masalah overflow
double computePID_fixed(int index, double setpoint, double input, double Kp, double Ki, double Kd, int Minintegral, double Maxintegral) {
  // Validate input parameters
  if (index < 0 || index >= numOutputs) {
    logError(ERROR_PID_CALCULATION, "PID index tidak valid");
    return 0.0;
  }

  // Check for invalid values (NaN or infinity)
  if (isnan(setpoint) || isnan(input) || isinf(setpoint) || isinf(input)) {
    logError(ERROR_PID_CALCULATION, "PID input NaN/Inf");
    return 0.0;
  }

  // Debug output untuk diagnosa
  Serial.printf("PID Calculation - Index: %d, Setpoint: %.2f, Input: %.2f\n", index, setpoint, input);

  // Hitung error - pastikan tipe data double digunakan dengan benar
  double error = setpoint - input;
  
  // Store error secara aman ke dalam pidData
  pidData[index].error = error;
  
  // Hitung integral dengan batas aman
  pidData[index].integral += error;
  pidData[index].integral = constrain(pidData[index].integral, Minintegral, Maxintegral);
  
  // Hitung derivative
  pidData[index].derivative = error - pidData[index].previousError;
  
  // Hitung output PID
  double output = Kp * error + Ki * pidData[index].integral + Kd * pidData[index].derivative;

  // Check for invalid output
  if (isnan(output) || isinf(output)) {
    logError(ERROR_PID_CALCULATION, "PID output NaN/Inf");
    return 0.0;
  }

  // Simpan nilai error untuk penggunaan selanjutnya
  pidData[index].previousError = error;

  // Debug output
  Serial.printf("PID Output - Error: %.2f, Integral: %.2f, Derivative: %.2f, Output: %.2f\n", 
                error, pidData[index].integral, pidData[index].derivative, output);

  return output;
}

// Versi yang diperbaiki dari fungsi rpmMotor untuk menghindari masalah overflow
void rpmMotor_fixed(float rpm1, float rpm2) {
  // Gunakan PID parameters dari preferences
  double kp = pidConfig.kp;
  double ki = pidConfig.ki; 
  double kd = pidConfig.kd;
  
  // Hitung integral limits berdasarkan Ki
  double minintegral = (ki != 0) ? -500.0 / ki : -500.0;  // Nilai dibatasi untuk mencegah overflow
  double maxintegral = (ki != 0) ? 500.0 / ki : 500.0;    // Nilai dibatasi untuk mencegah overflow
    
  // Constrain RPM to valid range
  rpm1 = constrain(rpm1, -maxrpm, maxrpm);
  rpm2 = constrain(rpm2, -maxrpm, maxrpm);
  
  // Debug output
  Serial.printf("Target RPM - Kanan: %.2f, Kiri: %.2f | Current RPM - Kanan: %.2f, Kiri: %.2f\n", 
                rpm1, rpm2, rpm_depan_kanan, rpm_depan_kiri);
  
  // ===== MOTOR KANAN (Motor 1) PID Control =====
  if (rpm1 > 0) {
    // Forward direction for motor kanan
    pwmKanan = computePID_fixed(0, rpm1, rpm_depan_kanan, kp, ki, kd, minintegral, maxintegral);
    pwmKanan = constrain(pwmKanan, pwm_zero, pwm_max);
    
  } else if (rpm1 < 0) {
    // Reverse direction for motor kanan
    pwmKanan = computePID_fixed(0, abs(rpm1), rpm_depan_kanan, kp, ki, kd, minintegral, maxintegral);
    pwmKanan = -pwmKanan;  // Make negative for reverse
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
    pwmKiri = computePID_fixed(1, rpm2, rpm_depan_kiri, kp, ki, kd, minintegral, maxintegral);
    pwmKiri = constrain(pwmKiri, pwm_zero, pwm_max);
    
  } else if (rpm2 < 0) {
    // Reverse direction for motor kiri
    pwmKiri = computePID_fixed(1, abs(rpm2), rpm_depan_kiri, kp, ki, kd, minintegral, maxintegral);
    pwmKiri = -pwmKiri;  // Make negative for reverse
    pwmKiri = constrain(pwmKiri, pwm_min, pwm_zero);
    
  } else if (rpm2 == 0) {
    // Stop motor kiri
    pwmKiri = 0;
    pidData[1].error = 0;
    pidData[1].integral = 0;
  }
  
  // Apply PWM values to motors
  Serial.printf("Error RPM (double) - Kanan: %.2f, Kiri: %.2f\n", pidData[0].error, pidData[1].error);
  setMotorSpeed(1, pwmKanan);  // Motor 1 = Kanan
  setMotorSpeed(2, pwmKiri);   // Motor 2 = Kiri
}

// Helper function untuk set target RPM dengan validasi
void setTargetRPM_fixed(int rpmKanan, int rpmKiri) {
  // Validasi input RPM
  rpmKanan = constrain(rpmKanan, -maxrpm, maxrpm);
  rpmKiri = constrain(rpmKiri, -maxrpm, maxrpm);
  
  // Panggil PID RPM control dengan versi yang sudah diperbaiki
  rpmMotor_fixed(rpmKanan, rpmKiri);
}
