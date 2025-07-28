

// PID variables with optimization
struct PIDController {
  float error;
  float lastError;
  float integral;
  float derivative;
  float lastMeasurement;
  unsigned long lastTime;
};

static PIDController pidController = {0, 0, 0, 0, 0, 0};

// Global variable for stop state (declared in config.h)
bool sudahStopPelanPelan = false;

// PID limits for anti-windup
const float integralMax = 1000.0;
const float integralMin = -1000.0;
void pidLinefollower(int errorPosisi, String mode) {
  // kalo sensor jarak mendeteksi ada benda di depan maka berhenti dulu
  if (obstacleDetected && mode != "BERHENTI") {
    // Emergency stop - obstacle detected
    pwmMotor(0, 0);
    // Reset PID state on emergency stop
    pidController.integral = 0;
    sudahStopPelanPelan = false;
    music("error");
    return; // Exit function early
  }

  unsigned long currentTime = millis();
  float deltaTime = (currentTime - pidController.lastTime) / 1000.0; // Convert to seconds
  
  if (deltaTime <= 0) deltaTime = 0.01; // Prevent division by zero
  
  pidController.error = errorPosisi;
  
  // Apply X-axis inversion (kiri-kanan) if enabled
  if (invertMotorX) {
    pidController.error = -pidController.error;
  }
  
  // Integral with anti-windup protection
  pidController.integral += pidController.error * deltaTime;
  pidController.integral = constrain(pidController.integral, integralMin, integralMax);
  
  // Derivative on measurement (prevents derivative kick)
  float measurement = pidController.error;
  pidController.derivative = (pidController.lastMeasurement - measurement) / deltaTime;
  pidController.lastMeasurement = measurement;

  float koreksi = kpLinefollower * pidController.error + 
                  kiLinefollower * pidController.integral +
                  kdLinefollower * pidController.derivative;

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
    // Reset integral on force commands
    pidController.integral = 0;
  } else if (mode == "FORCEMAJU") {
    pwmMotor(baseSpeed, -baseSpeed);
    // Reset integral on force commands
    pidController.integral = 0;
  } else if (mode == "STOPPELANPELAN") {
    if (!sudahStopPelanPelan) {
      pwmMotor(-baseSpeed / 2, baseSpeed / 2);
      static unsigned long stopStartTime = millis();
      if (millis() - stopStartTime >= 500) {
        pwmMotor(0, 0);
        sudahStopPelanPelan = true;
        pidController.integral = 0; // Reset integral on stop
      }
    } else {
      pwmMotor(0, 0);
    }
  } else {
    pwmMotor(0, 0);
    // Reset PID state when stopped
    pidController.integral = 0;
    sudahStopPelanPelan = false;
  }
  
  pidController.lastError = pidController.error;
  pidController.lastTime = currentTime;
}