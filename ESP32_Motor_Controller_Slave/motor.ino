void setMotorSpeed(int motor, int speed) {
  int d1Pin, d2Pin, pwmPin;
  
  // Select motor pins
  if (motor == 1) {
    d1Pin = MOTOR1_D1;
    d2Pin = MOTOR1_D2;
    pwmPin = MOTOR1_PWM;
  } else if (motor == 2) {
    d1Pin = MOTOR2_D1;
    d2Pin = MOTOR2_D2;
    pwmPin = MOTOR2_PWM;
  } else {
    return; // Invalid motor number
  }
  
  // Limit speed to PWM range (0-4095 for 12-bit)
  int pwmValue = constrain(abs(speed), 0, 4095);
  
  // Set motor direction and speed
  if (speed > 0) {
    // Forward direction
    digitalWrite(d1Pin, HIGH);
    digitalWrite(d2Pin, LOW);
    analogWrite(pwmPin, pwmValue);
  } else if (speed < 0) {
    // Reverse direction
    digitalWrite(d1Pin, LOW);
    digitalWrite(d2Pin, HIGH);
    analogWrite(pwmPin, pwmValue);
  } else {
    // Stop motor
    digitalWrite(d1Pin, LOW);
    digitalWrite(d2Pin, LOW);
    analogWrite(pwmPin, 0);
  }
}

void stopAllMotors() {
  setMotorSpeed(1, 0);
  setMotorSpeed(2, 0);
}

// Emergency stop function
void emergencyStop() {
  stopAllMotors();
}
