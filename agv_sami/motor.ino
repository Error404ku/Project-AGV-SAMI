void pwmMotor(int motor1, int motor2) {
  // Reset watchdog timer untuk mencegah timeout saat operasi motor intensif
  esp_task_wdt_reset();
  
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

  // Kontrol Motor Kanan (motor1) menggunakan L298N
  if (motor1 > 0) {
    // Motor kanan maju
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, motor1);  // ESP32 v3.x: gunakan analogWrite dengan pin langsung
  } else if (motor1 < 0) {
    // Motor kanan mundur
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, abs(motor1));  // ESP32 v3.x: gunakan analogWrite dengan pin langsung
  } else {
    // Motor kanan stop
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);  // ESP32 v3.x: gunakan analogWrite dengan pin langsung
  }

  // Kontrol Motor Kiri (motor2) menggunakan L298N
  if (motor2 > 0) {
    // Motor kiri maju
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, motor2);  // ESP32 v3.x: gunakan analogWrite dengan pin langsung
  } else if (motor2 < 0) {
    // Motor kiri mundur
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, abs(motor2));  // ESP32 v3.x: gunakan analogWrite dengan pin langsung
  } else {
    // Motor kiri stop
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 0);  // ESP32 v3.x: gunakan analogWrite dengan pin langsung
  }
}
