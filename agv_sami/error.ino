/*
  ERROR HANDLING SYSTEM FOR AGV SAMI
  
  This system provides comprehensive error handling for the AGV system.
  
  ERROR CODES:
  1  - ERROR_SENSOR_COMMUNICATION: Sensor Modbus communication failures
  2  - ERROR_MOTOR_CONTROL: Motor control system failures
  3  - ERROR_RFID_COMMUNICATION: RFID reader communication errors
  4  - ERROR_WIFI_CONNECTION: WiFi connection failures
  5  - ERROR_LCD_COMMUNICATION: LCD display communication errors
  6  - ERROR_SYSTEM_INITIALIZATION: System startup failures
  7  - ERROR_ENCODER_FAILURE: Encoder feedback problems
  8  - ERROR_PID_CALCULATION: PID controller calculation errors
  9  - ERROR_MEMORY_ALLOCATION: Memory allocation failures
  10 - ERROR_INVALID_CONFIGURATION: Invalid configuration data
  
  FUNCTIONS:
  - error(int code, String text): Fatal error - stops system completely
  - logError(int code, String text): Warning - logs error but continues operation
  
  USAGE:
  - Call error() for critical failures that require immediate system shutdown
  - Call logError() for non-critical issues that should be logged but not stop operation
  
  INTEGRATION:
  - Sensor communication failures are monitored in bacasensor.ino
  - Motor control issues are checked in motor.ino
  - RFID errors are tracked in pembacaanRfid.ino
  - WiFi connection problems are handled in setup.ino
  - PID calculation errors are validated in pid.ino
  - HTTP parsing errors are logged in http.ino
*/

void systemError(int code, String text) {
  // Simplified error handling for production
  // Critical error - system stops
  if (code < 100) {
    // Log critical error (commented for production)
    // Serial.print("CRITICAL ERROR - Code: ");
    // Serial.print(code);
    // Serial.print(" - ");
    // Serial.println(text);
    
    // For critical errors, stop motors
    motorStop();
    
    // Could add LED indication or buzzer alarm here
    // Enter infinite loop to halt system
    while(true) {
      delay(1000);
    }
  }
  
  // For warnings (code >= 100), just continue
}

void systemWarning(int code, String text) {
  // Serial.print("WARNING - Code: ");
  // Serial.print(code);
  // Serial.print(" - ");
  // Serial.println(text);
}

// Non-fatal error function - logs error but continues operation
void logError(int code, String text) {
  // Serial.print("WARNING - Code: ");
  // Serial.print(code);
  // Serial.print(" - ");
  // Serial.println(text);

  // Could also briefly show on LCD without stopping system
  // For now, just log to Serial
}


/**
 * Buzzer error function - sounds alarm when obstacle detected or error
 */
//  void buzzerError() {
//     // Buzzer alarm pattern - 3 short beeps
//     // for (int i = 0; i < 3; i++) {
//     //   digitalWrite(BUZZER_PIN, HIGH);
//     //   delay(100);
//     //   digitalWrite(BUZZER_PIN, LOW);
//     //   delay(100);
//     // }
//   }