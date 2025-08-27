#include "config.h"

// PID RPM Control Function untuk 2 motor
void rpmMotor(int rpm1, int rpm2) {
  // PID Constants - Tuning parameters
  double kp = 1.0;     // Proportional gain
  double ki = 0.15;    // Integral gain  
  double kd = 0.05;    // Derivative gain
  
  // Integral constraints
  double minintegral = -1023 / ki;
  double maxintegral = 1023 / ki;
    
  // Constrain RPM to valid range
  rpm1 = constrain(rpm1, -maxrpm, maxrpm);
  rpm2 = constrain(rpm2, -maxrpm, maxrpm);
  
  // ===== MOTOR KANAN (Motor 1) PID Control =====
  if (rpm1 > 0) {
    // Forward direction for motor kanan
    pwmKanan = computePID(0, rpm1, rpm_depan_kanan, kp, ki, kd, minintegral, maxintegral);
    pwmKanan = constrain(pwmKanan, pwm_zero, pwm_max);
    
  } else if (rpm1 < 0) {
    // Reverse direction for motor kanan
    pwmKanan = computePID(0, abs(rpm1), rpm_depan_kanan, kp, ki, kd, minintegral, maxintegral);
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
    pwmKiri = computePID(1, rpm2, rpm_depan_kiri, kp, ki, kd, minintegral, maxintegral);
    pwmKiri = constrain(pwmKiri, pwm_zero, pwm_max);
    
  } else if (rpm2 < 0) {
    // Reverse direction for motor kiri
    pwmKiri = computePID(1, abs(rpm2), rpm_depan_kiri, kp, ki, kd, minintegral, maxintegral);
    pwmKiri = -pwmKiri;  // Make negative for reverse
    pwmKiri = constrain(pwmKiri, pwm_min, pwm_zero);
    
  } else if (rpm2 == 0) {
    // Stop motor kiri
    pwmKiri = 0;
    pidData[1].error = 0;
    pidData[1].integral = 0;
  }
  
  // Apply PWM values to motors
  setMotorSpeed(1, pwmKanan);  // Motor 1 = Kanan
  setMotorSpeed(2, pwmKiri);   // Motor 2 = Kiri
}

// Helper function untuk set target RPM dengan validasi
void setTargetRPM(int rpmKanan, int rpmKiri) {
  // Validasi input RPM
  rpmKanan = constrain(rpmKanan, -maxrpm, maxrpm);  // *2 karena akan dibagi 2 di rpmMotor
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
