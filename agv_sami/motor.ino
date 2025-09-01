// Motor control via Serial Communication to ESP32 Slave
// This replaces direct motor control with serial commands
// Now supporting both PWM and RPM modes for full conversion

// =============== NEW RPM-BASED MOTOR CONTROL ===============

void rpmMotor(int rpmKiri, int rpmKanan) {
  // Reset watchdog timer untuk mencegah timeout saat operasi motor intensif
  // esp_task_wdt_reset();
  
  // Safety checks for motor RPM values (10-90 range)
  if (abs(rpmKiri) > 90 || abs(rpmKanan) > 90) {
    logError(ERROR_MOTOR_CONTROL, "RPM nilai melebihi batas");
    rpmKiri = constrain(rpmKiri, -90, 90);
    rpmKanan = constrain(rpmKanan, -90, 90);
  }

  // Apply Y-axis inversion (maju-mundur) if enabled  
  if (invertMotorY) {
    rpmKiri = -rpmKiri;
    rpmKanan = -rpmKanan;
  }

  // Apply individual motor inversion
  if (invertMotorKanan) {
    rpmKanan = -rpmKanan;
  }
  if (invertMotorKiri) {
    rpmKiri = -rpmKiri;
  }

  // Kirim perintah RPM ke ESP32 motor controller
  sendRPM(rpmKiri, rpmKanan); // Direct RPM command
}

// =============== LEGACY PWM MOTOR CONTROL ===============

void pwmMotor(int motor1, int motor2) {
  // Reset watchdog timer untuk mencegah timeout saat operasi motor intensif
  // esp_task_wdt_reset();
  
  // Safety checks for motor PWM values
  if (abs(motor1) > maxPwm || abs(motor2) > maxPwm) {
    logError(ERROR_MOTOR_CONTROL, "PWM nilai melebihi batas");
    motor1 = constrain(motor1, minPwm, maxPwm);
    motor2 = constrain(motor2, minPwm, maxPwm);
  }

  // Additional safety for high PWM values to prevent system instability
  if (abs(motor1) > 3500 || abs(motor2) > 3500) {
    esp_task_wdt_reset();  // Extra watchdog reset for high current operations
  }

  // Apply Y-axis inversion (maju-mundur) if enabled
  if (invertMotorY) {
    motor1 = -motor1;
    motor2 = -motor2;
  }

  // Apply individual motor inversion
  if (invertMotorKanan) {
    motor1 = -motor1;
  }
  if (invertMotorKiri) {
    motor2 = -motor2;
  }

  // Kirim perintah ke ESP32 motor controller
  sendMotorCommand(motor2, motor1); // motor2=kiri, motor1=kanan
}
