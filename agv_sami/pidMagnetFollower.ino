

// ===================================================================
// PID CONTROLLER VARIABLES SUDAH DIPINDAHKAN KE config.h
// ===================================================================
void pidLinefollower(int errorPosisi, PidMode mode) {
  // Soft start variables
  static unsigned long softStartTime = 0;
  static bool softStartActive = false;
  static int lastMode = -1;
  
  // Check for magnet loss error (errorValue = 99)
  if (errorPosisi == 99 && mode != PID_MODE_BERHENTI) {
    // Emergency stop - no magnet detected for 5 seconds
    pwmMotor(0, 0);
    // Reset soft start when stopping
    softStartActive = false;
    pidSpeed = 0;
    #ifdef DEBUG_PID
    #endif
    // Debug removed for performance
    music(MUSIC_MODE_OUTOFLINE);
    return;  // Exit function early
  }
  
  // Check if AGV is back on track and stop music if it's OUTOFLINE mode (magnet loss music)
  if (errorPosisi != 99 && statusMusic && currentMusicMode == MUSIC_MODE_OUTOFLINE) {
    stopMusic();
  }

  // Initialize soft start when mode changes or first call
  if (mode != lastMode || (!softStartActive && pidSpeed == 0)) {
    if (mode == PID_MODE_MAJU || mode == PID_MODE_MAJU_MASSA || mode == PID_MODE_MUNDUR || mode == PID_MODE_MUNDUR_MASSA) {
      softStartTime = millis();
      softStartActive = true;
      pidSpeed = baseSpeed / 4; // Start with 25% of baseSpeed
    }
    lastMode = mode;
  }
  
  // Soft start implementation - gradually increase speed
  if (softStartActive && (mode == PID_MODE_MAJU || mode == PID_MODE_MAJU_MASSA || mode == PID_MODE_MUNDUR || mode == PID_MODE_MUNDUR_MASSA)) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - softStartTime;
    
    if (elapsedTime < 2000) { // 2 seconds soft start duration
      // Gradually increase from 25% to 100% of baseSpeed over 2 seconds
      int targetSpeed = map(elapsedTime, 0, 2000, baseSpeed / 4, baseSpeed);
      pidSpeed = targetSpeed;
    } else {
      // Soft start complete
      pidSpeed = baseSpeed;
      softStartActive = false;
    }
  } else if (mode == PID_MODE_FORCEMAJU || mode == PID_MODE_FORCEMUNDUR) {
    // For force modes, use baseSpeed directly
    pidSpeed = baseSpeed;
  } else if (mode == PID_MODE_STOPPELANPELAN) {
    // For gradual stop, use current pidSpeed
    // pidSpeed will be handled in the switch case
  } else if (mode == PID_MODE_BERHENTI || mode == PID_MODE_DEFAULT) {
    // Reset for stop modes
    pidSpeed = 0;
    softStartActive = false;
  }

  pidError = errorPosisi;

  // Apply X-axis inversion (kiri-kanan) if enabled
  if (invertMotorX) {
    pidError = -pidError;
  }

  integral += pidError;
  derivative = pidError - lastError;

  // Select PID parameters based on movement mode
  float currentKp, currentKi, currentKd;
  if (mode == PID_MODE_MAJU || mode == PID_MODE_FORCEMAJU) {
    // Use Default PID parameters for forward movement
    currentKp = kpLinefollowerForwardDefault;
    currentKi = kiLinefollowerForwardDefault;
    currentKd = kdLinefollowerForwardDefault;
  } else if (mode == PID_MODE_MAJU_MASSA) {
    // Use WithMassa PID parameters for forward movement with load
    currentKp = kpLinefollowerForwardWithMassa;
    currentKi = kiLinefollowerForwardWithMassa;
    currentKd = kdLinefollowerForwardWithMassa;
  } else if (mode == PID_MODE_MUNDUR || mode == PID_MODE_FORCEMUNDUR) {
    // Use Default PID parameters for backward movement
    currentKp = kpLinefollowerBackwardDefault;
    currentKi = kiLinefollowerBackwardDefault;
    currentKd = kdLinefollowerBackwardDefault;
  } else if (mode == PID_MODE_MUNDUR_MASSA) {
    // Use WithMassa PID parameters for backward movement with load
    currentKp = kpLinefollowerBackwardWithMassa;
    currentKi = kiLinefollowerBackwardWithMassa;
    currentKd = kdLinefollowerBackwardWithMassa;
  } else {
    // Default to legacy values for other modes
    currentKp = kpLinefollower;
    currentKi = kiLinefollower;
    currentKd = kdLinefollower;
  }

  float koreksi = currentKp * pidError + currentKi * integral + currentKd * derivative;
  int motorKiri = pidSpeed - koreksi;
  int motorKanan = pidSpeed + koreksi;
  if (PID_MODE_MAJU_MASSA || PID_MODE_MUNDUR_MASSA) {
    motorKiri = (pidSpeed*2) - koreksi;
    motorKanan = (pidSpeed*2) + koreksi;
  }

  motorKiri = constrain(motorKiri, -maxPwm, maxPwm);
  motorKanan = constrain(motorKanan, -maxPwm, maxPwm);
  switch (mode) {
    case PID_MODE_MAJU:
      pwmMotor(-motorKanan, motorKiri);
      break;
    case PID_MODE_MAJU_MASSA:
      pwmMotor(-motorKanan, motorKiri);
      break;
    case PID_MODE_MUNDUR:
      pwmMotor(motorKanan, -motorKiri);
      break;
    case PID_MODE_MUNDUR_MASSA:
      pwmMotor(motorKanan, -motorKiri);
      break;
    case PID_MODE_FORCEMUNDUR:
      pwmMotor(-pidSpeed, pidSpeed);
      break;
    case PID_MODE_FORCEMAJU:
      pwmMotor(pidSpeed, -pidSpeed);
      break;
    case PID_MODE_STOPPELANPELAN:
      if (!sudahStopPelanPelan) {
        pwmMotor(-pidSpeed / 2, pidSpeed / 2);
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