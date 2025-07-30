// Display throttling variables
static unsigned long lastDisplayUpdate = 0;
static unsigned long lastButtonCheck = 0;
const unsigned long displayUpdateInterval = 100; // Update display every 100ms
const unsigned long buttonCheckInterval = 50;    // Check buttons every 50ms

// Previous display state for change detection
static String lastDisplayContent = "";
static bool forceDisplayUpdate = false;

void displayPrint() {
  unsigned long currentMillis = millis();
  
  // Check buttons more frequently than display updates
  if (currentMillis - lastButtonCheck >= buttonCheckInterval) {
    lastButtonCheck = currentMillis;
    uji_tombol();
  }
  
  // Throttle display updates
  if (currentMillis - lastDisplayUpdate >= displayUpdateInterval || forceDisplayUpdate) {
    lastDisplayUpdate = currentMillis;
    forceDisplayUpdate = false;
    
    displaySensorDataOptimized();
    displayLogicAgv();
  }
}

// Force display update (call when important data changes)
void forceDisplayRefresh() {
  forceDisplayUpdate = true;
}


// Optimized display with change detection
void displaySensorDataOptimized() {
  // Build display content string
  String currentContent = "";
  uint8_t* currentMagnetData = getCurrentMagnetData();
  
  // Build sensor data string
  for (int i = 0; i < 16; i++) {
    currentContent += String(currentMagnetData[i] ? "1" : "0");
  }
  
  // Add error value
  currentContent += "E:" + String(errorValue);
  
  // Only update LCD if content changed
  if (currentContent != lastDisplayContent) {
    lastDisplayContent = currentContent;
    
    // Update LCD efficiently
    updateLCDContent();
  }
}

void updateLCDContent() {
  uint8_t* currentMagnetData = getCurrentMagnetData();
  
  lcd.setCursor(0, 0);
  lcd.print("Sensor:");
  lcd.setCursor(0, 1);
  for (int i = 0; i < 8; i++) {
    lcd.print(currentMagnetData[i] ? "1" : "0");
  }
  lcd.setCursor(8, 1);
  for (int i = 8; i < 16; i++) {
    lcd.print(currentMagnetData[i] ? "1" : "0");
  }
  
  // Show error value with padding
  lcd.setCursor(0, 2);
  lcd.print("Error: ");
  lcd.print(errorValue);
  lcd.print("   "); // Clear trailing characters
}

// Legacy function for compatibility
void displaySensorData() {
  displaySensorDataOptimized();
}

// void displayRpm(){
//   lcd.setCursor(0, 3);
//   lcd.print("RPM R:");
//   lcd.print(rpmKanan);
//   lcd.print(" L:");
//   lcd.print(rpmKiri);
// }

// void displayEncoderValue(){
//   lcd.setCursor(0, 3);
//   lcd.print("Enc R:");
//   lcd.print(encKananAVal);
//   lcd.print(" L:");
//   lcd.print(encKiriAVal);
// }