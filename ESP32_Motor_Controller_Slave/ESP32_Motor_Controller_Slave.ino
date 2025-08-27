#include "config.h"

// PID Data array definition
PIDData pidData[numOutputs];

// Motor PWM output variables definition
int pwmKanan = 0;  // PWM Motor Kanan
int pwmKiri = 0;   // PWM Motor Kiri

// Error logging function
void logError(int errorCode, const char* message) {
  Serial.print("[ERROR ");
  Serial.print(errorCode);
  Serial.print("] ");
  Serial.println(message);
}

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  
  // Initialize Serial1 for communication with ESP32 Master
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  
  // Setup motor pins
  setupMotorPins();
  
  // Setup encoder interrupts
  setupEncoders();
  
  Serial.println("ESP32 Motor Controller Ready");
  Serial.println("Waiting for commands from Master ESP32...");
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
  
  // Read RPM from encoders
  pembacaan_RPM();
}