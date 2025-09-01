// Setup functions - dipanggil dari main file

void setupMotorPins() {
  // Motor 1 pins
  pinMode(MOTOR1_D1, OUTPUT);
  pinMode(MOTOR1_D2, OUTPUT);
  pinMode(MOTOR1_PWM, OUTPUT);
  
  // Motor 2 pins
  pinMode(MOTOR2_D1, OUTPUT);
  pinMode(MOTOR2_D2, OUTPUT);
  pinMode(MOTOR2_PWM, OUTPUT);
  
  // Setup PWM channels - ESP32 v3.x API
  ledcAttach(MOTOR1_PWM, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(MOTOR2_PWM, PWM_FREQ, PWM_RESOLUTION);
  
  // Initialize motors to stop
  stopAllMotors();
}

void setupEncoders() {
  // Setup encoder pins
  pinMode(EncoderKananPinA, INPUT);
  pinMode(EncoderKananPinB, INPUT);
  pinMode(EncoderKiriPinA, INPUT);
  pinMode(EncoderKiriPinB, INPUT);
  
  // Attach interrupts untuk encoder
  attachInterrupt(digitalPinToInterrupt(EncoderKananPinA), EncoderKanan, CHANGE);
  attachInterrupt(digitalPinToInterrupt(EncoderKiriPinA), EncoderKiri, CHANGE);
}