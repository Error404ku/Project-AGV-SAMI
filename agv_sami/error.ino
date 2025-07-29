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

void error(int code, String text) {
  // Stop all motors immediately for safety
  pwmMotor(0, 0);

  // Display error on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Error Code : ");
  lcd.print(code);
  lcd.setCursor(0, 1);
  lcd.print("Error Text : ");
  lcd.setCursor(0, 2);
  lcd.print(text.substring(0, 20));  // Limit text to LCD width
  if (text.length() > 20) {
    lcd.setCursor(0, 3);
    lcd.print(text.substring(20, 40));  // Continue on next line
  }

  // Also print to Serial for debugging
  Serial.print("SYSTEM ERROR - Code: ");
  Serial.print(code);
  Serial.print(" - ");
  Serial.println(text);

  // Attempt error recovery instead of infinite loop
  if (!attemptErrorRecovery(code)) {
    // If recovery fails, enter safe mode but allow system monitoring
    Serial.println("CRITICAL ERROR - Entering safe mode");

    // Set system to safe state
    systemInErrorState = true;

    // Start error recovery timer for periodic retry
    startTimer(&errorRecoveryTimer, 10000);  // Retry every 10 seconds

    // Continue with limited functionality instead of complete halt
    return;
  } else {
    Serial.println("Error recovery successful - system resumed");
    systemInErrorState = false;
  }
}

// Non-fatal error function - logs error but continues operation
void logError(int code, String text) {
  Serial.print("WARNING - Code: ");
  Serial.print(code);
  Serial.print(" - ");
  Serial.println(text);

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