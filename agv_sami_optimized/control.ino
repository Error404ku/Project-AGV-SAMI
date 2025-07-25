/*
  CONTROL.INO - Motor Control and PID System
  
  This file combines all control-related functions:
  - PID controller implementation
  - Motor control with PWM
  - Line following logic
  - Movement control
*/

// ==================== PID CONTROLLER ====================

// PID data arrays for multiple controllers
PIDData pidData[2];  // 0 = general, 1 = line follower
const int numOutputs = 2;

// Line follower PID variables
float pidError = 0;
float lastError = 0;
float integral = 0;
float derivative = 0;
bool sudahStopPelanPelan = false;

double computePID(int index, double setpoint, double input, double Kp, double Ki, double Kd, double minIntegral, double maxIntegral) {
  // Validate input parameters
  if (index < 0 || index >= numOutputs) {
    logError(ERROR_PID_CALCULATION, "PID index tidak valid: " + String(index));
    return 0.0;
  }
  
  // Check for invalid values (NaN or infinity)
  if (isnan(setpoint) || isnan(input) || isinf(setpoint) || isinf(input)) {
    logError(ERROR_PID_CALCULATION, "PID input NaN/Inf detected");
    return 0.0;
  }
  
  // Calculate error
  pidData[index].error = setpoint - input;
  
  // Calculate integral with windup protection
  pidData[index].integral += pidData[index].error;
  pidData[index].integral = constrain(pidData[index].integral, minIntegral, maxIntegral);
  
  // Calculate derivative
  pidData[index].derivative = pidData[index].error - pidData[index].previousError;
  
  // Calculate PID output
  double output = Kp * pidData[index].error + 
                 Ki * pidData[index].integral + 
                 Kd * pidData[index].derivative;
  
  // Check for invalid output
  if (isnan(output) || isinf(output)) {
    logError(ERROR_PID_CALCULATION, "PID output NaN/Inf detected");
    return 0.0;
  }
  
  // Constrain output to valid range
  output = constrain(output, PID_MIN_OUTPUT, PID_MAX_OUTPUT);
  
  // Store error for next iteration
  pidData[index].previousError = pidData[index].error;
  
  return output;
}

void resetPID(int index) {
  if (index >= 0 && index < numOutputs) {
    pidData[index].error = 0;
    pidData[index].integral = 0;
    pidData[index].derivative = 0;
    pidData[index].previousError = 0;
  }
}

void resetAllPID() {
  for (int i = 0; i < numOutputs; i++) {
    resetPID(i);
  }
  
  // Reset line follower PID variables
  pidError = 0;
  lastError = 0;
  integral = 0;
  derivative = 0;
}

// ==================== LINE FOLLOWER PID ====================

void pidLinefollower(int errorPosisi, String mode) {
  // Emergency stop if obstacle detected (except when explicitly stopping)
  if (sensorData.obstacleDetected && mode != "BERHENTI" && mode != "STOPPELANPELAN") {
    pwmMotor(0, 0);
    music("error");
    DEBUG_PRINTLN("MOTOR STOPPED - Obstacle detected!");
    return;
  }
  
  pidError = errorPosisi;
  
  // Apply X-axis inversion (left-right) if enabled
  if (systemConfig.invertMotorX) {
    pidError = -pidError;
  }
  
  // Calculate PID terms
  integral += pidError;
  derivative = pidError - lastError;
  
  // Calculate correction using PID
  float koreksi = systemConfig.pidLinefollower.kp * pidError + 
                 systemConfig.pidLinefollower.ki * integral +
                 systemConfig.pidLinefollower.kd * derivative;
  
  // Calculate motor speeds
  int motorKiri = systemConfig.baseSpeed - koreksi;
  int motorKanan = systemConfig.baseSpeed + koreksi;
  
  // Constrain motor speeds
  motorKiri = constrain(motorKiri, -systemConfig.maxPwm, systemConfig.maxPwm);
  motorKanan = constrain(motorKanan, -systemConfig.maxPwm, systemConfig.maxPwm);
  
  // Execute movement based on mode
  if (mode == "MAJU") {
    pwmMotor(motorKanan, -motorKiri);
    systemState.currentMovement = MOVEMENT_FORWARD;
  } 
  else if (mode == "MUNDUR") {
    pwmMotor(-motorKanan, motorKiri);
    systemState.currentMovement = MOVEMENT_BACKWARD;
  } 
  else if (mode == "FORCEMUNDUR") {
    pwmMotor(-systemConfig.baseSpeed, systemConfig.baseSpeed);
    systemState.currentMovement = MOVEMENT_BACKWARD;
  } 
  else if (mode == "FORCEMAJU") {
    pwmMotor(systemConfig.baseSpeed, -systemConfig.baseSpeed);
    systemState.currentMovement = MOVEMENT_FORWARD;
  } 
  else if (mode == "STOPPELANPELAN") {
    if (!sudahStopPelanPelan) {
      pwmMotor(-systemConfig.baseSpeed / 2, systemConfig.baseSpeed / 2);
      delay(500);
      pwmMotor(0, 0);
      sudahStopPelanPelan = true;
    } else {
      pwmMotor(0, 0);
    }
    systemState.currentMovement = MOVEMENT_STOP;
  } 
  else {
    pwmMotor(0, 0);
    systemState.currentMovement = MOVEMENT_STOP;
  }
  
  // Store error for next iteration
  lastError = pidError;
  
  DEBUG_PRINTF("PID Mode: %s, Error: %d, Correction: %.2f\n", mode.c_str(), errorPosisi, koreksi);
}

// ==================== MOTOR CONTROL ====================

void pwmMotor(int speedKanan, int speedKiri) {
  // Validate input parameters
  if (abs(speedKanan) > systemConfig.maxPwm || abs(speedKiri) > systemConfig.maxPwm) {
    logError(ERROR_MOTOR_CONTROL, "Motor speed out of range");
    speedKanan = constrain(speedKanan, -systemConfig.maxPwm, systemConfig.maxPwm);
    speedKiri = constrain(speedKiri, -systemConfig.maxPwm, systemConfig.maxPwm);
  }
  
  // Apply motor inversions
  if (systemConfig.invertMotorY) {
    speedKanan = -speedKanan;
    speedKiri = -speedKiri;
  }
  
  if (systemConfig.invertMotorKanan) {
    speedKanan = -speedKanan;
  }
  
  if (systemConfig.invertMotorKiri) {
    speedKiri = -speedKiri;
  }
  
  // Control right motor
  if (speedKanan > 0) {
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
  } else if (speedKanan < 0) {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, HIGH);
  } else {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);
  }
  
  // Control left motor
  if (speedKiri > 0) {
    digitalWrite(MOTOR_IN3, HIGH);
    digitalWrite(MOTOR_IN4, LOW);
  } else if (speedKiri < 0) {
    digitalWrite(MOTOR_IN3, LOW);
    digitalWrite(MOTOR_IN4, HIGH);
  } else {
    digitalWrite(MOTOR_IN3, LOW);
    digitalWrite(MOTOR_IN4, LOW);
  }
  
  // Set PWM speeds
  ledcWrite(PWM_CHANNEL_ENA, abs(speedKanan));
  ledcWrite(PWM_CHANNEL_ENB, abs(speedKiri));
  
  DEBUG_PRINTF("Motor speeds - Right: %d, Left: %d\n", speedKanan, speedKiri);
}

void stopMotors() {
  pwmMotor(0, 0);
  systemState.currentMovement = MOVEMENT_STOP;
  
  // Reset PID to prevent integral windup
  resetAllPID();
  sudahStopPelanPelan = false;
  
  DEBUG_PRINTLN("Motors stopped");
}

void emergencyStop() {
  // Immediate motor stop without PID reset (for emergency situations)
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
  ledcWrite(PWM_CHANNEL_ENA, 0);
  ledcWrite(PWM_CHANNEL_ENB, 0);
  
  systemState.currentMovement = MOVEMENT_STOP;
  
  logError(ERROR_MOTOR_CONTROL, "Emergency stop activated");
  DEBUG_PRINTLN("EMERGENCY STOP!");
}

// ==================== MOVEMENT CONTROL ====================

void moveForward(int speed) {
  speed = constrain(speed, 0, systemConfig.maxPwm);
  pwmMotor(speed, -speed);
  systemState.currentMovement = MOVEMENT_FORWARD;
  modeMaju = true;
  modeMundur = false;
  modeBerhenti = false;
}

void moveBackward(int speed) {
  speed = constrain(speed, 0, systemConfig.maxPwm);
  pwmMotor(-speed, speed);
  systemState.currentMovement = MOVEMENT_BACKWARD;
  modeMaju = false;
  modeMundur = true;
  modeBerhenti = false;
}

void turnLeft(int speed) {
  speed = constrain(speed, 0, systemConfig.maxPwm);
  pwmMotor(-speed, -speed);
  systemState.currentMovement = MOVEMENT_LEFT;
  modeMaju = false;
  modeMundur = false;
  modeBerhenti = false;
}

void turnRight(int speed) {
  speed = constrain(speed, 0, systemConfig.maxPwm);
  pwmMotor(speed, speed);
  systemState.currentMovement = MOVEMENT_RIGHT;
  modeMaju = false;
  modeMundur = false;
  modeBerhenti = false;
}

void stopMovement() {
  stopMotors();
  modeMaju = false;
  modeMundur = false;
  modeBerhenti = true;
}

// ==================== MOTOR TEST FUNCTIONS ====================

void testMotorForward(int duration_ms) {
  DEBUG_PRINTLN("Testing motor forward");
  moveForward(systemConfig.baseSpeed / 2);
  delay(duration_ms);
  stopMovement();
}

void testMotorBackward(int duration_ms) {
  DEBUG_PRINTLN("Testing motor backward");
  moveBackward(systemConfig.baseSpeed / 2);
  delay(duration_ms);
  stopMovement();
}

void testMotorLeft(int duration_ms) {
  DEBUG_PRINTLN("Testing motor left turn");
  turnLeft(systemConfig.baseSpeed / 2);
  delay(duration_ms);
  stopMovement();
}

void testMotorRight(int duration_ms) {
  DEBUG_PRINTLN("Testing motor right turn");
  turnRight(systemConfig.baseSpeed / 2);
  delay(duration_ms);
  stopMovement();
}

void testAllMotors() {
  DEBUG_PRINTLN("Starting motor test sequence");
  
  testMotorForward(1000);
  delay(500);
  testMotorBackward(1000);
  delay(500);
  testMotorLeft(1000);
  delay(500);
  testMotorRight(1000);
  delay(500);
  
  DEBUG_PRINTLN("Motor test sequence complete");
}

// ==================== SAFETY FUNCTIONS ====================

bool isMotorSafe() {
  // Check if it's safe to operate motors
  
  // Check for obstacles in movement direction
  if (systemState.currentMovement == MOVEMENT_FORWARD && hasObstacle(true)) {
    return false;
  }
  
  if (systemState.currentMovement == MOVEMENT_BACKWARD && hasObstacle(false)) {
    return false;
  }
  
  // Check system state
  if (!systemState.isInitialized) {
    return false;
  }
  
  // Check sensor communication
  if (!deviceStatus[0].isOnline) {  // Front magnet sensor
    return false;
  }
  
  return true;
}

void enforceMotorSafety() {
  if (!isMotorSafe() && systemState.currentMovement != MOVEMENT_STOP) {
    emergencyStop();
    music("error");
  }
}

// ==================== COMPATIBILITY FUNCTIONS ====================

// These functions maintain compatibility with existing code
void rpmMotor() {
  // RPM control functionality was commented out in original code
  // This function is kept for compatibility but does nothing
  DEBUG_PRINTLN("RPM motor control not implemented");
}

// Legacy function names for backward compatibility
void motorControl(int rightSpeed, int leftSpeed) {
  pwmMotor(rightSpeed, leftSpeed);
}

void setMotorSpeed(int speed) {
  systemConfig.baseSpeed = constrain(speed, 0, systemConfig.maxPwm);
}

int getMotorSpeed() {
  return systemConfig.baseSpeed;
}