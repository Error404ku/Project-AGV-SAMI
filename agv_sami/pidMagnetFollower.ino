

float pidError = 0;
float lastError = 0;
float integral = 0;
float derivative = 0;

// Kecepatan dasar
bool sudahStopPelanPelan = false;
void pidLinefollower(int errorPosisi, String mode) {
  // kalo sensor jarak mendeteksi ada benda di depan maka berhenti dulu
  if (obstacleDetected && mode != "BERHENTI") {
    // Emergency stop - obstacle detected
    pwmMotor(0, 0);
    Serial.println("MOTOR STOPPED - Obstacle detected!");
    // buzzerError();
    music("error");
    return;  // Exit function early
  }

  pidError = errorPosisi;

  // Apply X-axis inversion (kiri-kanan) if enabled
  if (invertMotorX) {
    pidError = -pidError;
  }

  integral += pidError;
  derivative = pidError - lastError;

  float koreksi = kpLinefollower * pidError + kiLinefollower * integral + kdLinefollower * derivative;

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
      startTimer(&stopPelanPelanTimer, 500);
      sudahStopPelanPelan = true;
    } else if (checkTimer(&stopPelanPelanTimer)) {
      pwmMotor(0, 0);
    } else if (!isTimerActive(&stopPelanPelanTimer)) {
      pwmMotor(0, 0);
    }
  } else {
    pwmMotor(0, 0);
  }
  Serial.println(mode);
  lastError = pidError;
}