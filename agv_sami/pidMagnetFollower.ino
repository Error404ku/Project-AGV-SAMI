

// ===================================================================
// PID CONTROLLER VARIABLES SUDAH DIPINDAHKAN KE config.h
// ===================================================================
void pidLinefollower(int errorPosisi, PidMode mode) {
  // kalo sensor jarak mendeteksi ada benda di depan maka berhenti dulu
  if (obstacleDetected && mode != PID_MODE_BERHENTI) {
    // Emergency stop - obstacle detected
    pwmMotor(0, 0);
    Serial.println("MOTOR STOPPED - Obstacle detected!");
    // buzzerError();
    music(MUSIC_MODE_ERROR);
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
  switch (mode) {
    case PID_MODE_MAJU:
      pwmMotor(-motorKanan, motorKiri);
      break;
    case PID_MODE_MUNDUR:
      pwmMotor(motorKanan, -motorKiri);
      break;
    case PID_MODE_FORCEMUNDUR:
      pwmMotor(-baseSpeed, baseSpeed);
      break;
    case PID_MODE_FORCEMAJU:
      pwmMotor(baseSpeed, -baseSpeed);
      break;
    case PID_MODE_STOPPELANPELAN:
      if (!sudahStopPelanPelan) {
        pwmMotor(-baseSpeed / 2, baseSpeed / 2);
        startTimer(&stopPelanPelanTimer, 500);
        sudahStopPelanPelan = true;
      } else if (checkTimer(&stopPelanPelanTimer)) {
        pwmMotor(0, 0);
      } else if (!isTimerActive(&stopPelanPelanTimer)) {
        pwmMotor(0, 0);
      }
      break;
    case PID_MODE_BERHENTI:
    case PID_MODE_DEFAULT:
      pwmMotor(0, 0);
      break;
  }
  // Serial.println(mode); // Tidak bisa mencetak enum secara langsung

  lastError = pidError;
}