// =============================================
// ENHANCED PID CONTROLLER with BEST PRACTICES
// =============================================
// Improvements based on:
// 1. Anti-Windup (Back-calculation) - Åström & Hägglund
// 2. Derivative Filtering (Low-pass filter) - NI White Paper
// 3. Setpoint Weighting (2-DOF PID) - Wikipedia PID Controller
// Reference: "PID Controllers: Theory, Design, and Tuning"

double computePID(int index, double setpoint, double input, double Kp, double Ki, double Kd, double Minintegral, double Maxintegral) {
  // Validate input parameters
  if (index < 0 || index >= numOutputs) {
    logError(ERROR_PID_CALCULATION, "PID index tidak valid");
    return 0.0;
  }

  // Check for invalid values (NaN or infinity)
  if (isnan(setpoint) || isnan(input) || isinf(setpoint) || isinf(input)) {
    logError(ERROR_PID_CALCULATION, "PID input NaN/Inf");
    return 0.0;
  }

  // Ensure input values are within reasonable limits
  input = constrain(input, -1.0 * (double)maxrpm, (double)maxrpm);

  // Calculate dt (sampling time in seconds)
  static unsigned long lastTime[numOutputs] = {0};
  unsigned long currentTime = millis();
  double dt = (currentTime - lastTime[index]) / 1000.0;  // Convert to seconds
  if (lastTime[index] == 0 || dt <= 0.0 || dt > 1.0) {
    dt = 0.1;  // Default 100ms if first run or invalid
  }
  lastTime[index] = currentTime;

  // 🔴 SAFE MODE: Print dt untuk debugging
  if (ENABLE_SAFE_MODE && index == 0) {  // Hanya motor kanan untuk debug
    static unsigned long lastDebugPrint = 0;
    if (currentTime - lastDebugPrint > 2000) {  // Print setiap 2 detik
      Serial.printf("🔍 SAFE MODE - PID[%d] dt: %.3fs\n", index, dt);
      lastDebugPrint = currentTime;
    }
  }

  // Debug output untuk diagnosa
  Serial.printf("PID[%d] Calculation - SP: %.2f, PV: %.2f, dt: %.3f\n", index, setpoint, input, dt);

  // =============================================
  // BEST PRACTICE 1: DYNAMIC SETPOINT WEIGHTING (2-DOF PID)
  // =============================================
  // Reference: Wikipedia - "Setpoint weighting reduces overshoot on setpoint changes"
  // DYNAMIC: Use high weight when far from target (fast rise), low weight when near (reduce overshoot)
  
  double errorFull = setpoint - input;  // Full error for calculation
  double errorRatio = abs(errorFull) / max(abs(setpoint), 1.0);  // Normalized error (0-1)
  
  // Dynamic weight: 0.8 when far (fast), 0.3 when near (smooth)
  double dynamicWeight = SETPOINT_WEIGHT_P_FAST;
  if (errorRatio < SETPOINT_WEIGHT_TRANSITION) {
    // Transition from fast to slow as we approach target
    double transitionRatio = errorRatio / SETPOINT_WEIGHT_TRANSITION;
    dynamicWeight = SETPOINT_WEIGHT_P_SLOW + (SETPOINT_WEIGHT_P_FAST - SETPOINT_WEIGHT_P_SLOW) * transitionRatio;
  }
  
  // Proportional on weighted setpoint (b*SP - PV) instead of (SP - PV)
  double errorP = (dynamicWeight * setpoint) - input;
  double pTerm = Kp * errorP;
  
  // Integral on full error (MUST use full error for zero steady-state error)
  double errorI = errorFull;
  pidData[index].error = errorI;  // Store full error
  
  // Accumulate integral
  pidData[index].integral += errorI * dt;
  
  // =============================================
  // BEST PRACTICE 2: DERIVATIVE ON MEASUREMENT (not on error)
  // =============================================
  // Reference: Wikipedia - "Derivative of PV rather than error eliminates derivative kick"
  // This prevents large derivative spike when setpoint changes suddenly
  
  double dTerm = 0.0;  // Initialize
  
  // 🔴 SAFE MODE: Disable derivative completely if enabled
  if (!ENABLE_SAFE_MODE) {
    double rawDerivative = 0.0;
    if (dt > 0.0) {
      // Derivative on measurement (negative because we want to oppose change in PV)
      rawDerivative = -(input - pidData[index].previousInput) / dt;
    }
    pidData[index].previousInput = input;  // Store current input for next iteration
    
    // =============================================
    // BEST PRACTICE 3: DERIVATIVE FILTERING (Low-pass filter)
    // =============================================
    // Reference: NI White Paper - "Derivative requires filtering due to noise sensitivity"
    // IIR Low-pass filter: D(k) = alpha*D(k-1) + (1-alpha)*D_raw(k)
    // alpha = 1 / (1 + N), where N is filter coefficient (typical: 5-20)
    double filterAlpha = 1.0 / (1.0 + DERIVATIVE_FILTER_N);
    pidData[index].filteredDerivative = filterAlpha * pidData[index].filteredDerivative 
                                       + (1.0 - filterAlpha) * rawDerivative;
    
    // Limit derivative to prevent extreme spikes
    pidData[index].filteredDerivative = constrain(pidData[index].filteredDerivative, -500.0, 500.0);
    
    dTerm = Kd * pidData[index].filteredDerivative;
  } else {
    // SAFE MODE: Zero derivative term
    pidData[index].previousInput = input;
    pidData[index].filteredDerivative = 0.0;
    dTerm = 0.0;
  }
  
  // Calculate PID output BEFORE saturation
  double iTerm = Ki * pidData[index].integral;
  double outputBeforeSaturation = pTerm + iTerm + dTerm;
  
  // Apply output saturation
  double output = constrain(outputBeforeSaturation, -4000.0, 4000.0);
  
  // =============================================
  // BEST PRACTICE 4: ANTI-WINDUP (Back-calculation method)
  // =============================================
  // Reference: Åström & Hägglund - "Back-calculation prevents integral windup"
  // When output saturates, adjust integral to prevent further buildup
  
  // 🔴 SAFE MODE: Less aggressive anti-windup
  if (output != outputBeforeSaturation && Ki > 0.01 && !ENABLE_SAFE_MODE) {
    // Calculate saturation error
    double saturationError = output - outputBeforeSaturation;
    
    // Calculate tracking time constant: Tt = ANTI_WINDUP_TRACKING_TIME_RATIO * Ti
    // where Ti = Kp/Ki (integral time constant)
    double Ti = (Ki > 0.01) ? (Kp / Ki) : 1.0;
    double Tt = ANTI_WINDUP_TRACKING_TIME_RATIO * Ti;
    
    // Back-calculate integral adjustment
    if (Tt > 0.0) {
      double integralAdjustment = (saturationError / Ki) * (dt / Tt);
      pidData[index].integral += integralAdjustment;
    }
  } else if (ENABLE_SAFE_MODE && output != outputBeforeSaturation) {
    // SAFE MODE: Simple clamping only
    if (output >= 4000.0 && pidData[index].integral > 0) {
      pidData[index].integral *= 0.9;  // Reduce integral slowly
    } else if (output <= -4000.0 && pidData[index].integral < 0) {
      pidData[index].integral *= 0.9;
    }
  }
  
  // Apply integral limits (secondary protection)
  pidData[index].integral = constrain(pidData[index].integral, Minintegral, Maxintegral);

  // Check for invalid output
  if (isnan(output) || isinf(output)) {
    logError(ERROR_PID_CALCULATION, "PID output NaN/Inf");
    pidData[index].integral = 0; // Reset integral saat output tidak valid
    return 0.0;
  }

  // Limit P-term to prevent excessive proportional action
  pTerm = constrain(pTerm, -2000.0, 2000.0);

  // Debug output untuk troubleshooting
  Serial.printf("PID Terms - P: %.2f, I: %.2f (int: %.2f), D: %.2f, Out: %.2f\n", 
                pTerm, iTerm, pidData[index].integral, dTerm, output);

  // Store previous error for compatibility (though not used in new algorithm)
  pidData[index].previousError = errorI;

  return output;
}

