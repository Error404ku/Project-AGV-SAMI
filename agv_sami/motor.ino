void pwmMotor(int motor1, int motor2) {
  // Safety checks for motor PWM values
  if (abs(motor1) > maxPwm || abs(motor2) > maxPwm) {
    logError(ERROR_MOTOR_CONTROL, "PWM nilai melebihi batas");
    motor1 = constrain(motor1, minPwm, maxPwm);
    motor2 = constrain(motor2, minPwm, maxPwm);
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
    ledcWrite(channelKanan, motor1);
  } else if (motor1 < 0) {
    // Motor kanan mundur
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    ledcWrite(channelKanan, abs(motor1));
  } else {
    // Motor kanan stop
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    ledcWrite(channelKanan, 0);
  }

  // Kontrol Motor Kiri (motor2) menggunakan L298N
  if (motor2 > 0) {
    // Motor kiri maju
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    ledcWrite(channelKiri, motor2);
  } else if (motor2 < 0) {
    // Motor kiri mundur
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    ledcWrite(channelKiri, abs(motor2));
  } else {
    // Motor kiri stop
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    ledcWrite(channelKiri, 0);
  }
}
