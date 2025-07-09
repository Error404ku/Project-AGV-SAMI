

float error = 0;
float lastError = 0;
float integral = 0;
float derivative = 0;

// Kecepatan dasar
bool sudahStopPelanPelan = false;
void pidLinefollower(int errorPosisi, String mode) {
  error = errorPosisi;
  integral += error;
  derivative = error - lastError;

  float koreksi = kpLinefollower * error + kiLinefollower * integral +
                  kdLinefollower * derivative;

  int motorKiri = baseSpeed - koreksi;
  int motorKanan = baseSpeed + koreksi;

  motorKiri = constrain(motorKiri, -maxPwm, maxPwm);
  motorKanan = constrain(motorKanan, -maxPwm, maxPwm);
  if (mode == "MAJU") {
    pwmMotor(motorKanan, -motorKiri);
  } else if (mode == "MUNDUR") {
    pwmMotor(-motorKanan, motorKiri);
  } else if (mode == "FORCEMUNDUR") {
    pwmMotor(-baseSpeed, baseSpeed);
  } else if (mode == "FORCEMAJU") {
    pwmMotor(baseSpeed, -baseSpeed);
  } else if (mode == "STOPPELANPELAN") {
    if (!sudahStopPelanPelan) {
      pwmMotor(-baseSpeed / 2, baseSpeed / 2);
      delay(500);
      pwmMotor(0, 0);
      sudahStopPelanPelan = true;
    } else {
      pwmMotor(0, 0);
    }
  } else {
    pwmMotor(0, 0);
  }
  Serial.println(mode);
  lastError = error;
}