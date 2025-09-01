

// ===================================================================
// PID CONTROLLER VARIABLES SUDAH DIPINDAHKAN KE config.h
// ===================================================================
void pidLinefollower(int errorPosisi, PidMode mode) {
  // Reset watchdog timer untuk operasi PID yang intensif
  // esp_task_wdt_reset();
  
  // Soft start variables
  // Check for magnet loss error (errorValue = 99)
  if (errorPosisi == 99 && mode != PID_MODE_BERHENTI) {
    // Emergency stop - no magnet detected for 5 seconds
    pwmMotor(0, 0);  // Use PWM stop command
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
      pidSpeed = maxMotorRpm / 4; // Start with 25% of maxMotorRpm
    }
    // Reset PID data when mode changes to prevent carry-over
    if (mode != lastMode) {
      pidData[0].integral = 0;
      pidData[0].derivative = 0;
      pidData[0].previousError = 0;
    }
    lastMode = mode;
  }
  
  // Soft start implementation - gradually increase speed
  if (softStartActive && (mode == PID_MODE_MAJU || mode == PID_MODE_MAJU_MASSA || mode == PID_MODE_MUNDUR || mode == PID_MODE_MUNDUR_MASSA)) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - softStartTime;
    
    // Determine target speed based on mode
    int targetBaseSpeed = maxMotorRpm;
    if (mode == PID_MODE_MAJU_MASSA || mode == PID_MODE_MUNDUR_MASSA) {
      targetBaseSpeed = maxMotorRpm * 3 / 2;
    }
    
    if (elapsedTime < 5000) { // 5 seconds soft start duration
      // Gradually increase from 25% to 100% of baseSpeed over 5 seconds
      int targetSpeed = map(elapsedTime, 0, 5000, targetBaseSpeed / 4, targetBaseSpeed);
      pidSpeed = targetSpeed;
    } else {
      // Soft start complete
      pidSpeed = targetBaseSpeed;
      softStartActive = false;
    }
  } else if (mode == PID_MODE_FORCEMAJU || mode == PID_MODE_FORCEMUNDUR) {
    // For force modes, use baseSpeed directly
    pidSpeed = maxMotorRpm;
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

  // Use computePID function with proper integral constraints
  // setpoint = 0 (target center), input = -pidError (current error with correct sign)
  double minintegral, maxintegral;
  if (currentKi > 0.001) {  // Prevent divide by zero
    minintegral = -500.0 / currentKi;
    maxintegral = 500.0 / currentKi;
  } else {
    // If Ki is zero or near zero, disable integral
    minintegral = 0.0;
    maxintegral = 0.0;
  }
  // Fix: Use correct error sign - pidError is already the deviation from center
  double koreksi = computePID(0, 0, -pidError, currentKp, currentKi, currentKd, minintegral, maxintegral);
  
  int motorKiri = pidSpeed + (int)koreksi;   // Fixed: subtract correction for left motor
  int motorKanan = pidSpeed - (int)koreksi;  // Fixed: add correction for right motor

  // Apply PID corrections directly to RPM values
  // maxMotorRpm is the desired speed setting for AGV
  // Maximum constraint is 90 RPM (hardware limit)
  
  int rpmKiri = constrain(motorKiri, -90, 90);
  int rpmKanan = constrain(motorKanan, -90, 90);
  
  switch (mode) {
    case PID_MODE_MAJU:
      rpmMotor(rpmKiri, rpmKanan);  // RPM: left motor, right motor
      break;
    case PID_MODE_MAJU_MASSA:
      rpmMotor(rpmKiri, rpmKanan);  // RPM: left motor, right motor
      break;
    case PID_MODE_MUNDUR:
      rpmMotor(-rpmKiri, -rpmKanan);  // RPM: reverse direction
      break;
    case PID_MODE_MUNDUR_MASSA:
      rpmMotor(-rpmKiri, -rpmKanan);  // RPM: reverse direction
      break;
    case PID_MODE_FORCEMUNDUR:
      rpmMotor(-maxMotorRpm, -maxMotorRpm);  // Force backward with setting speed
      break;
    case PID_MODE_FORCEMAJU:
      rpmMotor(maxMotorRpm, maxMotorRpm);  // Force forward with setting speed
      break;
    case PID_MODE_STOPPELANPELAN:
      if (!sudahStopPelanPelan) {
        rpmMotor(maxMotorRpm / 2, maxMotorRpm / 2);  // Gradual stop with half setting speed
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
  
  #ifdef DEBUG_PID
  // Debug output every 100ms to monitor PID behavior
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 100) {
    // Serial.printf("[PID] E=%d V=%d Kp=%.1f Ki=%.1f Kd=%.1f U=%.1f L=%d R=%d\n", 
    //               errorPosisi, pidSpeed, currentKp, currentKi, currentKd, 
    //               koreksi, motorKiri, motorKanan);
    lastDebugTime = millis();
  }
  #endif
  
  // // Serial.println() - removed for production // Tidak bisa mencetak enum secara langsung
  
  // Note: lastError is now handled inside computePID function via pidData[0].previousError
}