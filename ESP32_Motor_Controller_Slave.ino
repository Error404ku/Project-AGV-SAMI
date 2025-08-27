/*
  ESP32 Motor Controller (Slave)
  Pin Configuration:
  - RX: 41, TX: 42 (Serial komunikasi dengan ESP32 Master)
  - Motor 1: D1=4, D2=5, PWM=6
  - Motor 2: D1=7, D2=15, PWM=16
*/

#include <Arduino.h>

// Pin Motor 1
#define MOTOR1_D1   4
#define MOTOR1_D2   5
#define MOTOR1_PWM  6

// Pin Motor 2
#define MOTOR2_D1   7
#define MOTOR2_D2   15
#define MOTOR2_PWM  16

// Serial communication pins - Update to match Master
#define RX_PIN 41  // Tetap sama seperti sebelumnya
#define TX_PIN 42  // Tetap sama seperti sebelumnya

// PWM settings - 12-bit resolution
#define PWM_FREQ 1000
#define PWM_RESOLUTION 12  // 12-bit PWM (0-4095)
#define PWM_CHANNEL_1 0
#define PWM_CHANNEL_2 1

// Variables for serial communication
String inputString = "";
bool stringComplete = false;

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  
  // Initialize Serial1 for communication with ESP32 Master
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  
  // Setup motor pins
  setupMotorPins();
  
  Serial.println("ESP32 Motor Controller Ready");
  Serial.println("Waiting for commands from Master ESP32...");
}

void setupMotorPins() {
  // Motor 1 pins
  pinMode(MOTOR1_D1, OUTPUT);
  pinMode(MOTOR1_D2, OUTPUT);
  pinMode(MOTOR1_PWM, OUTPUT);
  
  // Motor 2 pins
  pinMode(MOTOR2_D1, OUTPUT);
  pinMode(MOTOR2_D2, OUTPUT);
  pinMode(MOTOR2_PWM, OUTPUT);
  
  // Setup PWM channels
  ledcSetup(PWM_CHANNEL_1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_2, PWM_FREQ, PWM_RESOLUTION);
  
  // Attach PWM channels to pins
  ledcAttachPin(MOTOR1_PWM, PWM_CHANNEL_1);
  ledcAttachPin(MOTOR2_PWM, PWM_CHANNEL_2);
  
  // Initialize motors to stop
  stopAllMotors();
  
  Serial.println("Motor pins initialized");
}

void loop() {
  // Check for incoming serial data
  if (Serial1.available()) {
    serialEvent();
  }
  
  // Process complete command
  if (stringComplete) {
    processCommand(inputString);
    inputString = "";
    stringComplete = false;
  }
}

void serialEvent() {
  while (Serial1.available()) {
    char inChar = (char)Serial1.read();
    
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
}

void processCommand(String command) {
  // Format perintah: "L[speed]R[speed]"
  // Contoh: "L150R-100" (kiri maju 150, kanan mundur 100)
  
  Serial.print("Received command: ");
  Serial.println(command);
  
  int leftSpeed = 0;
  int rightSpeed = 0;
  
  // Parse command
  if (parseCommand(command, leftSpeed, rightSpeed)) {
    // Apply motor speeds
    setMotorSpeed(1, leftSpeed);   // Motor kiri
    setMotorSpeed(2, rightSpeed);  // Motor kanan
    
    Serial.print("Motor speeds set - Left: ");
    Serial.print(leftSpeed);
    Serial.print(", Right: ");
    Serial.println(rightSpeed);
  } else {
    Serial.println("Invalid command format");
    stopAllMotors();
  }
}

bool parseCommand(String command, int &leftSpeed, int &rightSpeed) {
  // Find L and R positions
  int lPos = command.indexOf('L');
  int rPos = command.indexOf('R');
  
  if (lPos == -1 || rPos == -1 || lPos >= rPos) {
    return false;
  }
  
  // Extract speeds
  String leftStr = command.substring(lPos + 1, rPos);
  String rightStr = command.substring(rPos + 1);
  
  leftSpeed = leftStr.toInt();
  rightSpeed = rightStr.toInt();
  
  // Constrain speeds to 12-bit PWM range (-4095 to 4095)
  leftSpeed = constrain(leftSpeed, -4095, 4095);
  rightSpeed = constrain(rightSpeed, -4095, 4095);
  
  return true;
}

void setMotorSpeed(int motor, int speed) {
  int d1Pin, d2Pin, pwmChannel;
  
  // Select motor pins
  if (motor == 1) {
    d1Pin = MOTOR1_D1;
    d2Pin = MOTOR1_D2;
    pwmChannel = PWM_CHANNEL_1;
  } else if (motor == 2) {
    d1Pin = MOTOR2_D1;
    d2Pin = MOTOR2_D2;
    pwmChannel = PWM_CHANNEL_2;
  } else {
    return; // Invalid motor number
  }
  
  // Set motor direction and speed
  if (speed > 0) {
    // Forward direction
    digitalWrite(d1Pin, HIGH);
    digitalWrite(d2Pin, LOW);
    ledcWrite(pwmChannel, speed);
  } else if (speed < 0) {
    // Reverse direction
    digitalWrite(d1Pin, LOW);
    digitalWrite(d2Pin, HIGH);
    ledcWrite(pwmChannel, abs(speed));
  } else {
    // Stop motor
    digitalWrite(d1Pin, LOW);
    digitalWrite(d2Pin, LOW);
    ledcWrite(pwmChannel, 0);
  }
}

void stopAllMotors() {
  setMotorSpeed(1, 0);
  setMotorSpeed(2, 0);
  Serial.println("All motors stopped");
}

// Emergency stop function
void emergencyStop() {
  stopAllMotors();
  Serial.println("EMERGENCY STOP ACTIVATED");
}
