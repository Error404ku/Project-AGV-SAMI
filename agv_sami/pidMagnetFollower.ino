// Helper function: Initialize soft start state
void initSoftStart(int initialSpeed) {
  softStartTime = millis();
  softStartActive = true;
  pidSpeed = initialSpeed;
}

// Helper function: Reset PID state
void resetPIDState() {
  pidData[0].integral = 0;
  pidData[0].derivative = 0;
  pidData[0].previousError = 0;
}

// Helper function: Check if mode requires soft start
bool isSoftStartMode(PidMode mode) {
  return (mode == PID_MODE_MAJU || mode == PID_MODE_MAJU_MASSA);
}

// Helper function: Get PID parameters based on mode
void getPIDParameters(PidMode mode, float& kp, float& ki, float& kd) {
  if (mode == PID_MODE_MAJU) {
    kp = tempKpForwardDefault;
    ki = tempKiForwardDefault;
    kd = tempKdForwardDefault;
  } else if (mode == PID_MODE_MAJU_MASSA) {
    kp = tempKpForwardWithMassa;
    ki = tempKiForwardWithMassa;
    kd = tempKdForwardWithMassa;
  } else {
    // Default fallback - use forward default parameters
    kp = tempKpForwardDefault;
    ki = tempKiForwardDefault;
    kd = tempKdForwardDefault;
  }
}

// Helper function: Calculate speed ratio for gain scheduling
float calculateSpeedRatio(bool isSoftStart, int currentSpeed, int targetSpeed) {
  if (!isSoftStart || currentSpeed >= targetSpeed) {
    return 1.0;
  }
  return constrain((float)currentSpeed / (float)targetSpeed, 0.5, 1.0);
}

// Helper function: Calculate integral constraints
void calculateIntegralConstraints(float ki, double& minIntegral, double& maxIntegral) {
  if (ki > 0.001) {
    minIntegral = -500.0 / ki;
    maxIntegral = 500.0 / ki;
  } else {
    minIntegral = 0.0;
    maxIntegral = 0.0;
  }
}

void pidLinefollower(int errorPosisi, PidMode mode) {
  // === PHASE 1: Handle Magnet Loss ===
  if (errorPosisi == 99 && mode != PID_MODE_BERHENTI) {
    rpmMotor(0, 0);
    initSoftStart(0);
    music(MUSIC_MODE_OUTOFLINE);
    return;
  }
  
  // === PHASE 2: Handle Recovery from Magnet Loss ===
  if (errorPosisi != 99 && statusMusic && currentMusicMode == MUSIC_MODE_OUTOFLINE) {
    initSoftStart(maxMotorRpm / 2);
    stopMusic();
  }

  // === PHASE 3: Handle Mode Changes ===
  bool modeChanged = (mode != lastMode);
  bool needsInitialization = (!softStartActive && pidSpeed == 0);
  
  if (modeChanged || needsInitialization) {
    if (isSoftStartMode(mode)) {
      initSoftStart(maxMotorRpm / 2);
    }
    
    if (modeChanged) {
      resetPIDState();
      lastMode = mode;
    }
  }
  
  // === PHASE 4: Soft Start Speed Ramp ===
  if (softStartActive && (mode == PID_MODE_MAJU || mode == PID_MODE_MAJU_MASSA)) {
    unsigned long elapsedTime = millis() - softStartTime;
    
    if (elapsedTime < 2000) {
      pidSpeed = map(elapsedTime, 0, 2000, maxMotorRpm / 2, maxMotorRpm);
    } else {
      pidSpeed = maxMotorRpm;
      softStartActive = false;
    }
  } else if (mode == PID_MODE_BERHENTI || mode == PID_MODE_DEFAULT) {
    pidSpeed = 0;
    softStartActive = false;
  }

  // === PHASE 5: Apply Motor X-axis Inversion ===
  if (invertMotorX) {
    errorPosisi = -errorPosisi;
  }

  // === PHASE 6: Get PID Parameters ===
  float baseKp, baseKi, baseKd;
  getPIDParameters(mode, baseKp, baseKi, baseKd);

  // === PHASE 7: Gain Scheduling ===
  float speedRatio = calculateSpeedRatio(softStartActive, pidSpeed, maxMotorRpm);
  
  float currentKp = baseKp * speedRatio;
  float currentKi = baseKi * speedRatio;
  float currentKd = baseKd * speedRatio;

  // === PHASE 8: Calculate Integral Constraints ===
  double minintegral, maxintegral;
  calculateIntegralConstraints(currentKi, minintegral, maxintegral);

  // === PHASE 9: Compute PID Correction ===
  double koreksi = computePID(0, 0, -errorPosisi, currentKp, currentKi, currentKd, 
                               minintegral, maxintegral);
  
  // === PHASE 10: Apply Motor Commands ===
  int rpmKiri = constrain(pidSpeed + (int)koreksi, -90, 90);
  int rpmKanan = constrain(pidSpeed - (int)koreksi, -90, 90);
  
  switch (mode) {
    case PID_MODE_MAJU:
    case PID_MODE_MAJU_MASSA:
      rpmMotor(rpmKiri, rpmKanan);
      break;
      
    case PID_MODE_BERHENTI:
    case PID_MODE_DEFAULT:
      pwmMotor(0, 0);
      break;
  }
}
