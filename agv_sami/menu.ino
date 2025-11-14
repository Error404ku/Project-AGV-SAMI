// ===================================================================
// ALL CONSTANTS AND DEFINES MOVED TO config.h
// ===================================================================

// ===================================================================
// HELPER FUNCTIONS - Mengurangi nested if statements & complexity
// ===================================================================

/**
 * Macro untuk simplifikasi button timing check pattern
 * Mengurangi nested if statements dengan early return pattern
 */
#define CHECK_BUTTON_TIMING(currentMillis, lastButtonPress) \
  if ((currentMillis) - (lastButtonPress) < buttonDelay) break

/**
 * Update lastButtonPress jika ada button yang ditekan
 */
inline void updateButtonPressIfAnyPressed(unsigned long currentMillis, unsigned long& lastButtonPress) {
  if (UP() || DOWN() || LEFT() || RIGHT() || START() || STOP()) {
    lastButtonPress = currentMillis;
  }
}

// ===================================================================
// REMOVED: initMenuTempVariables() function
// Sekarang langsung edit variabel asli tanpa temporary variables
// ===================================================================

/**
 * Check dan handle exit dari AGV mode
 * @return true jika masih dalam AGV mode, false jika sudah keluar
 */
bool handleAgvModeExit() {
  if (!isAgvMode) return false;
  
  if (STOP()) {
    isAgvMode = false;
    currentMenu = MENU_MAIN;
    agvMode(AGV_STATE_STOP);
    resetDisplayFlags();
    newRfidScanned = false;
    menuNeedsRefresh = true;
  }
  return true; // Masih dalam AGV mode
}

/**
 * Handle navigasi UP/DOWN di main menu
 * @param currentMillis Current timestamp
 * @param lastButtonPress Reference to last button press time
 * @return true jika ada input navigasi
 */
bool handleMainMenuNavigation(unsigned long currentMillis, unsigned long& lastButtonPress) {
  if (currentMillis - lastButtonPress < buttonDelay) return false;
  
  if (UP()) {
    selectedItem = (selectedItem - 1 + maxItems) % maxItems;
    lastButtonPress = currentMillis;
    return true;
  } 
  
  if (DOWN()) {
    selectedItem = (selectedItem + 1) % maxItems;
    lastButtonPress = currentMillis;
    return true;
  }
  
  if (STOP()) {
    currentMenu = MENU_MAIN;
    selectedItem = 0;
    menuNeedsRefresh = true;
    lastButtonPress = currentMillis;
    return true;
  }
  
  return false;
}

/**
 * Handle pemilihan item di main menu (tombol START)
 * @param currentMillis Current timestamp
 * @param lastButtonPress Reference to last button press time
 */
void handleMainMenuSelection(unsigned long currentMillis, unsigned long& lastButtonPress) {
  if (currentMillis - lastButtonPress < buttonDelay) return;
  if (!START()) return;
  
  // Map selectedItem to corresponding menu constants
  switch (selectedItem) {
    case 0:  // AGV Mode
      isAgvMode = true;
      newRfidScanned = false;
      menuStartIndex = 0;
      menuNeedsRefresh = true;
      break;
      
    case 1:  // Reset AGV State
      currentMenu = MENU_RESET_AGV_STATE;
      menuNeedsRefresh = true;
      break;
      
    case 2:  // Motor Test
      selectedItem = 0;
      currentMenu = MENU_MOTOR_TEST;
      menuNeedsRefresh = true;
      break;
      
    case 3:  // PID Settings
          currentMenu = MENU_PID_SETTINGS;
      menuNeedsRefresh = true;
      break;
      
    case 4:  // Target Settings
      currentMenu = MENU_TARGET_SETTINGS;
      menuNeedsRefresh = true;
      break;
      
    case 5:  // Reset Settings
      currentMenu = MENU_RESET;
      menuNeedsRefresh = true;
      break;
      
    case 6:  // RFID Settings
      currentMenu = MENU_RFID_SETTINGS;
      menuNeedsRefresh = true;
      break;
      
    case 7:  // Motor Settings
      currentMenu = MENU_MOTOR_SETTINGS;
      selectedItem = 0;
      menuNeedsRefresh = true;
      break;
      
    case 8:  // Motor Invert
      currentMenu = MENU_MOTOR_INVERT;
      menuNeedsRefresh = true;
      break;
      
    case 9:  // Music Settings
      currentMenu = MENU_MUSIC_SETTINGS;
      menuNeedsRefresh = true;
      break;
      
    case 10:  // Music Test
      currentMenu = MENU_MUSIC_TEST;
      menuNeedsRefresh = true;
      break;
      
    case 11:  // Hook Test
      currentMenu = MENU_HOOK_TEST;
      menuNeedsRefresh = true;
      break;
      
    case 12:  // Magnet Check
      currentMenu = MENU_MAGNET_CHECK;
      menuNeedsRefresh = true;
      break;
      
    case 13:  // Ultrasonic Check
      currentMenu = MENU_ULTRASONIC_CHECK;
      menuNeedsRefresh = true;
      break;
      
    case 14:  // Ultrasonic Settings
          selectedItem = 0;
      currentMenu = MENU_ULTRASONIC_SETTINGS;
      menuNeedsRefresh = true;
      break;
      
    case 15:  // WiFi Settings
      currentMenu = MENU_WIFI_SETTINGS;
      menuNeedsRefresh = true;
      break;
      
    case 16:  // Tuning RPM
      currentMenu = MENU_RPM_TUNING;
      selectedTuningItem = 0;
      menuNeedsRefresh = true;
      break;
      
    default:
      currentMenu = MENU_MAIN;
      menuNeedsRefresh = true;
      break;
  }
  lastButtonPress = currentMillis;
}

/**
 * Handle Motor Invert menu logic
 * Extract dari handleMenu() untuk mengurangi complexity
 */
void handleMotorInvertMenu(unsigned long currentMillis, unsigned long& lastButtonPress) {
  static bool displayInitialized;
  static int lastSelectedInvertItem = -1;
  static bool lastInvertValues[5];
  
  // Check if display needs refresh
  bool needsRefresh = !displayInitialized || 
                     selectedInvertItem != lastSelectedInvertItem ||
                     invertMotorY != lastInvertValues[0] ||
                     invertMotorX != lastInvertValues[1] ||
                     invertMotorKanan != lastInvertValues[2] ||
                     invertMotorKiri != lastInvertValues[3] ||
                     invertHook != lastInvertValues[4];
  
  if (needsRefresh) {
    displayMenuHeader("Motor Invert");

    // Calculate what items to show (3 items max, with scrolling)
    int startIdx = max(0, min(selectedInvertItem - 1, maxInvertItems - 3));

    String invertLabels[5] = { "Y-Axis", "X-Axis", "M-Kanan", "M-Kiri", "Hook" };
    bool* invertValues[5] = { &invertMotorY, &invertMotorX, &invertMotorKanan, &invertMotorKiri, &invertHook };

    for (int i = 0; i < 3 && (startIdx + i) < maxInvertItems; i++) {
      int itemIndex = startIdx + i;
      lcd.setCursor(0, i + 1);
      lcd.print("                    ");
      lcd.setCursor(0, i + 1);

      // Show cursor for selected item
      lcd.print(itemIndex == selectedInvertItem ? "> " : "  ");
      
      // Show label and value
      lcd.print(invertLabels[itemIndex]);
      lcd.print(": ");
      lcd.print(*invertValues[itemIndex] ? "Yes" : "No");

      // Show controls on the right
      lcd.setCursor(12, i + 1);
      if (i == 0) lcd.print("UP/DN:Nav");
      else if (i == 1) lcd.print("LF/RT:Set");
      else if (i == 2) lcd.print("A:OK B:Back");
    }
    
    // Update tracking variables
    displayInitialized = true;
    lastSelectedInvertItem = selectedInvertItem;
    lastInvertValues[0] = invertMotorY;
    lastInvertValues[1] = invertMotorX;
    lastInvertValues[2] = invertMotorKanan;
    lastInvertValues[3] = invertMotorKiri;
    lastInvertValues[4] = invertHook;
  }

  if (currentMillis - lastButtonPress < buttonDelay) return;
  
  if (UP()) {
    selectedInvertItem = (selectedInvertItem - 1 + maxInvertItems) % maxInvertItems;
    lastButtonPress = currentMillis;
  } else if (DOWN()) {
    selectedInvertItem = (selectedInvertItem + 1) % maxInvertItems;
    lastButtonPress = currentMillis;
  } else if (LEFT() || RIGHT()) {
    // Toggle selected item
    switch (selectedInvertItem) {
      case 0: invertMotorY = !invertMotorY; break;
      case 1: invertMotorX = !invertMotorX; break;
      case 2: invertMotorKanan = !invertMotorKanan; break;
      case 3: invertMotorKiri = !invertMotorKiri; break;
      case 4: invertHook = !invertHook; break;
    }
    lastButtonPress = currentMillis;
  } else if (START()) {
    // Save all settings
    invertMotorY = invertMotorY;
    invertMotorX = invertMotorX;
    invertMotorKanan = invertMotorKanan;
    invertMotorKiri = invertMotorKiri;
    invertHook = invertHook;
    saveSettings();
    // Reset and exit
    displayInitialized = false;
    currentMenu = MENU_MAIN;
    selectedInvertItem = 0;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
    lastButtonPress = currentMillis;
  } else if (STOP()) {
    // Cancel changes
    invertMotorY = invertMotorY;
    invertMotorX = invertMotorX;
    invertMotorKanan = invertMotorKanan;
    invertMotorKiri = invertMotorKiri;
    invertHook = invertHook;
    // Reset and exit
    displayInitialized = false;
    currentMenu = MENU_MAIN;
    selectedInvertItem = 0;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
    lastButtonPress = currentMillis;
  }
}

// ===================================================================
// REUSABLE BUTTON HANDLER - Menghilangkan duplikasi 1280+ baris
// ===================================================================

/**
 * Helper function untuk handle button adjustment dengan hold detection
 * Menggantikan 8+ fungsi duplikat dengan 1 fungsi reusable
 * @param values Array of 3 double pointers untuk Kp, Ki, Kd
 * @param selectedParam Index parameter yang dipilih (0=Kp, 1=Ki, 2=Kd)
 * @param smallIncrement Increment untuk single click (default 0.01)
 * @param largeIncrement Increment untuk hold (default 0.1)
 * @param holdInterval Interval update saat hold dalam ms (default 500)
 */
void handlePidButtonAdjustment(double* values[3], int selectedParam, 
                               float smallIncrement = 0.01f, 
                               float largeIncrement = 0.1f,
                               unsigned long holdInterval = 500) {
  // Static variables auto-initialize to 0/false - explicit init redundant
  static unsigned long lastRightPress;
  static unsigned long lastLeftPress;
  static unsigned long rightHoldStart;
  static unsigned long leftHoldStart;
  static bool rightHolding;
  static bool leftHolding;
  
  unsigned long currentMillis = millis();
  
  // Handle RIGHT button (increment)
  if (digitalRead(rightPin) == HIGH) {
    if (!rightHolding) {
      // Button just pressed - single click increment
      rightHoldStart = currentMillis;
      rightHolding = true;
      *values[selectedParam] += smallIncrement;
      lastRightPress = currentMillis;
    } else if (currentMillis - rightHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - larger increment
      if (currentMillis - lastRightPress >= holdInterval) {
        *values[selectedParam] += largeIncrement;
        lastRightPress = currentMillis;
      }
    }
  } else {
    rightHolding = false;
  }
  
  // Handle LEFT button (decrement)
  if (digitalRead(leftPin) == HIGH) {
    if (!leftHolding) {
      // Button just pressed - single click decrement
      leftHoldStart = currentMillis;
      leftHolding = true;
      *values[selectedParam] = max(0.0, *values[selectedParam] - smallIncrement);
      lastLeftPress = currentMillis;
    } else if (currentMillis - leftHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - larger decrement
      if (currentMillis - lastLeftPress >= holdInterval) {
        *values[selectedParam] = max(0.0, *values[selectedParam] - largeIncrement);
        lastLeftPress = currentMillis;
      }
    }
  } else {
    leftHolding = false;
  }
}

// Global display functions
void displayIndicator(int current, int selected) {
  if (current == selected) {
    lcd.print("> ");
  } else {
    lcd.print("  ");
  }
}

void displayMenuHeader(const char* title) {
  // lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(title);
}

void displayMenuFooter(const char* text) {
  lcd.setCursor(0, 3);
  lcd.print(text);
}

// ===================================================================
// SETTINGS MANAGEMENT - Fix #6: Split long functions
// ===================================================================

/**
 * Save PID settings to NVS
 */
void savePidSettings() {
  // Save Line Follower PID
  preferences.putDouble("kpLinefollower", kp);
  preferences.putDouble("kiLinefollower", ki);
  preferences.putDouble("kdLinefollower", kd);
  
  // Save Forward PID WithMassa
  preferences.putDouble("kpFwdMassa", kpForwardWithMassa);
  preferences.putDouble("kiFwdMassa", kiForwardWithMassa);
  preferences.putDouble("kdFwdMassa", kdForwardWithMassa);
  
  // Save Forward PID Default (with corruption fix)
  preferences.remove("kpFwdDefault");
  delay(10);
  preferences.putDouble("kpFwdDefault", kpForwardDefault);
  
  preferences.remove("kiFwdDefault");
  delay(10);
  preferences.putDouble("kiFwdDefault", kiForwardDefault);
  
  preferences.remove("kdFwdDefault");
  delay(10);
  preferences.putDouble("kdFwdDefault", kdForwardDefault);
  
  // Save Backward PID WithMassa
  preferences.putDouble("kpBwdMassa", kpBackwardWithMassa);
  preferences.putDouble("kiBwdMassa", kiBackwardWithMassa);
  preferences.putDouble("kdBwdMassa", kdBackwardWithMassa);
  
  // Save Backward PID Default
  preferences.putDouble("kpBwdDefault", kpBackwardDefault);
  preferences.putDouble("kiBwdDefault", kiBackwardDefault);
  preferences.putDouble("kdBwdDefault", kdBackwardDefault);
}

/**
 * Save motor settings to NVS
 */
void saveMotorSettings() {
  // Base speed
  preferences.putInt("baseSpeed", baseSpeed);
  
  // Motor invert flags
  preferences.putBool("invertY", invertMotorY);
  preferences.putBool("invertX", invertMotorX);
  preferences.putBool("invertKanan", invertMotorKanan);
  preferences.putBool("invertKiri", invertMotorKiri);
  preferences.putBool("invertHook", invertHook);
  
  // Motor Control PID
  preferences.putInt("maxMotorRpm", maxMotorRpm);
  preferences.putDouble("motorPidKp", motorPidKp);
  preferences.putDouble("motorPidKi", motorPidKi);
  preferences.putDouble("motorPidKd", motorPidKd);
  
  // Individual motor PID
  preferences.putDouble("motorKpR", motorPidKpRight);
  preferences.putDouble("motorKiR", motorPidKiRight);
  preferences.putDouble("motorKdR", motorPidKdRight);
  preferences.putDouble("motorKpL", motorPidKpLeft);
  preferences.putDouble("motorKiL", motorPidKiLeft);
  preferences.putDouble("motorKdL", motorPidKdLeft);
}

/**
 * Save peripheral settings to NVS
 */
void savePeripheralSettings() {
  // Music pins
  preferences.putInt("musicOn", musicOnPin);
  preferences.putInt("musicObstacle", musicObstaclePin);
  preferences.putInt("musicStation", musicStationPin);
  preferences.putInt("musicOutOfLine", musicOutOfLinePin);
  preferences.putInt("musicWarning", musicWarningPin);
  
  // Ultrasonic settings
  preferences.putUShort("SafeDistFront", minSafeDistanceFront);
  preferences.putUShort("SafeDistBack", minSafeDistanceBack);
}

/**
 * Main save settings function - coordinates all saves
 * REMOVED: applySettings() - tidak perlu lagi karena langsung edit variabel asli
 */
void saveSettings() {
  Serial.println("\n=== SAVING SETTINGS ===");
  
  if (!preferences.begin("agv-settings", false)) {
    logError(ERROR_INVALID_CONFIGURATION, "Failed to open preferences for writing");
    return;
  }
  
  // Check free entries
  size_t freeEntries = preferences.freeEntries();
  Serial.print("NVS Free Entries: ");
  Serial.println(freeEntries);
  
  delay(NVS_WRITE_DELAY);
  
  // Save all settings in logical groups
  savePidSettings();
  saveMotorSettings();
  savePeripheralSettings();
  
  // Verify critical settings
  double verifyKp = preferences.getDouble("kpFwdDefault", -1.0);
  Serial.print("VERIFY - kpFwdDefault: ");
  Serial.println(verifyKp, 2);
  
  if (abs(verifyKp - kpForwardDefault) > 0.01) {
    String msg = "Value mismatch - Expected: " + String(kpForwardDefault, 2) + 
                 ", Got: " + String(verifyKp, 2);
    logError(ERROR_INVALID_CONFIGURATION, msg);
  }
  
  preferences.end();
  Serial.println("=== SETTINGS SAVED SUCCESSFULLY ===\n");
}

// ===================================================================
// DISPLAY FUNCTIONS
// ===================================================================


void displayMainMenu() {
  // Menu items array
  String menuItems[17] = {
    "AGV Mode",           // selectedItem 0 -> MENU_AGV_MODE (1)
    "Reset AGV State",    // selectedItem 1 -> MENU_RESET_AGV_STATE (25)
    "Motor Test",         // selectedItem 2 -> MENU_MOTOR_TEST (2)
    "PID Settings",       // selectedItem 3 -> MENU_PID_SETTINGS (3)
    "Target Settings",    // selectedItem 4 -> MENU_TARGET_SETTINGS (4)
    "Reset Settings",     // selectedItem 5 -> MENU_RESET (5)
    "RFID Settings",      // selectedItem 6 -> MENU_RFID_SETTINGS (6)
    "Motor Settings",     // selectedItem 7 -> MENU_MOTOR_SETTINGS (18) - Updated to show sub menu
    "Motor Invert",       // selectedItem 8 -> MENU_MOTOR_INVERT (19)
    "Music Settings",     // selectedItem 9 -> MENU_MUSIC_SETTINGS (20)
    "Music Test",         // selectedItem 10 -> MENU_MUSIC_TEST (21)
    "Hook Test",          // selectedItem 11 -> MENU_HOOK_TEST (22)
    "Magnet Check",       // selectedItem 12 -> MENU_MAGNET_CHECK (23)
    "Ultrasonic Check",   // selectedItem 13 -> MENU_ULTRASONIC_CHECK (24)
    "Ultrasonic Settings", // selectedItem 14 -> MENU_ULTRASONIC_SETTINGS (30)
    "WiFi Settings",      // selectedItem 15 -> MENU_WIFI_SETTINGS (14)
    "Tuning RPM"          // selectedItem 16 -> MENU_RPM_TUNING (55)
  };
  maxItems = sizeof(menuItems) / sizeof(menuItems[0]);
  // Update scroll position if needed
  if (selectedItem < menuStartIndex) {
    menuStartIndex = selectedItem;
    menuNeedsRefresh = true;
  } else if (selectedItem >= menuStartIndex + maxMenuDisplay) {
    menuStartIndex = selectedItem - maxMenuDisplay + 1;
    menuNeedsRefresh = true;
  }

  // Check if we need to refresh the entire display
  if (menuNeedsRefresh || selectedItem != lastSelectedItem || menuStartIndex != lastMenuStartIndex) {
    lcd.clear();  // Only clear when really needed
    displayMenuHeader("AGV Menu:");

    // Display menu items (3 items max)
    for (int i = 0; i < maxMenuDisplay && (menuStartIndex + i) < maxItems; i++) {
      int menuIndex = menuStartIndex + i;
      lcd.setCursor(0, i + 1);

      // Show cursor for selected item
      if (menuIndex == selectedItem) {
        lcd.print("> ");
      } else {
        lcd.print("  ");
      }

      // Print menu item (max 16 chars to fit cursor)
      String item = menuItems[menuIndex];
      if (item.length() > 16) {
        item = item.substring(0, 16);
      }
      lcd.print(item);

      // Show item number
      lcd.setCursor(18, i + 1);
      lcd.print(menuIndex + 1);
    }

    // Show scroll indicators
    lcd.setCursor(19, 1);
    lcd.setCursor(19, 3);

    // Update last states
    lastSelectedItem = selectedItem;
    lastMenuStartIndex = menuStartIndex;
    menuNeedsRefresh = false;
  } else {
    // Quick cursor update without full refresh
    for (int i = 0; i < maxMenuDisplay && (menuStartIndex + i) < maxItems; i++) {
      int menuIndex = menuStartIndex + i;
      lcd.setCursor(0, i + 1);

      if (menuIndex == selectedItem) {
        lcd.print(">");
      } else {
        lcd.print(" ");
      }
    }
  }
}
void handleMenu() {
  unsigned long currentMillis = millis();

  // Check AGV mode exit - early return pattern
  if (handleAgvModeExit()) return;

  switch (currentMenu) {
    case MENU_MAIN:
      displayMainMenu();
      rpmMotor(0, 0);
      Serial.println("STOP");
      stopMusic();
      digitalWrite(lampPin, HIGH);
      
      // Handle navigation (UP/DOWN/STOP)
      if (!handleMainMenuNavigation(currentMillis, lastButtonPress)) {
        // Handle selection (START) hanya jika tidak ada navigasi
        handleMainMenuSelection(currentMillis, lastButtonPress);
      }
      break;
    case MENU_MOTOR_TEST:
      displayMotorTest();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleMotorTest();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_MOTOR_TEST_PWM:
      displayMotorTestPWM();
      handleMotorTestPWM();
      break;

    case MENU_MOTOR_TEST_RPM:
      // requestRpmDataFromSlave();
      displayMotorTestRPM();
      handleMotorTestRPM();
      break;

    case MENU_PID_SETTINGS:
      displayPidSubmenu();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handlePidSubmenu();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_PID_FORWARD:
      displayPidForwardSubmenu();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handlePidForwardSubmenu();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_PID_BACKWARD:
      displayPidBackwardSubmenu();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handlePidBackwardSubmenu();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_PID_FORWARD_WITHMASSA:
      displayPidForwardWithMassaSettings();
      handlePidForwardWithMassaSettings();
      break;

    case MENU_PID_FORWARD_DEFAULT:
      displayPidForwardDefaultSettings();
      handlePidForwardDefaultSettings();
      break;

    case MENU_PID_BACKWARD_WITHMASSA:
      displayPidBackwardWithMassaSettings();
      handlePidBackwardWithMassaSettings();
      break;

    case MENU_PID_BACKWARD_DEFAULT:
      displayPidBackwardDefaultSettings();
      handlePidBackwardDefaultSettings();
      break;

    case MENU_TARGET_SETTINGS:
      displayTargetSettings();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleTargetSettings();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_RFID_SETTINGS:
      displayRfidSettings();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleRfidSettings();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_MOTOR_SETTINGS:
      displayMotorSettings();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleMotorSettings();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_SPEED_SETTING:
      displaySpeedSetting();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleSpeedSetting();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_PID_RPM_SETTING:
      displayPidRpmSetting();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handlePidRpmSetting();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_PID_RPM_RIGHT:
      displayPidRpmRight();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handlePidRpmRight();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_PID_RPM_LEFT:
      displayPidRpmLeft();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handlePidRpmLeft();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_MOTOR_INVERT:
      handleMotorInvertMenu(currentMillis, lastButtonPress);
      break;

    case MENU_MUSIC_SETTINGS:
      displayMusicSettings();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleMusicSettings();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_MUSIC_TEST:
      displayMusicTest();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleMusicTest();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_MUSIC_ON:
      displayMusicSubmenu("On Music", &musicOnPin);
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleMusicSubmenu(&musicOnPin);
      break;

    case MENU_MUSIC_OBSTACLE:
      displayMusicSubmenu("Obstacle Music", &musicObstaclePin);
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleMusicSubmenu(&musicObstaclePin);
      break;

    case MENU_MUSIC_STATION:
      displayMusicSubmenu("Station Music", &musicStationPin);
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleMusicSubmenu(&musicStationPin);
      break;

    case MENU_MUSIC_OUTOFLINE:
      displayMusicSubmenu("OutOfLine Music", &musicOutOfLinePin);
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleMusicSubmenu(&musicOutOfLinePin);
      break;

    case MENU_MUSIC_WARNING:
      displayMusicSubmenu("Warning Music", &musicWarningPin);
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleMusicSubmenu(&musicWarningPin);
      break;

    case MENU_HOOK_TEST:
      displayHookTest();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleHookTest();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_RESET:
      displayResetMenu();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleResetMenu();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;
      
    case MENU_RESET_AGV_STATE:
      displayResetAgvStateMenu();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleResetAgvStateMenu();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_MAGNET_CHECK:
      displayMagnetCheck();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleMagnetCheck();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;
 
    case MENU_ULTRASONIC_CHECK:
      checkObstacles();
      displayUltrasonicCheck();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleUltrasonicCheck();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_ULTRASONIC_SETTINGS:
      displayUltrasonicSettings();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleUltrasonicSettings();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_ULTRASONIC_FRONT:
      displayUltrasonicFrontSettings();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleUltrasonicFrontSettings();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_ULTRASONIC_BACK:
      displayUltrasonicBackSettings();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleUltrasonicBackSettings();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_ULTRASONIC_FRONT_TENGAH:
      displayUltrasonicFrontTengahSettings();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleUltrasonicFrontTengahSettings();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_ULTRASONIC_FRONT_SERONG:
      displayUltrasonicFrontSerongSettings();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleUltrasonicFrontSerongSettings();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_ULTRASONIC_BACK_TENGAH:
      displayUltrasonicBackTengahSettings();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleUltrasonicBackTengahSettings();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_ULTRASONIC_BACK_SERONG:
      displayUltrasonicBackSerongSettings();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleUltrasonicBackSerongSettings();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_WIFI_SETTINGS:
      displayWifiSettings();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleWifiSettings();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_RPM_TUNING:
      displayRpmTuningMenu();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleRpmTuningMenu();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_RPM_TUNE_STATUS:
      displayTuningStatus();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      if (STOP()) {
        currentMenu = MENU_RPM_TUNING;
        selectedTuningItem = 0;
        menuNeedsRefresh = true;
        lastButtonPress = currentMillis;
      }
      break;

    case MENU_RPM_TUNE_START:
      displayTuningStartMenu("Both Motors");
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleTuningStartMenu("TUNE");
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_RPM_TUNE_RIGHT:
      displayTuningStartMenu("Right Motor");
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleTuningStartMenu("RIGHT_TUNE");
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_RPM_TUNE_LEFT:
      displayTuningStartMenu("Left Motor");
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleTuningStartMenu("LEFT_TUNE");
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_RFID_UJUNG:
      displayRfidUjung();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleRfidUjung();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_RFID_WAREHOUSE:
      displayRfidWarehouse();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleRfidWarehouse();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;

    case MENU_AUTO_INPUT_STATION:
      displayAutoInputStation();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleAutoInputStation();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;
      
    case MENU_TERMINAL_DROP:
      displayTerminalDrop();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleTerminalDrop();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;
      
    case MENU_TERMINAL_PICKUP:
      displayTerminalPickup();
      CHECK_BUTTON_TIMING(currentMillis, lastButtonPress);
      handleTerminalPickup();
      updateButtonPressIfAnyPressed(currentMillis, lastButtonPress);
      break;
      
  }
}
// ====== MOTOR TEST SUBMENU FUNCTIONS ======
void displayMotorTest() {
  static int lastSelectedItem = -1;  // -1 needs explicit init
  
  if (menuNeedsRefresh || selectedItem != lastSelectedItem) {
    lcd.clear();
    displayMenuHeader("Motor Test Mode:");
    
    String menuItems[2] = {
      "PWM Control",       // selectedItem 0 -> MENU_MOTOR_TEST_PWM (49)
      "RPM Control"        // selectedItem 1 -> MENU_MOTOR_TEST_RPM (50)
    };
    
    for (int i = 0; i < 2; i++) {
      lcd.setCursor(0, i + 1);
      displayIndicator(i, selectedItem);
      lcd.print(menuItems[i]);
    }
    
    displayMenuFooter("A:Select B:Back");
    
    menuNeedsRefresh = false;
    lastSelectedItem = selectedItem;
  }
}

void handleMotorTest() {
  if (UP()) {
    selectedItem = (selectedItem - 1 + 2) % 2;
    menuNeedsRefresh = true;
  } else if (DOWN()) {
    selectedItem = (selectedItem + 1) % 2;
    menuNeedsRefresh = true;
  } else if (START()) {
    lcd.clear();
    motorTestState = 0;  // Reset motor state
    switch (selectedItem) {
      case 0:  // PWM Control
        lcd.clear();
        currentMenu = MENU_MOTOR_TEST_PWM;
        break;
      case 1:  // RPM Control
        lcd.clear();
        currentMenu = MENU_MOTOR_TEST_RPM;
        break;
    }
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Back to main menu
    lcd.clear();
    currentMenu = MENU_MAIN;
    selectedItem = 2;  // Return to Motor Test item
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}

void displayMotorTestPWM() {
  displayMenuHeader("Motor Test (PWM)");

  lcd.setCursor(0, 1);
  lcd.print("UP:Maju DOWN:Mundur");
  lcd.setCursor(0, 2);
  lcd.print("LF:Kiri RT:Kanan");
  lcd.setCursor(0, 3);

  // Show current motor state with PWM info
  switch (motorTestState) {
    case 0: lcd.print("Status: STOP    "); break;
    case 1: lcd.print("Status: MAJU PWM"); break;
    case 2: lcd.print("Status: MNDR PWM"); break;  
    case 3: lcd.print("Status: KIRI PWM"); break;
    case 4: lcd.print("Status: KNAN PWM"); break;
  }

  displayMenuFooter("B:Back");
}

void handleMotorTestPWM() {
  if (UP()) {
    motorTestState = 1;  // Set to forward
  } else if (DOWN()) {
    motorTestState = 2;  // Set to backward
  } else if (LEFT()) {
    motorTestState = 3;  // Set to left
  } else if (RIGHT()) {
    motorTestState = 4;  // Set to right
  } else if (STOP()) {
    motorTestState = 0;  // Stop motor
    pwmMotor(0, 0);  // Use PWM stop command
    lcd.clear();
    currentMenu = MENU_MOTOR_TEST;
    selectedItem = 0;  // Return to PWM selection
    menuStartIndex = 0;
    menuNeedsRefresh = true;
    return;
  }

  // Execute continuous motor movement based on state - PWM mode
  switch (motorTestState) {
    case 0:  // STOP
      pwmMotor(0, 0);  // Use PWM command
      break;
    case 1:  // FORWARD
      pwmMotor(testMotorSpeed, testMotorSpeed);  // Both motors forward
      break;
    case 2:  // BACKWARD  
      pwmMotor(-testMotorSpeed, -testMotorSpeed);  // Both motors backward
      break;
    case 3:  // LEFT
      pwmMotor(testMotorSpeed, -testMotorSpeed);  // Left motor backward, right forward
      break;
    case 4:  // RIGHT
      pwmMotor(-testMotorSpeed, testMotorSpeed);  // Left motor forward, right backward
      break;
  }
}

void displayMotorTestRPM() {
  displayMenuHeader("Motor Test (RPM)");

  lcd.setCursor(0, 1);
  lcd.print("UP:Maju DOWN:Mundur");
  lcd.setCursor(0, 2);
  lcd.print("LF:Kiri RT:Kanan");
  lcd.setCursor(0, 3);
  lcd.print("R: ");
  lcd.print(currentRpmKanan);
  lcd.print(", L: ");
  lcd.print(currentRpmKiri);
}

void handleMotorTestRPM() {
  // // Request RPM data from slave periodically

    requestRpmDataFromSlave();  // Request current RPM values

  
  if (UP()) {
    motorTestState = 1;  // Set to forward
  } else if (DOWN()) {
    motorTestState = 2;  // Set to backward
  } else if (LEFT()) {
    motorTestState = 3;  // Set to left
  } else if (RIGHT()) {
    motorTestState = 4;  // Set to right
  } else if (STOP()) {
    motorTestState = 0;  // Stop motor
    pwmMotor(0, 0);  // Use PWM stop command
    lcd.clear();
    currentMenu = MENU_MOTOR_TEST;
    selectedItem = 1;  // Return to RPM selection
    menuStartIndex = 0;
    menuNeedsRefresh = true;
    return;
  }

  // Execute continuous motor movement based on state - RPM mode
  switch (motorTestState) {
    case 0:  // STOP
      rpmMotor(0, 0);  // Use RPM command
      break;
    case 1:  // FORWARD
      rpmMotor(maxMotorRpm, maxMotorRpm);  // Both motors forward
      break;
    case 2:  // BACKWARD
      rpmMotor(-maxMotorRpm, -maxMotorRpm);  // Both motors backward
      break;
    case 3:  // LEFT
      rpmMotor(maxMotorRpm, -maxMotorRpm);  // Left motor backward, right forward
      break;
    case 4:  // RIGHT
      rpmMotor(-maxMotorRpm, maxMotorRpm);  // Left motor forward, right backward
      break;
  }
}

// PID Submenu Functions
void displayPidSubmenu() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("PID Settings");

  // Display Forward option
  lcd.setCursor(0, 1);
  if (selectedParam == 0) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Forward PID");

  // Display Backward option
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Backward PID");

  // Show controls
  lcd.setCursor(0, 3);
  lcd.print("A:Select B:Back");
}

void handlePidSubmenu() {
  if (UP()) {
    selectedParam = (selectedParam - 1 + 2) % 2;
  } else if (DOWN()) {
    selectedParam = (selectedParam + 1) % 2;
  } else if (START()) {
    if (selectedParam == 0) {
      // Forward PID
      lcd.clear();
          currentMenu = MENU_PID_FORWARD;
      selectedParam = 0; // Reset for PID parameter selection
      menuNeedsRefresh = true;
    } else if (selectedParam == 1) {
      // Backward PID
      lcd.clear();
          currentMenu = MENU_PID_BACKWARD;
      selectedParam = 0; // Reset for PID parameter selection
      menuNeedsRefresh = true;
    }
  } else if (STOP()) {
    // Save settings before going back to main menu
    saveSettings();
    currentMenu = MENU_MAIN;
    selectedParam = 0;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}



// PID Forward Submenu Functions
void displayPidForwardSubmenu() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("PID Forward");

  // Display WithMassa option
  lcd.setCursor(0, 1);
  if (selectedParam == 0) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("WithMassa");

  // Display Default option
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Default");

  // Show controls
  lcd.setCursor(0, 3);
  lcd.print("A:Select B:Back");
}

void handlePidForwardSubmenu() {
  if (UP()) {
    selectedParam = (selectedParam - 1 + 2) % 2;
  } else if (DOWN()) {
    selectedParam = (selectedParam + 1) % 2;
  } else if (START()) {
    if (selectedParam == 0) {
      // WithMassa PID
      lcd.clear();
      // initMenuTempVariables() already called in parent menu
      currentMenu = MENU_PID_FORWARD_WITHMASSA;
      selectedParam = 0; // Reset for PID parameter selection
      menuNeedsRefresh = true;
    } else if (selectedParam == 1) {
      // Default PID
      lcd.clear();
      // initMenuTempVariables() already called in parent menu
      currentMenu = MENU_PID_FORWARD_DEFAULT;
      selectedParam = 0; // Reset for PID parameter selection
      menuNeedsRefresh = true;
    }
  } else if (STOP()) {
    // Save settings before going back
    saveSettings();
    currentMenu = MENU_PID_SETTINGS;
    selectedParam = 0;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}

// PID Backward Submenu Functions
void displayPidBackwardSubmenu() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("PID Backward");

  // Display WithMassa option
  lcd.setCursor(0, 1);
  if (selectedParam == 0) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("WithMassa");

  // Display Default option
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Default");

  // Show controls
  lcd.setCursor(0, 3);
  lcd.print("A:Select B:Back");
}

void handlePidBackwardSubmenu() {
  if (UP()) {
    selectedParam = (selectedParam - 1 + 2) % 2;
  } else if (DOWN()) {
    selectedParam = (selectedParam + 1) % 2;
  } else if (START()) {
    if (selectedParam == 0) {
      // WithMassa PID
      lcd.clear();
      // initMenuTempVariables() already called in parent menu
      currentMenu = MENU_PID_BACKWARD_WITHMASSA;
      selectedParam = 0; // Reset for PID parameter selection
      menuNeedsRefresh = true;
    } else if (selectedParam == 1) {
      // Default PID
      lcd.clear();
      // initMenuTempVariables() already called in parent menu
      currentMenu = MENU_PID_BACKWARD_DEFAULT;
      selectedParam = 0; // Reset for PID parameter selection
      menuNeedsRefresh = true;
    }
  } else if (STOP()) {
    // Save settings before going back
    saveSettings();
    currentMenu = MENU_PID_SETTINGS;
    selectedParam = 0;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}

// PID Forward WithMassa Settings Functions
void displayPidForwardWithMassaSettings() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("PID Fwd WithMassa");

  // Display Kp
  lcd.setCursor(0, 1);
  if (selectedParam == 0) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kp: ");
  lcd.print(kpForwardWithMassa);
  lcd.print("       ");

  // Display Ki
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Ki: ");
  lcd.print(kiForwardWithMassa);
  lcd.print("       ");

  // Display Kd
  lcd.setCursor(0, 3);
  if (selectedParam == 2) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kd: ");
  lcd.print(kdForwardWithMassa);
  lcd.print("       ");

  // Show controls
  lcd.setCursor(12, 3);
  lcd.print("B:OK");
}

void handlePidForwardWithMassaSettings() {
  // Handle UP/DOWN navigation
  if (UP()) {
    selectedParam = (selectedParam - 1 + 3) % 3;
  } else if (DOWN()) {
    selectedParam = (selectedParam + 1) % 3;
  }

  // Create array of pointers to PID values
  double* pidValues[3] = {
    &kpForwardWithMassa,
    &kiForwardWithMassa,
    &kdForwardWithMassa
  };
  
  // Use reusable button handler (0.1 increment, 1.0 hold, 100ms interval)
  handlePidButtonAdjustment(pidValues, selectedParam, 0.1f, 1.0f, 100);

  // Handle back button
  if (STOP()) {
    saveSettings();
    currentMenu = MENU_PID_FORWARD;
    selectedParam = 0;
    menuNeedsRefresh = true;
  }
}

// PID Forward Default Settings Functions
void displayPidForwardDefaultSettings() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("PID Fwd Default");

  // Display Kp
  lcd.setCursor(0, 1);
  if (selectedParam == 0) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kp: ");
  lcd.print(kpForwardDefault);
  lcd.print("       ");

  // Display Ki
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Ki: ");
  lcd.print(kiForwardDefault);
  lcd.print("       ");

  // Display Kd
  lcd.setCursor(0, 3);
  if (selectedParam == 2) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kd: ");
  lcd.print(kdForwardDefault);
  lcd.print("       ");

  // Show controls
  lcd.setCursor(12, 3);
  lcd.print("B:OK");
}

void handlePidForwardDefaultSettings() {
  // Handle UP/DOWN navigation
  if (UP()) {
    selectedParam = (selectedParam - 1 + 3) % 3;
  } else if (DOWN()) {
    selectedParam = (selectedParam + 1) % 3;
  }

  // Create array of pointers to PID values
  double* pidValues[3] = {
    &kpForwardDefault,
    &kiForwardDefault,
    &kdForwardDefault
  };
  
  // Use reusable button handler (0.01 increment, 0.1 hold, 500ms interval)
  handlePidButtonAdjustment(pidValues, selectedParam, 0.01f, 0.1f, 200);

  // Handle back button
  if (STOP()) {
    saveSettings();
    delay(BUTTON_DEBOUNCE_DELAY);
    currentMenu = MENU_PID_FORWARD;
    selectedParam = 0;
    menuNeedsRefresh = true;
  }
}

// PID Backward WithMassa Settings Functions
void displayPidBackwardWithMassaSettings() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("PID Bwd WithMassa");

  // Display Kp
  lcd.setCursor(0, 1);
  if (selectedParam == 0) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kp: ");
  lcd.print(kpBackwardWithMassa);
  lcd.print("       ");

  // Display Ki
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Ki: ");
  lcd.print(kiBackwardWithMassa);
  lcd.print("       ");

  // Display Kd
  lcd.setCursor(0, 3);
  if (selectedParam == 2) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kd: ");
  lcd.print(kdBackwardWithMassa);
  lcd.print("       ");

  // Show controls
  lcd.setCursor(12, 3);
  lcd.print("B:OK");
}

void handlePidBackwardWithMassaSettings() {
  // Handle UP/DOWN navigation
  if (UP()) {
    selectedParam = (selectedParam - 1 + 3) % 3;
  } else if (DOWN()) {
    selectedParam = (selectedParam + 1) % 3;
  }

  // Create array of pointers to PID values
  double* pidValues[3] = {
    &kpBackwardWithMassa,
    &kiBackwardWithMassa,
    &kdBackwardWithMassa
  };
  
  // Use reusable button handler (0.1 increment, 1.0 hold, 100ms interval)
  handlePidButtonAdjustment(pidValues, selectedParam, 0.1f, 1.0f, 100);

  // Handle back button
  if (STOP()) {
    saveSettings();
    currentMenu = MENU_PID_BACKWARD;
    selectedParam = 0;
    menuNeedsRefresh = true;
  }
}

// PID Backward Default Settings Functions
void displayPidBackwardDefaultSettings() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("PID Bwd Default");

  // Display Kp
  lcd.setCursor(0, 1);
  if (selectedParam == 0) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kp: ");
  lcd.print(kpBackwardDefault);
  lcd.print("       ");

  // Display Ki
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Ki: ");
  lcd.print(kiBackwardDefault);
  lcd.print("       ");

  // Display Kd
  lcd.setCursor(0, 3);
  if (selectedParam == 2) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kd: ");
  lcd.print(kdBackwardDefault);
  lcd.print("       ");

  // Show controls
  lcd.setCursor(12, 3);
  lcd.print("B:OK");
}

void handlePidBackwardDefaultSettings() {
  // Handle UP/DOWN navigation
  if (UP()) {
    selectedParam = (selectedParam - 1 + 3) % 3;
  } else if (DOWN()) {
    selectedParam = (selectedParam + 1) % 3;
  }

  // Create array of pointers to PID values
  double* pidValues[3] = {
    &kpBackwardDefault,
    &kiBackwardDefault,
    &kdBackwardDefault
  };
  
  // Use reusable button handler (0.01 increment, 0.1 hold, 500ms interval)
  handlePidButtonAdjustment(pidValues, selectedParam, 0.01f, 0.1f, 500);

  // Handle back button
  if (STOP()) {
    saveSettings();
    currentMenu = MENU_PID_BACKWARD;
    selectedParam = 0;
    menuNeedsRefresh = true;
  }
}

void displayTargetSettings() {
  displayMenuHeader("Target Settings");

  if (isClearingStations) {
    lcd.setCursor(0, 1);
    lcd.print("Clearing...");

    // Show progress bar
    unsigned long elapsed = millis() - xButtonHoldStart;
    int progress = (elapsed * 100) / X_HOLD_DURATION;
    progress = min(progress, 100);

    lcd.setCursor(0, 2);
    lcd.print("Progress: ");
    lcd.print(progress);
    lcd.print("%");

    lcd.setCursor(0, 3);
    lcd.print("Release X=cancel");
    return;
  }

  lcd.setCursor(0, 1);
  lcd.print("HTTP Stations:");

  if (targetStationsList.empty()) {
    lcd.setCursor(0, 2);
    lcd.print("No stations");
    lcd.setCursor(0, 3);
    lcd.print("Use HTTP API");
  } else {
    lcd.setCursor(0, 2);
    lcd.print("Total:");
    lcd.print(targetStationsList.size());

    lcd.setCursor(0, 3);
    for (size_t i = 0; i < targetStationsList.size() && i < 3; i++) {
      lcd.print(targetStationsList[i]);
      if (i < targetStationsList.size() - 1 && i < 2) lcd.print(",");
    }
    if (targetStationsList.size() > 3) {
      lcd.print("..");
    }
  }

  // Show controls
  lcd.setCursor(8, 3);
  lcd.print("X:Clear B:OK");
}

void displayRfidSettings() {
  static int lastRemainingTime = -1;  // -1 needs explicit init
  static int lastSelectedRfidItem = -1;  // -1 needs explicit init
  static int lastSelectedStationId = -1;  // -1 needs explicit init
  static bool lastMenuDrawn;  // Auto-initializes to false
  
  // Check if we need to refresh the display
  bool needsRefresh = menuNeedsRefresh || lastSelectedRfidItem != selectedRfidItem || lastSelectedStationId != selectedStationId || !lastMenuDrawn;
  
  if (needsRefresh) {
    lcd.clear();
    displayMenuHeader("RFID Settings");
    menuNeedsRefresh = false;
  }

  if (isWaitingForRfid) {
    lcd.setCursor(0, 1);
    lcd.print("Scan Station ");
    lcd.print(selectedStationId);

    lcd.setCursor(0, 2);
    lcd.print("Tap RFID card");

    lcd.setCursor(0, 3);
    int remainingTime = (RFID_SCAN_TIMEOUT - (millis() - rfidScanTimeout)) / 1000;
    lcd.print("Timeout:");
    lcd.print(remainingTime);
    lcd.print("s B:Cancel");

    // Set menuNeedsRefresh to true if we are still waiting for RFID and timeout changes
    // This ensures the timeout counter updates without full refresh
    if (lastRemainingTime != remainingTime) {
      lastRemainingTime = remainingTime;
      // No full refresh needed, just update the time
    }
    return;
  }

  // Only redraw menu items if refresh is needed
  if (needsRefresh) {
    // RFID Menu items
    String rfidMenuItems[10] = {
      "Station: " + String(selectedStationId),
      "Scan RFID",
      "Auto Input Station",
      "View All",
      "Delete Station",
      "Clear All",
      "RFID Ujung",
      "RFID Warehouse",
      "Terminal Drop",
      "Terminal Pickup"
    };

    // Clear menu area only when needed
    for (int i = 1; i <= 3; i++) {
      lcd.setCursor(0, i);
      lcd.print("                    ");  // Clear entire line
    }

    // Simple display - show items with scrolling if needed
    int startIdx = max(0, min(selectedRfidItem - 1, 10 - 3));

    for (int i = 0; i < 3 && (startIdx + i) < 10; i++) {
      int itemIndex = startIdx + i;
      lcd.setCursor(0, i + 1);

      if (itemIndex == selectedRfidItem) {
        lcd.print("> ");
      } else {
        lcd.print("  ");
      }

      String item = rfidMenuItems[itemIndex];
      if (item.length() > 13) {
        item = item.substring(0, 13);
      }
      lcd.print(item);
    }

    // Show navigation hint
    lcd.setCursor(16, 3);
    lcd.print("A:OK");
    
    // Update tracking variables
    lastSelectedRfidItem = selectedRfidItem;
    lastSelectedStationId = selectedStationId;
    lastMenuDrawn = true;
  }
}

void displayMotorSettings() {
  static int lastSelectedItem = -1;
  
  if (menuNeedsRefresh || selectedItem != lastSelectedItem) {
    lcd.clear();
    displayMenuHeader("Motor Settings:");
    
    String menuItems[2] = {
      "Speed Setting",      // selectedItem 0 -> MENU_SPEED_SETTING (46)
      "PID RPM"            // selectedItem 1 -> MENU_PID_RPM_SETTING (47)
    };
    
    for (int i = 0; i < 2; i++) {
      lcd.setCursor(0, i + 1);
      displayIndicator(i, selectedItem);
      lcd.print(menuItems[i]);
    }
    
    displayMenuFooter("A:OK B:Back");
    
    menuNeedsRefresh = false;
    lastSelectedItem = selectedItem;
  }
}

void displayResetMenu() {
  displayMenuHeader("Reset Settings");

  lcd.setCursor(0, 1);
  lcd.print("Reset ALL settings:");
  lcd.setCursor(0, 2);
  lcd.print("PID, Motor, RFID,");
  lcd.setCursor(0, 3);
  lcd.print("Target, Button A:OK");
}

void displayResetAgvStateMenu() {
  displayMenuHeader("Reset AGV State");

  lcd.setCursor(0, 1);
  lcd.print("Reset AGV state to");
  lcd.setCursor(0, 2);
  lcd.print("default (STOP)");
  lcd.setCursor(0, 3);
  lcd.print("A:Reset   B:Cancel");
}

void handlePidSettings() {
  // Static variables auto-initialize to 0/false
  static unsigned long lastRightPress;
  static unsigned long lastLeftPress;
  static unsigned long rightHoldStart;
  static unsigned long leftHoldStart;
  static bool rightHolding;
  static bool leftHolding;
  unsigned long currentMillis = millis();

  if (UP()) {
    selectedParam = (selectedParam - 1 + 3) % 3;
  } else if (DOWN()) {
    selectedParam = (selectedParam + 1) % 3;
  }

  // Check RIGHT button
  if (digitalRead(rightPin) == HIGH) {
    if (!rightHolding) {
      // Button just pressed
      rightHoldStart = currentMillis;
      rightHolding = true;
      
      // Single click - increment by 0.1
      pidIncrement = 0.1f;
      switch (selectedParam) {
        case 0: kp += pidIncrement; break;
        case 1: ki += pidIncrement; break;
        case 2: kd += pidIncrement; break;
      }
      lastRightPress = currentMillis;
    } else if (currentMillis - rightHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - increment by 1.0 every 100ms
      if (currentMillis - lastRightPress >= BUTTON_HOLD_INTERVAL) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: kp += pidIncrement; break;
          case 1: ki += pidIncrement; break;
          case 2: kd += pidIncrement; break;
        }
        lastRightPress = currentMillis;
      }
    }
  } else {
    rightHolding = false;
  }

  // Check LEFT button
  if (digitalRead(leftPin) == HIGH) {
    if (!leftHolding) {
      // Button just pressed
      leftHoldStart = currentMillis;
      leftHolding = true;
      
      // Single click - decrement by 0.1
      pidIncrement = 0.1f;
      switch (selectedParam) {
        case 0: kp = max(0.0f, (float)(kp - pidIncrement)); break;
        case 1: ki = max(0.0f, (float)(ki - pidIncrement)); break;
        case 2: kd = max(0.0f, (float)(kd - pidIncrement)); break;
      }
      lastLeftPress = currentMillis;
    } else if (currentMillis - leftHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - decrement by 1.0 every 100ms
      if (currentMillis - lastLeftPress >= BUTTON_HOLD_INTERVAL) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: kp = max(0.0f, (float)(kp - pidIncrement)); break;
          case 1: ki = max(0.0f, (float)(ki - pidIncrement)); break;
          case 2: kd = max(0.0f, (float)(kd - pidIncrement)); break;
        }
        lastLeftPress = currentMillis;
      }
    }
  } else {
    leftHolding = false;
  }

  if (STOP()) {
    saveSettings();
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;  // Reset scroll position
    menuNeedsRefresh = true;
  }
}



void handleTargetSettings() {
  unsigned long currentMillis = millis();

  if (START()) {
    if (!isClearingStations) {
      // Start X button hold detection
      xButtonHoldStart = currentMillis;
      isClearingStations = true;
    } else {
      // Check if held for 3 seconds
      if (currentMillis - xButtonHoldStart >= X_HOLD_DURATION) {
        // Call clearTargetStationsData after 3 seconds
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Clearing stations...");

        clearTargetStationsData();

        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Stations cleared!");
        delay(1500);

        // Reset state
        isClearingStations = false;
        xButtonHoldStart = 0;
      }
    }
  } else {
    // A button released, cancel clearing
    if (isClearingStations) {
      isClearingStations = false;
      xButtonHoldStart = 0;
    }
  }

  if (STOP()) {
    // Load latest stations from preferences
    loadTargetStationsListFromPreferences();
    isClearingStations = false;
    xButtonHoldStart = 0;
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;  // Reset scroll position
    menuNeedsRefresh = true;
  }
}

// ===================================================================
// RFID SETTINGS HANDLERS - Fix #6: Split long functions
// ===================================================================

/**
 * Handle RFID scanning process
 */
bool handleRfidScanning(unsigned long currentMillis) {
  if (!isWaitingForRfid) return false;
  
  // Check if new RFID was scanned
  if (newRfidScanned) {
    if (addRfidStation(selectedStationId, String(lastScannedRfidOptimized))) {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Station ");
      lcd.print(selectedStationId);
      lcd.print(" saved!");
      lcd.setCursor(0, 2);
      lcd.print("RFID: ");
      char rfidDisplay[9];
      strncpy(rfidDisplay, lastScannedRfidOptimized, 8);
      rfidDisplay[8] = '\0';
      lcd.print(rfidDisplay);
      lcd.print("...");
      delay(MESSAGE_DISPLAY_DURATION);
    } else {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Error saving!");
      delay(MESSAGE_DISPLAY_DURATION);
    }
    
    isWaitingForRfid = false;
    newRfidScanned = false;
    selectedRfidItem = 1;
  }
  
  // Check timeout or cancel
  if ((currentMillis - rfidScanTimeout > RFID_SCAN_TIMEOUT) || STOP()) {
    isWaitingForRfid = false;
    newRfidScanned = false;
  }
  
  return true;
}

/**
 * Display all RFID stations
 */
void displayAllRfidStations() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RFID Stations:");
  
  int displayRow = 1;
  bool hasData = false;
  for (int i = 0; i < rfidStationCount && displayRow < 4; i++) {
    if (rfidStations[i].isActive) {
      lcd.setCursor(0, displayRow);
      lcd.print("S");
      lcd.print(rfidStations[i].stationId);
      lcd.print(":");
      String shortRfid = rfidStations[i].rfidId.substring(0, 8);
      lcd.print(shortRfid);
      displayRow++;
      hasData = true;
    }
  }
  
  if (!hasData) {
    lcd.setCursor(0, 1);
    lcd.print("No stations set");
  }
  
  if (hasData && rfidStationCount > 3) {
    lcd.setCursor(13, 3);
    lcd.print("...");
  }
  
  // Wait for button press
  while (START() == 0 && STOP() == 0) delay(1);
  delay(BUTTON_DEBOUNCE_DELAY);
  menuNeedsRefresh = true;
}

/**
 * Delete RFID station with confirmation
 */
void handleDeleteRfidStation() {
  if (deleteRfidStation(selectedStationId)) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Station ");
    lcd.print(selectedStationId);
    lcd.print(" deleted!");
    delay(1500);
  } else {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Station not found!");
    delay(1500);
  }
}

/**
 * Clear all RFID stations with confirmation
 */
void handleClearAllRfid() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Clear all RFID?");
  lcd.setCursor(0, 2);
  lcd.print("A: Yes  B: No");
  
  // Wait for confirmation
  while (true) {
    if (START()) {
      clearAllRfidStations();
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("All stations");
      lcd.setCursor(0, 2);
      lcd.print("cleared!");
      delay(1500);
      break;
    } else if (STOP()) {
      break;
    }
    delay(NVS_WRITE_DELAY);
  }
}

/**
 * Main RFID settings handler
 */
void handleRfidSettings() {
  unsigned long currentMillis = millis();
  
  // Handle scanning process
  if (handleRfidScanning(currentMillis)) return;
  
  // Navigation
  if (UP()) {
    selectedRfidItem = (selectedRfidItem - 1 + 10) % 10;
  } else if (DOWN()) {
    selectedRfidItem = (selectedRfidItem + 1) % 10;
  } else if (RIGHT() && selectedRfidItem == 0) {
    selectedStationId = (selectedStationId % MAX_RFID_STATIONS) + 1;
    menuNeedsRefresh = true;
  } else if (LEFT() && selectedRfidItem == 0) {
    selectedStationId = selectedStationId == 1 ? MAX_RFID_STATIONS : selectedStationId - 1;
    menuNeedsRefresh = true;
  } else if (START()) {
    switch (selectedRfidItem) {
      case 1:  // Scan RFID
        isWaitingForRfid = true;
        rfidScanTimeout = currentMillis;
        newRfidScanned = false;
        break;
        
      case 2:  // Auto Input Station
        currentMenu = MENU_AUTO_INPUT_STATION;
        menuNeedsRefresh = true;
        break;
        
      case 3:  // View All
        displayAllRfidStations();
        break;
        
      case 4:  // Delete Station
        handleDeleteRfidStation();
        break;
        
      case 5:  // Clear All
        handleClearAllRfid();
        break;
        
      case 6:  // RFID Ujung
        currentMenu = MENU_RFID_UJUNG;
        menuNeedsRefresh = true;
        break;
        
      case 7:  // RFID Warehouse
        currentMenu = MENU_RFID_WAREHOUSE;
        menuNeedsRefresh = true;
        break;
        
      case 8:  // Terminal Drop
        currentMenu = MENU_TERMINAL_DROP;
        menuNeedsRefresh = true;
        break;
        
      case 9:  // Terminal Pickup
        currentMenu = MENU_TERMINAL_PICKUP;
        menuNeedsRefresh = true;
        break;
    }
  } else if (STOP()) {
    currentMenu = MENU_MAIN;
    selectedRfidItem = 0;
    menuStartIndex = 0;  // Reset scroll position
    menuNeedsRefresh = true;
  }
}

void handleMotorSettings() {
  if (UP()) {
    selectedItem = (selectedItem - 1 + 2) % 2;
    menuNeedsRefresh = true;
  } else if (DOWN()) {
    selectedItem = (selectedItem + 1) % 2;
    menuNeedsRefresh = true;
  } else if (START()) {
    lcd.clear();
    switch (selectedItem) {
      case 0:  // Speed Setting
        lcd.clear();
        currentMenu = MENU_SPEED_SETTING;
        break;
      case 1:  // PID RPM
        lcd.clear();
        currentMenu = MENU_PID_RPM_SETTING;
        selectedItem = 0;  // Reset for PID menu
        break;
    }
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Back to main menu
    lcd.clear();
    currentMenu = MENU_MAIN;
    selectedItem = 7;  // Return to Motor Settings item
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}

void handleResetMenu() {
  if (START()) {
    // Reset PID values to defaults
    kp = 70.0;
    ki = 0.0;
    kd = 0.0;
    
    // Reset Forward PID WithMassa values to defaults
    kpForwardWithMassa = 70.0;
    kiForwardWithMassa = 0.0;
    kdForwardWithMassa = 0.0;
    
    // Reset Forward PID Default values to defaults
    kpForwardDefault = 70.0;
    kiForwardDefault = 0.0;
    kdForwardDefault = 0.0;
    
    // Reset Backward PID WithMassa values to defaults
    kpBackwardWithMassa = 70.0;
    kiBackwardWithMassa = 0.0;
    kdBackwardWithMassa = 0.0;
    
    // Reset Backward PID Default values to defaults
    kpBackwardDefault = 70.0;
    kiBackwardDefault = 0.0;
    kdBackwardDefault = 0.0;
    
    // Debug: Print reset values
    // Serial.println() - removed for production
    // Serial.println() - removed for production
    // Serial.println() - removed for production
    // Serial.println() - removed for production
    // Serial.println() - removed for production

    // Reset Motor values to defaults
    baseSpeed = 1000;

    // Reset Motor invert values to defaults
    invertMotorY = false;
    invertMotorX = false;
    invertMotorKanan = false;
    invertMotorKiri = false;
    invertHook = false;

    // Reset Music mapping values to defaults
    musicOnPin = 0;        // pinMusic1
    musicObstaclePin = 1;  // pinMusic2
    musicStationPin = 2;   // pinMusic3
    musicOutOfLinePin = 3; // pinMusic4
    
    // Reset Ultrasonic settings to defaults
    minSafeDistanceFront = 30;
    minSafeDistanceBack = 20;  // Default safe distance

    // Clear all preferences first before saving
    preferences.begin("agv-settings", false);
    preferences.clear();
    preferences.end();

    // Save default values (saveSettings will handle begin/end)
    saveSettings();

    // Clear RFID stations too
    clearAllRfidStations();

    // Clear stations list from HTTP preferences
    stationsPreferences.begin(STATIONS_NAMESPACE, false);
    stationsPreferences.clear();
    stationsPreferences.end();

    // Clear targetStationsList in memory
    targetStationsList.clear();

    currentMenu = MENU_MAIN;
    menuStartIndex = 0;  // Reset scroll position
    menuNeedsRefresh = true;
    tombolBoot = true;
    setup();  // Return to main menu
  } else if (STOP()) {
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;  // Reset scroll position
    menuNeedsRefresh = true;
  }
}

void handleResetAgvStateMenu() {
  if (START()) {
    // Reset AGV state to default (STOP)
    currentRFID = AGV_STATE_TERMINAL_PICKUP;
    currentStateAgv = AGV_STATE_NULL;
    saveCurrentStateAGVToPreferences(currentStateAgv);
    // Show confirmation message
    lcd.clear();
    displayMenuHeader("AGV State Reset");
    lcd.setCursor(0, 1);
    lcd.print("AGV state has been");
    lcd.setCursor(0, 2);
    lcd.print("reset to STOP");
    delay(MESSAGE_DISPLAY_DURATION); // Show message for 2 seconds
    
    // Return to main menu
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;  // Reset scroll position
    menuNeedsRefresh = true;
  } else if (STOP()) {
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;  // Reset scroll position
    menuNeedsRefresh = true;
  }
}




void displayMusicTest() {
  static bool displayInitialized;  // Auto-initializes to false
  static int lastSelectedMusicItem = -1;  // -1 needs explicit init
  static int musicTestMenuStartIndex;  // Auto-initializes to 0
  static int lastMusicTestMenuStartIndex = -1;  // -1 needs explicit init
  
  // Music mode labels (5 main categories)
  String musicModes[5] = { "On", "Obstacle", "Station", "OutOfLine", "Warning" };
  int maxMusicTestDisplay = 2; // Show 2 items at a time (to avoid overlap with controls)
  
  // Update scroll position if needed
  if (selectedMusicItem < musicTestMenuStartIndex) {
    musicTestMenuStartIndex = selectedMusicItem;
    menuNeedsRefresh = true;
  } else if (selectedMusicItem >= musicTestMenuStartIndex + maxMusicTestDisplay) {
    musicTestMenuStartIndex = selectedMusicItem - maxMusicTestDisplay + 1;
    menuNeedsRefresh = true;
  }
  
  // Check if display needs refresh
  bool needsRefresh = !displayInitialized || 
                     selectedMusicItem != lastSelectedMusicItem ||
                     musicTestMenuStartIndex != lastMusicTestMenuStartIndex ||
                     menuNeedsRefresh;
  
  if (needsRefresh) {
    lcd.clear();
    displayMenuHeader("Music Test");

    // Display music items with scrolling (2 items max)
    for (int i = 0; i < maxMusicTestDisplay && (musicTestMenuStartIndex + i) < maxMusicItems; i++) {
      int musicIndex = musicTestMenuStartIndex + i;
      lcd.setCursor(0, i + 1);

      // Show cursor for selected item
      if (musicIndex == selectedMusicItem) {
        lcd.print("> ");
      } else {
        lcd.print("  ");
      }

      // Show mode name only
      lcd.print(musicModes[musicIndex]);
    }
    
    // Show controls at bottom (baris 3)
    lcd.setCursor(0, 3);
    lcd.print("A:Play B:Back");
    
    // Update tracking variables
    displayInitialized = true;
    lastSelectedMusicItem = selectedMusicItem;
    lastMusicTestMenuStartIndex = musicTestMenuStartIndex;
    menuNeedsRefresh = false;
  }
}

void handleMusicTest() {
  static bool displayInitialized;  // Auto-initializes to false
  
  if (UP()) {
    selectedMusicItem = (selectedMusicItem - 1 + maxMusicItems) % maxMusicItems;
    displayInitialized = false;
  } else if (DOWN()) {
    selectedMusicItem = (selectedMusicItem + 1) % maxMusicItems;
    displayInitialized = false;
  } else if (START()) {
    // Play selected music mode
    statusMusic = false;
    switch (selectedMusicItem) {
      case 0: 
        music(MUSIC_MODE_ON);
        break;
      case 1: 
        music(MUSIC_MODE_OBSTACLE);
        break;
      case 2: 
        music(MUSIC_MODE_STATION);
        break;
      case 3: 
        music(MUSIC_MODE_OUTOFLINE);
        break;
      case 4: 
        music(MUSIC_MODE_WARNING);
        break;
    }
    displayInitialized = false;
  } else if (STOP()) {
    // Stop all music and back to main menu
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;
    selectedMusicItem = 0;
    menuNeedsRefresh = true;
  }
}

void displayHookTest() {
  displayMenuHeader("Hook Test");

  lcd.setCursor(0, 1);
  lcd.print("UP   : Hook Naik");
  lcd.setCursor(0, 2);
  lcd.print("DOWN : Hook Turun");
  lcd.setCursor(0, 3);

  // Show current hook state
  switch (hookTestState) {
    case 0: lcd.print("Status: STOP    "); break;
    case 1: lcd.print("Status: NAIK    "); break;
    case 2: lcd.print("Status: TURUN   "); break;
  }

  lcd.setCursor(15, 3);
  lcd.print("B:OK");
}

void displayMagnetCheck() {
  displayMenuHeader("Magnet Check");

  lcd.setCursor(0, 1);
  lcd.print("Err:");
  lcd.print(errorValue);
  lcd.print(" Act:");
  lcd.print(totalSensorAktif);
  lcd.print("    ");  // Clear remaining characters

  // Display magnet sensor status: 1 = detected, 0 = not detected
  lcd.setCursor(0, 2);
  // Menampilkan dari kanan ke kiri (sensor 15, 14, 13, ... 0)
  for (int i = 15; i >= 0; i--) {
    if (jumlahMagnet[i] == 1) {
      lcd.print("1");
    } else {  
      lcd.print("0");
    } 
  }

  lcd.setCursor(0, 3);
  // Show current sensor (Front/Back) and controls
  if (getCurrentMagnetSlaveId() == SLAVEID_MAGNET_DEPAN) {
    lcd.print("F");
  } else {
    lcd.print("B");
  }
  lcd.print(" LR:Switch B:Back");
}

void displayUltrasonicCheck() {
  static unsigned long lastDisplayUpdate;  // Auto-initializes to 0
  
  unsigned long currentTime = millis();
  
  // Rate-limited display update to prevent excessive LCD operations
  if (currentTime - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL) {
    return;
  }
  lastDisplayUpdate = currentTime;
  
  displayMenuHeader("Ultrasonic Check");

  lcd.setCursor(0, 1);
  lcd.print("P1:");
  lcd.print(ultrasonicDistances[0]);
  lcd.print(" P2:");
  lcd.print(ultrasonicDistances[1]);
  lcd.print("    "); // Clear remaining characters

  lcd.setCursor(0, 2);
  lcd.print("P3:");
  lcd.print(ultrasonicDistances[2]);
  lcd.print(" P4:");
  lcd.print(ultrasonicDistances[3]);
  lcd.print(" P5:");
  lcd.print(ultrasonicDistances[4]);
  lcd.print("   "); // Clear remaining characters

  lcd.setCursor(0, 3);
  // Manual obstacle detection based on distance values only
  uint16_t currentMinSafeDistance = (getCurrentUltrasonicSlaveId() == SLAVEID_ULTRASONIK_DEPAN) ? 
                                   minSafeDistanceFront : minSafeDistanceBack;

  if (obstacleDetected) {
    lcd.print("OBSTACLE! ");
  } else {
    lcd.print("Clear     ");
  }

  // Show current sensor (Front/Back) and controls
  lcd.setCursor(10, 3);
  if (getCurrentUltrasonicSlaveId() == SLAVEID_ULTRASONIK_DEPAN) {
    lcd.print("F");
  } else {
    lcd.print("B");
  }
  lcd.print(" LR:Switch");
}

void handleHookTest() {
  if (UP()) {
    hookTestState = 1;  // Set to naik
    hookPosition = hook(UP_HOOK);
  } else if (DOWN()) {
    hookTestState = 2;  // Set to turun
    hookPosition = hook(DOWN_HOOK);
  } else if (STOP()) {
    hookTestState = 0;  // Stop
    hookPosition = hook(STOP_HOOK);       // Stop hook movement
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  } else {
    // Continue current state
    switch (hookTestState) {
      case 1:  // Continue naik
        hookPosition = hook(UP_HOOK);
        break;
      case 2:  // Continue turun
        hookPosition = hook(DOWN_HOOK);
        break;
      case 0:  // Stopped
      default:
        hookPosition = hook(STOP_HOOK);
        break;
    }
  }
}

void handleMagnetCheck() {
  // Selalu baca sensor saat menu ini aktif
  loopMagneticSensor();

  if (LEFT()) {
    // Switch to front magnet sensor
    switchMagnetSensor(true);
  } else if (RIGHT()) {
    // Switch to back magnet sensor
    switchMagnetSensor(false);
  } else if (STOP()) {
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
  // Display updates automatically since displayMagnetCheck reads current sensor values
}

void handleUltrasonicCheck() {
  static unsigned long lastSwitchTime;  // Auto-initializes to 0
  static unsigned long lastSensorRead;  // Auto-initializes to 0
  
  unsigned long currentTime = millis();
  
  // Rate-limited sensor reading to prevent stack overflow
  if (currentTime - lastSensorRead >= SENSOR_READ_INTERVAL) {
    // Only read ultrasonic sensor, avoid calling checkObstacles in menu mode
    loopUltrasonik();
    lastSensorRead = currentTime;
  }
  
  if (LEFT() && (currentTime - lastSwitchTime >= SWITCH_DEBOUNCE)) {
    // Serial.println() - removed for production
    // Switch to front ultrasonic sensor
    setUltrasonicSlaveId(SLAVEID_ULTRASONIK_DEPAN);
    lastSwitchTime = currentTime;
  } else if (RIGHT() && (currentTime - lastSwitchTime >= SWITCH_DEBOUNCE)) {
    // Serial.println() - removed for production
    // Switch to back ultrasonic sensor
    setUltrasonicSlaveId(SLAVEID_ULTRASONIK_BELAKANG);
    lastSwitchTime = currentTime;
  } else if (STOP()) {
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
  // Display updates automatically since displayUltrasonicCheck reads current sensor values
}

void displayWifiSettings() {
  // Display different info based on scroll index
  switch (wifiScrollIndex) {
    case 0: // Status & Connection Info
      displayMenuHeader("WiFi Settings");
      if (WiFi.status() == WL_CONNECTED) {
        lcd.setCursor(0, 1);
        lcd.print("Status: Terhubung   ");
        lcd.setCursor(0, 2);
        lcd.print("SSID: ");
        String ssidStr = WiFi.SSID();
        if (ssidStr.length() > 10) {
          lcd.print(ssidStr.substring(0, 10));
        } else {
          lcd.print(ssidStr);
          for (int i = ssidStr.length(); i < 10; i++) {
            lcd.print(" ");
          }
        }
        lcd.print("    ");
      } else {
        lcd.setCursor(0, 1);
        lcd.print("Status: Terputus    ");
        lcd.setCursor(0, 2);
        lcd.print("Mode: Access Point  ");
      }
      break;
      
    case 1: // IP Address Info
      displayMenuHeader("IP Address         ");
      if (WiFi.status() == WL_CONNECTED) {
        lcd.setCursor(0, 1);
        lcd.print("Client IP:           ");
        lcd.setCursor(0, 2);
        String clientIP = WiFi.localIP().toString();
        lcd.print(clientIP);
        for (int i = clientIP.length(); i < 16; i++) {
          lcd.print(" ");
        }
      } else {
        lcd.setCursor(0, 1);
        lcd.print("AP IP: ");
        lcd.setCursor(0, 2);
        lcd.print("192.168.121.14         ");
      }
      break;
      
    case 2: // AP Configuration
      displayMenuHeader("Access Point        ");
      lcd.setCursor(0, 1);
      lcd.print("SSID:ESP32-AGV-Cfg ");
      lcd.setCursor(0, 2);
      lcd.print("Pass:12345678       ");
      break;
      
    case 3: // Web Interface
      displayMenuHeader("Web Interface");
      lcd.setCursor(0, 1);
      lcd.print("URL: 192.168.121.14 ");
      lcd.setCursor(0, 2);
      lcd.print("Port: 80            ");
      break;
      

  }
  
  // Show navigation controls
  lcd.setCursor(0, 3);
  lcd.print("^v:Scroll A:Conn B:<");
}

void handleWifiSettings() {
 
  if (UP()) {
    // Scroll up in main WiFi menu
    wifiScrollIndex--;
    if (wifiScrollIndex < 0) {
      wifiScrollIndex = WIFI_MAX_SCROLL;
    }
    menuNeedsRefresh = true;
    delay(150); // Reduced debounce delay
  } else if (DOWN()) {
    // Scroll down in main WiFi menu
    wifiScrollIndex++;
    if (wifiScrollIndex > WIFI_MAX_SCROLL) {
      wifiScrollIndex = 0;
    }
    menuNeedsRefresh = true;
    delay(150); // Reduced debounce delay
  } else if (START()) {
    // Start WiFi connection
    startWifiConnection();
    
  } else if (STOP()) {
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}

// ===== RFID UJUNG FUNCTIONS =====
void displayRfidUjung() {
  displayMenuHeader("RFID Ujung");
  
  lcd.setCursor(0, 1);
  lcd.print("Count: ");
  lcd.print(rfidUjungCount);
  lcd.print("/");
  lcd.print(MAX_RFID_UJUNG);
  lcd.print("        ");
  
  // Tampilkan daftar RFID yang sudah tersimpan
  if (rfidUjungCount > 0) {
    lcd.setCursor(0, 2);
    lcd.print("1:");
    if (rfidUjungList[0].isActive) {
      lcd.print(rfidUjungList[0].rfidId.substring(0, 17));
    } else {
      lcd.print("Empty           ");
    }
  } else {
    lcd.setCursor(0, 2);
    lcd.print("1:Empty             ");
  }
  
  if (rfidUjungCount > 1) {
    lcd.setCursor(0, 3);
    lcd.print("2:");
    if (rfidUjungList[1].isActive) {
      lcd.print(rfidUjungList[1].rfidId.substring(0, 17));
    } else {
      lcd.print("Empty           ");
    }
  } else {
    lcd.setCursor(0, 3);
    lcd.print("2:Empty             ");
  }
}

void handleRfidUjung() {
  if (START()) { // Scan RFID
    // Cek apakah sudah penuh
    if (rfidUjungCount >= MAX_RFID_UJUNG) {
      lcd.setCursor(0, 1);
      lcd.print("Storage Full!       ");
      lcd.setCursor(0, 2);
      lcd.print("Delete data first   ");
      lcd.setCursor(0, 3);
      lcd.print("STOP:Back           ");
      delay(MESSAGE_DISPLAY_DURATION);
      displayRfidUjung();
      return;
    }
    
    lcd.setCursor(0, 1);
    lcd.print("Scanning RFID...    ");
    lcd.setCursor(0, 2);
    lcd.print("Place card on reader");
    lcd.setCursor(0, 3);
    lcd.print("STOP:Cancel         ");
    
    unsigned long scanStart = millis();
    while (millis() - scanStart < 10000) { // 10 second timeout
      if (STOP()) {
        displayRfidUjung();
        return;
      }
      
      if (newRfidScanned) {
        String rfidData = String(lastScannedRfidOptimized);
        newRfidScanned = false;

        // Cek apakah RFID sudah ada di list
        bool alreadyExists = false;
        for (int i = 0; i < rfidUjungCount; i++) {
          if (rfidUjungList[i].rfidId == rfidData) {
            alreadyExists = true;
            break;
          }
        }
        
        if (alreadyExists) {
          lcd.setCursor(0, 1);
          lcd.print("RFID Already Exists!");
          lcd.setCursor(0, 2);
          lcd.print("                    ");
          delay(MESSAGE_DISPLAY_DURATION);
          displayRfidUjung();
          return;
        }
        
        // Simpan RFID baru
        rfidUjungList[rfidUjungCount].ujungId = rfidUjungCount + 1;
        rfidUjungList[rfidUjungCount].rfidId = rfidData;
        rfidUjungList[rfidUjungCount].isActive = true;
        rfidUjungCount++;
        saveRfidUjungToPreferences();
        
        // Sinkronisasi dengan ujungRfidId untuk logika AGV (simpan RFID pertama)
        if (rfidUjungCount == 1) {
          saveUjungRfid(rfidData);
        }
        
        lcd.setCursor(0, 1);
        lcd.print("RFID Saved!         ");
        lcd.setCursor(0, 2);
        lcd.print(rfidData.substring(0, 16));
        lcd.print("    ");
        delay(MESSAGE_DISPLAY_DURATION);
        
        displayRfidUjung();
        return;
      }
      delay(OPERATION_DELAY);
    }
    
    lcd.setCursor(0, 1);
    lcd.print("Scan Timeout!       ");
    delay(MESSAGE_DISPLAY_DURATION);
    displayRfidUjung();
    
  } else if (LEFT()) { // View data
    if (rfidUjungCount == 0) {
      lcd.setCursor(0, 1);
      lcd.print("No Data Available   ");
      delay(MESSAGE_DISPLAY_DURATION);
      displayRfidUjung();
      return;
    }
    
    int viewIndex = 0;
    while (true) {
      displayMenuHeader("View RFID Ujung");
      
      lcd.setCursor(0, 1);
      lcd.print("[");
      lcd.print(viewIndex + 1);
      lcd.print("/");
      lcd.print(rfidUjungCount);
      lcd.print("]             ");
      
      lcd.setCursor(0, 2);
      lcd.print("ID: ");
      lcd.print(rfidUjungList[viewIndex].ujungId);
      lcd.print("              ");
      
      lcd.setCursor(0, 3);
      lcd.print(rfidUjungList[viewIndex].rfidId.substring(0, 20));
      
      if (UP() && viewIndex > 0) {
        viewIndex--;
        delay(BUTTON_DEBOUNCE_DELAY);
      } else if (DOWN() && viewIndex < rfidUjungCount - 1) {
        viewIndex++;
        delay(BUTTON_DEBOUNCE_DELAY);
      } else if (STOP()) {
        displayRfidUjung();
        return;
      }
      delay(NVS_WRITE_DELAY);
    }
    
  } else if (RIGHT()) { // Delete menu
    displayMenuHeader("Delete RFID Ujung");
    lcd.setCursor(0, 1);
    lcd.print("A:Del1 B:Del2 C:All ");
    lcd.setCursor(0, 2);
    lcd.print("STOP:Back           ");
    
    while (true) {
      if (START()) { // Delete RFID 1
        if (rfidUjungCount < 1) {
          lcd.setCursor(0, 1);
          lcd.print("No Data at Index 1  ");
          delay(1500);
          displayRfidUjung();
          return;
        }
        
        // Shift data jika ada RFID ke-2
        if (rfidUjungCount == 2) {
          rfidUjungList[0] = rfidUjungList[1];
          rfidUjungList[0].ujungId = 1;
        }
        rfidUjungCount--;
        saveRfidUjungToPreferences();
        
        // Update ujungRfidId
        if (rfidUjungCount > 0) {
          saveUjungRfid(rfidUjungList[0].rfidId);
        } else {
          saveUjungRfid("");
        }
        
        lcd.setCursor(0, 1);
        lcd.print("RFID 1 Deleted!     ");
        delay(1500);
        displayRfidUjung();
        return;
        
      } else if (LEFT()) { // Delete RFID 2
        if (rfidUjungCount < 2) {
          lcd.setCursor(0, 1);
          lcd.print("No Data at Index 2  ");
          delay(1500);
          displayRfidUjung();
          return;
        }
        
        rfidUjungCount--;
        saveRfidUjungToPreferences();
        
        lcd.setCursor(0, 1);
        lcd.print("RFID 2 Deleted!     ");
        delay(1500);
        displayRfidUjung();
        return;
        
      } else if (RIGHT()) { // Delete All
        displayMenuHeader("Delete All Ujung");
        lcd.setCursor(0, 1);
        lcd.print("Are you sure?       ");
        lcd.setCursor(0, 2);
        lcd.print("A:Yes B:No          ");
        
        while (true) {
          if (START()) {
            rfidUjungCount = 0;
            saveRfidUjungToPreferences();
            
            // Hapus juga ujungRfidId dari logika AGV
            saveUjungRfid("");
            
            lcd.setCursor(0, 1);
            lcd.print("All Data Deleted!   ");
            delay(MESSAGE_DISPLAY_DURATION);
            displayRfidUjung();
            return;
          } else if (LEFT() || STOP()) {
            displayRfidUjung();
            return;
          }
          delay(NVS_WRITE_DELAY);
        }
        
      } else if (STOP()) {
        displayRfidUjung();
        return;
      }
      delay(NVS_WRITE_DELAY);
    }
    
  } else if (STOP()) {
    currentMenu = MENU_RFID_SETTINGS;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}

void saveRfidUjungToPreferences() {
  preferences.begin("rfid_ujung", false);
  preferences.putInt("count", rfidUjungCount);
  
  for (int i = 0; i < rfidUjungCount; i++) {
    String key = "rfid_" + String(i);
    preferences.putString(key.c_str(), rfidUjungList[i].rfidId);
    
    String idKey = "id_" + String(i);
    preferences.putInt(idKey.c_str(), rfidUjungList[i].ujungId);
    
    String activeKey = "active_" + String(i);
    preferences.putBool(activeKey.c_str(), rfidUjungList[i].isActive);
  }
  
  preferences.end();
}

void loadRfidUjungFromPreferences() {
  preferences.begin("rfid_ujung", true);
  rfidUjungCount = preferences.getInt("count", 0);
  
  for (int i = 0; i < rfidUjungCount && i < MAX_RFID_UJUNG; i++) {
    String key = "rfid_" + String(i);
    rfidUjungList[i].rfidId = preferences.getString(key.c_str(), "");
    
    String idKey = "id_" + String(i);
    rfidUjungList[i].ujungId = preferences.getInt(idKey.c_str(), i + 1);
    
    String activeKey = "active_" + String(i);
    rfidUjungList[i].isActive = preferences.getBool(activeKey.c_str(), true);
  }
  
  preferences.end();
}

// ===== RFID WAREHOUSE FUNCTIONS =====
void displayRfidWarehouse() {
  displayMenuHeader("RFID Warehouse");
  
  lcd.setCursor(0, 1);
  lcd.print("Count: ");
  lcd.print(rfidWarehouseCount);
  lcd.print("/");
  lcd.print(MAX_RFID_WAREHOUSE);
  lcd.print("        ");
  
  lcd.setCursor(0, 2);
  lcd.print("A:Scan B:View C:Del ");
  
  lcd.setCursor(0, 3);
  lcd.print("STOP:Back           ");
}

void handleRfidWarehouse() {
  if (START()) { // Scan RFID
    lcd.setCursor(0, 1);
    lcd.print("Scanning RFID...    ");
    lcd.setCursor(0, 2);
    lcd.print("Place card on reader");
    lcd.setCursor(0, 3);
    lcd.print("STOP:Cancel         ");
    
    unsigned long scanStart = millis();
    while (millis() - scanStart < 10000) { // 10 second timeout
      if (STOP()) {
        displayRfidWarehouse();
        return;
      }
      
      if (newRfidScanned) {
        String rfidData = String(lastScannedRfidOptimized);
        newRfidScanned = false;
        
        // Hanya simpan 1 data - ganti data lama jika ada
        rfidWarehouseList[0].warehouseId = 1;
        rfidWarehouseList[0].rfidId = rfidData;
        rfidWarehouseList[0].isActive = true;
        rfidWarehouseCount = 1;  // Selalu 1 data saja
        saveRfidWarehouseToPreferences();
        
        // Sinkronisasi dengan warehouseRfidId untuk logika AGV
        saveWarehouseRfid(rfidData);
        
        lcd.setCursor(0, 1);
        lcd.print("RFID Saved!         ");
        lcd.setCursor(0, 2);
        lcd.print(rfidData.substring(0, 16));
        lcd.print("    ");
        delay(MESSAGE_DISPLAY_DURATION);
        
        displayRfidWarehouse();
        return;
      }
      delay(OPERATION_DELAY);
    }
    
    lcd.setCursor(0, 1);
    lcd.print("Scan Timeout!       ");
    delay(MESSAGE_DISPLAY_DURATION);
    displayRfidWarehouse();
    
  } else if (LEFT()) { // View data
    if (rfidWarehouseCount == 0) {
      lcd.setCursor(0, 1);
      lcd.print("No Data Available   ");
      delay(MESSAGE_DISPLAY_DURATION);
      displayRfidWarehouse();
      return;
    }
    
    int viewIndex = 0;
    while (true) {
      displayMenuHeader("View Warehouse");
      
      lcd.setCursor(0, 1);
      lcd.print("[");
      lcd.print(viewIndex + 1);
      lcd.print("/");
      lcd.print(rfidWarehouseCount);
      lcd.print("]             ");
      
      lcd.setCursor(0, 2);
      lcd.print(rfidWarehouseList[viewIndex].rfidId.substring(0, 16));
      lcd.print("    ");
      
      lcd.setCursor(0, 3);
      lcd.print("^v:Nav STOP:Back    ");
      
      if (UP() && viewIndex > 0) {
        viewIndex--;
        delay(BUTTON_DEBOUNCE_DELAY);
      } else if (DOWN() && viewIndex < rfidWarehouseCount - 1) {
        viewIndex++;
        delay(BUTTON_DEBOUNCE_DELAY);
      } else if (STOP()) {
        displayRfidWarehouse();
        return;
      }
      delay(NVS_WRITE_DELAY);
    }
    
  } else if (RIGHT()) { // Delete all
    displayMenuHeader("Delete All Warehouse");
    lcd.setCursor(0, 1);
    lcd.print("Are you sure?       ");
    lcd.setCursor(0, 2);
    lcd.print("A:Yes B:No          ");
    
    while (true) {
      if (START()) {
        rfidWarehouseCount = 0;
        saveRfidWarehouseToPreferences();
        
        // Hapus juga warehouseRfidId dari logika AGV
        saveWarehouseRfid("");
        
        lcd.setCursor(0, 1);
        lcd.print("All Data Deleted!   ");
        delay(MESSAGE_DISPLAY_DURATION);
        displayRfidWarehouse();
        return;
      } else if (LEFT() || STOP()) {
        displayRfidWarehouse();
        return;
      }
      delay(NVS_WRITE_DELAY);
    }
    
  } else if (STOP()) {
    currentMenu = MENU_RFID_SETTINGS;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}

void saveRfidWarehouseToPreferences() {
  preferences.begin("rfid_warehouse", false);
  preferences.putInt("count", rfidWarehouseCount);
  
  for (int i = 0; i < rfidWarehouseCount; i++) {
    String key = "rfid_" + String(i);
    preferences.putString(key.c_str(), rfidWarehouseList[i].rfidId);
    
    String idKey = "id_" + String(i);
    preferences.putInt(idKey.c_str(), rfidWarehouseList[i].warehouseId);
    
    String activeKey = "active_" + String(i);
    preferences.putBool(activeKey.c_str(), rfidWarehouseList[i].isActive);
  }
  
  preferences.end();
}

void loadRfidWarehouseFromPreferences() {
  preferences.begin("rfid_warehouse", true);
  rfidWarehouseCount = preferences.getInt("count", 0);
  
  for (int i = 0; i < rfidWarehouseCount && i < MAX_RFID_WAREHOUSE; i++) {
    String key = "rfid_" + String(i);
    rfidWarehouseList[i].rfidId = preferences.getString(key.c_str(), "");
    
    String idKey = "id_" + String(i);
    rfidWarehouseList[i].warehouseId = preferences.getInt(idKey.c_str(), i + 1);
    
    String activeKey = "active_" + String(i);
    rfidWarehouseList[i].isActive = preferences.getBool(activeKey.c_str(), true);
  }
  
  preferences.end();
}

// ===== AUTO INPUT STATION FUNCTIONS =====
void displayAutoInputStation() {
  displayMenuHeader("Auto Input Station");
  
  lcd.setCursor(0, 1);
  lcd.print("Count: ");
  lcd.print(rfidStationCount);
  lcd.print("/");
  lcd.print(MAX_RFID_STATIONS);
  lcd.print("        ");
  
  lcd.setCursor(0, 2);
  lcd.print("A:Scan B:View C:Del ");
  
  lcd.setCursor(0, 3);
  lcd.print("STOP:Back           ");
}

void handleAutoInputStation() {
  if (START()) { // Scan RFID
    lcd.setCursor(0, 1);
    lcd.print("Scanning RFID...    ");
    lcd.setCursor(0, 2);
    lcd.print("Place card on reader");
    lcd.setCursor(0, 3);
    lcd.print("STOP:Cancel         ");
    
    unsigned long scanStart = millis();
    while (millis() - scanStart < 10000) { // 10 second timeout
      if (STOP()) {
        displayAutoInputStation();
        return;
      }
      
      if (newRfidScanned) {
        String rfidData = String(lastScannedRfidOptimized);
        newRfidScanned = false;
        
        // Check if already exists using existing function
        if (findRfidStationByRfidId(rfidData) == -1 && rfidStationCount < MAX_RFID_STATIONS) {
          // Use existing addRfidStation function
          int newStationId = rfidStationCount + 1;
          addRfidStation(newStationId, rfidData);
          
          lcd.setCursor(0, 1);
          lcd.print("Station Added!      ");
          lcd.setCursor(0, 2);
          lcd.print("ID: ");
          lcd.print(newStationId);
          lcd.print(" ");
          lcd.print(rfidData.substring(0, 8));
          lcd.print("    ");
          delay(MESSAGE_DISPLAY_DURATION);
        } else if (findRfidStationByRfidId(rfidData) != -1) {
          lcd.setCursor(0, 1);
          lcd.print("Station Exists!     ");
          delay(MESSAGE_DISPLAY_DURATION);
        } else {
          lcd.setCursor(0, 1);
          lcd.print("Storage Full!       ");
          delay(MESSAGE_DISPLAY_DURATION);
        }
        
        displayAutoInputStation();
        return;
      }
      delay(OPERATION_DELAY);
    }
    
    lcd.setCursor(0, 1);
    lcd.print("Scan Timeout!       ");
    delay(MESSAGE_DISPLAY_DURATION);
    displayAutoInputStation();
    
  } else if (LEFT()) { // View data
    if (rfidStationCount == 0) {
      lcd.setCursor(0, 1);
      lcd.print("No Data Available   ");
      delay(MESSAGE_DISPLAY_DURATION);
      displayAutoInputStation();
      return;
    }
    
    int viewIndex = 0;
    while (true) {
      displayMenuHeader("View RFID Stations");
      
      lcd.setCursor(0, 1);
      lcd.print("[");
      lcd.print(viewIndex + 1);
      lcd.print("/");
      lcd.print(rfidStationCount);
      lcd.print("] ID:");
      lcd.print(rfidStations[viewIndex].stationId);
      lcd.print("        ");
      
      lcd.setCursor(0, 2);
      lcd.print(rfidStations[viewIndex].rfidId.substring(0, 16));
      lcd.print("    ");
      
      lcd.setCursor(0, 3);
      lcd.print("^v:Nav STOP:Back    ");
      
      if (UP() && viewIndex > 0) {
        viewIndex--;
        delay(BUTTON_DEBOUNCE_DELAY);
      } else if (DOWN() && viewIndex < rfidStationCount - 1) {
        viewIndex++;
        delay(BUTTON_DEBOUNCE_DELAY);
      } else if (STOP()) {
        displayAutoInputStation();
        return;
      }
      delay(NVS_WRITE_DELAY);
    }
    
  } else if (RIGHT()) { // Delete all
    displayMenuHeader("Delete All Stations");
    lcd.setCursor(0, 1);
    lcd.print("Are you sure?       ");
    lcd.setCursor(0, 2);
    lcd.print("A:Yes B:No          ");
    
    while (true) {
      if (START()) {
        clearAllRfidStations(); // Use existing function
        lcd.setCursor(0, 1);
        lcd.print("All Data Deleted!   ");
        delay(MESSAGE_DISPLAY_DURATION);
        displayAutoInputStation();
        return;
      } else if (LEFT() || STOP()) {
        displayAutoInputStation();
        return;
      }
      delay(NVS_WRITE_DELAY);
    }
    
  } else if (STOP()) {
    currentMenu = MENU_RFID_SETTINGS;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}

// saveAutoStationsToPreferences function removed - using existing RFID station management functions

// loadAutoStationsFromPreferences function removed - using existing RFID station management functions

bool isStationExists(String rfidData) {
  // Use existing findRfidStation function - returns -1 if not found
  return findRfidStationByRfidId(rfidData) != -1;
}

// ===== TERMINAL DROP FUNCTIONS =====
void displayTerminalDrop() {
  displayMenuHeader("Terminal Drop");
  
  lcd.setCursor(0, 1);
  lcd.print("Current RFID:");
  
  lcd.setCursor(0, 2);
  if (terminalDropRfidId.length() > 0) {
    String shortRfid = terminalDropRfidId.substring(0, 12);
    lcd.print(shortRfid);
    lcd.print("    ");
  } else {
    lcd.print("Not set         ");
  }
  
  lcd.setCursor(0, 3);
  lcd.print("A:Scan STOP:Back   ");
}

void handleTerminalDrop() {
  if (START()) { // Scan RFID
    lcd.setCursor(0, 1);
    lcd.print("Scanning RFID...    ");
    lcd.setCursor(0, 2);
    lcd.print("Place card on reader");
    lcd.setCursor(0, 3);
    lcd.print("STOP:Cancel         ");
    
    unsigned long scanStart = millis();
    newRfidScanned = false;
    
    while (millis() - scanStart < 10000) { // 10 second timeout
      if (newRfidScanned) {
        String scannedRfid = String(lastScannedRfidOptimized);
        saveTerminalDropRfid(scannedRfid);
        
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Terminal Drop RFID");
        lcd.setCursor(0, 2);
        lcd.print("saved successfully!");
        delay(MESSAGE_DISPLAY_DURATION);
        
        newRfidScanned = false;
        displayTerminalDrop();
        return;
      } else if (LEFT() || STOP()) {
        displayTerminalDrop();
        return;
      }
      delay(NVS_WRITE_DELAY);
    }
    
  } else if (STOP()) {
    currentMenu = MENU_RFID_SETTINGS;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}

// ===== TERMINAL PICKUP FUNCTIONS =====
void displayTerminalPickup() {
  displayMenuHeader("Terminal Pickup");
  
  lcd.setCursor(0, 1);
  lcd.print("Current RFID:");
  
  lcd.setCursor(0, 2);
  if (terminalPickUpRfidId.length() > 0) {
    String shortRfid = terminalPickUpRfidId.substring(0, 12);
    lcd.print(shortRfid);
    lcd.print("    ");
  } else {
    lcd.print("Not set         ");
  }
  
  lcd.setCursor(0, 3);
  lcd.print("A:Scan STOP:Back   ");
}

void handleTerminalPickup() {
  if (START()) { // Scan RFID
    lcd.setCursor(0, 1);
    lcd.print("Scanning RFID...    ");
    lcd.setCursor(0, 2);
    lcd.print("Place card on reader");
    lcd.setCursor(0, 3);
    lcd.print("STOP:Cancel         ");
    
    unsigned long scanStart = millis();
    newRfidScanned = false;
    
    while (millis() - scanStart < 10000) { // 10 second timeout
      if (newRfidScanned) {
        String scannedRfid = String(lastScannedRfidOptimized);
        saveTerminalPickUpRfid(scannedRfid);
        
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Terminal Pickup RFID");
        lcd.setCursor(0, 2);
        lcd.print("saved successfully!");
        delay(MESSAGE_DISPLAY_DURATION);
        
        newRfidScanned = false;
        displayTerminalPickup();
        return;
      } else if (LEFT() || STOP()) {
        displayTerminalPickup();
        return;
      }
      delay(OPERATION_DELAY);
    }
    
    // Timeout
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Scan timeout!");
    delay(1500);
    displayTerminalPickup();
  } else if (STOP()) {
    currentMenu = MENU_RFID_SETTINGS;
    menuNeedsRefresh = true;
  }
}



void displayUltrasonicSettings() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("Ultrasonic Settings");
  
  // Display Front Sensor option
  lcd.setCursor(0, 1);
  if (selectedItem == 0) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Front Sensor");
  
  // Display Back Sensor option
  lcd.setCursor(0, 2);
  if (selectedItem == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Back Sensor");
  
  lcd.setCursor(0, 3);
  lcd.print("A:Select B:Back");
}

void handleUltrasonicSettings() {
  if (UP()) {
    if (selectedItem > 0) {
      selectedItem--;
    }
  } else if (DOWN()) {
    if (selectedItem < 1) {  // 0=Front, 1=Back
      selectedItem++;
    }
  } else if (START()) {
    // Enter selected submenu
    if (selectedItem == 0) {
      // Front Sensor Settings
      currentMenu = MENU_ULTRASONIC_FRONT;
      menuNeedsRefresh = true;
    } else if (selectedItem == 1) {
      // Back Sensor Settings
      currentMenu = MENU_ULTRASONIC_BACK;
      menuNeedsRefresh = true;
    }
  } else if (STOP()) {
    // Back to main menu
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;
    selectedItem = 0;
    menuNeedsRefresh = true;
  }
}

// Ultrasonic Front Settings Functions
void displayUltrasonicFrontSettings() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    selectedItem = 0;  // Reset selection
    maxItems = 2;      // 2 items: Tengah, Serong
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("Front Sensor");
  
  const char* menuItems[] = {
    "Tengah",
    "Serong"
  };
  
  // Display menu items
  for (int i = 0; i < maxItems; i++) {
    lcd.setCursor(0, i + 1);
    if (i == selectedItem) {
      lcd.print("> " + String(menuItems[i]));
    } else {
      lcd.print("  " + String(menuItems[i]));
    }
  }
  
  lcd.setCursor(0, 3);
  lcd.print("U/D:Nav A:Select B:Back");
}

void handleUltrasonicFrontSettings() {
  if (UP()) {
    // Navigate up
    if (selectedItem > 0) {
      selectedItem--;
    } else {
      selectedItem = maxItems - 1; // Wrap to bottom
    }
  } else if (DOWN()) {
    // Navigate down
    if (selectedItem < maxItems - 1) {
      selectedItem++;
    } else {
      selectedItem = 0; // Wrap to top
    }
  } else if (START()) {
    // Select submenu
    switch (selectedItem) {
      case 0: // Tengah
        currentMenu = MENU_ULTRASONIC_FRONT_TENGAH;
        break;
      case 1: // Serong
        currentMenu = MENU_ULTRASONIC_FRONT_SERONG;
        break;
    }
    selectedItem = 0;
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Back to ultrasonic settings
    currentMenu = MENU_ULTRASONIC_SETTINGS;
    selectedItem = 0;
    menuNeedsRefresh = true;
  }
}

// Ultrasonic Back Settings Functions
void displayUltrasonicBackSettings() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    selectedItem = 0;  // Reset selection
    maxItems = 2;      // 2 items: Tengah, Serong
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("Back Sensor");
  
  const char* menuItems[] = {
    "Tengah",
    "Serong"
  };
  
  // Display menu items
  for (int i = 0; i < maxItems; i++) {
    lcd.setCursor(0, i + 1);
    if (i == selectedItem) {
      lcd.print("> " + String(menuItems[i]));
    } else {
      lcd.print("  " + String(menuItems[i]));
    }
  }
  
  lcd.setCursor(0, 3);
  lcd.print("U/D:Nav A:Select B:Back");
}

void handleUltrasonicBackSettings() {
  if (UP()) {
    // Navigate up
    if (selectedItem > 0) {
      selectedItem--;
    } else {
      selectedItem = maxItems - 1; // Wrap to bottom
    }
  } else if (DOWN()) {
    // Navigate down
    if (selectedItem < maxItems - 1) {
      selectedItem++;
    } else {
      selectedItem = 0; // Wrap to top
    }
  } else if (START()) {
    // Select submenu
    switch (selectedItem) {
      case 0: // Tengah
        currentMenu = MENU_ULTRASONIC_BACK_TENGAH;
        break;
      case 1: // Serong
        currentMenu = MENU_ULTRASONIC_BACK_SERONG;
        break;
    }
    selectedItem = 0;
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Back to ultrasonic settings
    currentMenu = MENU_ULTRASONIC_SETTINGS;
    selectedItem = 1;
    menuNeedsRefresh = true;
  }
}

void displayMusicSubmenu(const char* title, int* currentPin) {
  static bool displayInitialized;  // Auto-initializes to false
  static int lastSelectedMusicPin = -1;  // -1 needs explicit init
  
  // Check if display needs refresh
  bool needsRefresh = !displayInitialized || 
                     selectedMusicPin != lastSelectedMusicPin ||
                     menuNeedsRefresh;
  
  if (needsRefresh) {
    lcd.clear();
    displayMenuHeader(title);
    
    // Display pin options (Pin 1 to Pin 5 + Silent) with scrolling
    // Calculate scroll position to show 2 pins at a time (baris 1 dan 2)
    int startIdx = 0;
    if (selectedMusicPin >= 2) {
      startIdx = selectedMusicPin - 1; // Keep selected item visible
    }
    if (startIdx > 4) startIdx = 4; // Max start index for 6 items (show last 2)
    
    for (int i = 0; i < 2; i++) { // Show 2 pins at a time
      int pinIndex = startIdx + i;
      if (pinIndex < 6) {
        lcd.setCursor(0, i + 1);
        
        // Clear the line first
        lcd.print("                    ");
        lcd.setCursor(0, i + 1);
        
        // Show cursor for selected pin
        if (pinIndex == selectedMusicPin) {
          lcd.print("> ");
        } else {
          lcd.print("  ");
        }
        
        if (pinIndex == 5) {
          lcd.print("Silent");
        } else {
          lcd.print("Sound ");
          lcd.print(pinIndex + 1);
        }
        
        // Show current pin indicator
        if (pinIndex == *currentPin) {
          lcd.print(" (Current)");
        }
      }
    }
    
    // Show controls at baris 3
    lcd.setCursor(0, 3);
    lcd.print("                    "); // Clear line
    lcd.setCursor(0, 3);
    lcd.print("U/D:Navi A:OK B:Back");
    
    // Update tracking variables
    displayInitialized = true;
    lastSelectedMusicPin = selectedMusicPin;
    menuNeedsRefresh = false;
  }
}

void handleMusicSubmenu(int* targetPin) {
  unsigned long currentMillis = millis();
  if (currentMillis - lastButtonPress >= buttonDelay) {
    if (UP()) {
      selectedMusicPin = (selectedMusicPin - 1 + 6) % 6;
      menuNeedsRefresh = true;
      lastButtonPress = currentMillis;
    } else if (DOWN()) {
      selectedMusicPin = (selectedMusicPin + 1) % 6;
      menuNeedsRefresh = true;
      lastButtonPress = currentMillis;
    } else if (START()) {
      // Save selection
      *targetPin = selectedMusicPin;
      
      // Apply all changes and save
      musicOnPin = musicOnPin;
      musicObstaclePin = musicObstaclePin;
      musicStationPin = musicStationPin;
      musicOutOfLinePin = musicOutOfLinePin;
      musicWarningPin = musicWarningPin;
      saveSettings();
      
      // Back to music settings
      currentMenu = MENU_MUSIC_SETTINGS;
      menuNeedsRefresh = true;
      lastButtonPress = currentMillis;
    } else if (STOP()) {
      // Back to music settings without saving
      currentMenu = MENU_MUSIC_SETTINGS;
      menuNeedsRefresh = true;
      lastButtonPress = currentMillis;
    }
  }
}

void displayMusicSettings() {
  static bool displayInitialized;  // Auto-initializes to false
  static int lastSelectedMusicItem = -1;  // -1 needs explicit init
  static int musicMenuStartIndex;  // Auto-initializes to 0
  static int lastMusicMenuStartIndex = -1;  // -1 needs explicit init
  
  // Music mode labels (5 main categories)
  String musicModes[5] = { "On", "Obstacle", "Station", "OutOfLine", "Warning" };
  int maxMusicDisplay = 2; // Show 2 items at a time (to avoid overlap with controls)
  
  // Update scroll position if needed
  if (selectedMusicItem < musicMenuStartIndex) {
    musicMenuStartIndex = selectedMusicItem;
    menuNeedsRefresh = true;
  } else if (selectedMusicItem >= musicMenuStartIndex + maxMusicDisplay) {
    musicMenuStartIndex = selectedMusicItem - maxMusicDisplay + 1;
    menuNeedsRefresh = true;
  }
  
  // Check if display needs refresh
  bool needsRefresh = !displayInitialized || 
                     selectedMusicItem != lastSelectedMusicItem ||
                     musicMenuStartIndex != lastMusicMenuStartIndex ||
                     menuNeedsRefresh;
  
  if (needsRefresh) {
    lcd.clear();
    displayMenuHeader("Music Settings");

    // Display music items with scrolling (2 items max)
    for (int i = 0; i < maxMusicDisplay && (musicMenuStartIndex + i) < maxMusicItems; i++) {
      int musicIndex = musicMenuStartIndex + i;
      lcd.setCursor(0, i + 1);

      // Show cursor for selected item
      if (musicIndex == selectedMusicItem) {
        lcd.print("> ");
      } else {
        lcd.print("  ");
      }

      // Show mode name only
      lcd.print(musicModes[musicIndex]);
    }
    
    // Show controls at bottom (baris 3)
    lcd.setCursor(0, 3);
    lcd.print("A:Select B:Back");
    
    // Update tracking variables
    displayInitialized = true;
    lastSelectedMusicItem = selectedMusicItem;
    lastMusicMenuStartIndex = musicMenuStartIndex;
    menuNeedsRefresh = false;
  }
}

void handleMusicSettings() {
  static bool displayInitialized;  // Auto-initializes to false
  
  if (UP()) {
    selectedMusicItem = (selectedMusicItem - 1 + maxMusicItems) % maxMusicItems;
    displayInitialized = false;
  } else if (DOWN()) {
    selectedMusicItem = (selectedMusicItem + 1) % maxMusicItems;
    displayInitialized = false;
  } else if (START()) {
    // Enter submenu based on selected item
    switch (selectedMusicItem) {
      case 0: 
        selectedMusicPin = musicOnPin;
        currentMenu = MENU_MUSIC_ON; 
        break;
      case 1: 
        selectedMusicPin = musicObstaclePin;
        currentMenu = MENU_MUSIC_OBSTACLE; 
        break;
      case 2: 
        selectedMusicPin = musicStationPin;
        currentMenu = MENU_MUSIC_STATION; 
        break;
      case 3: 
        selectedMusicPin = musicOutOfLinePin;
        currentMenu = MENU_MUSIC_OUTOFLINE; 
        break;
      case 4: 
        selectedMusicPin = musicWarningPin;
        currentMenu = MENU_MUSIC_WARNING; 
        break;
    }
    displayInitialized = false;
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Back to main menu
    displayInitialized = false;
    currentMenu = MENU_MAIN;
    selectedMusicItem = 0;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}

// =============================================
// MOTOR SETTINGS SUB MENU FUNCTIONS
// =============================================

void displaySpeedSetting() {
  static int lastMaxMotorRpm = -1;
  
  if (menuNeedsRefresh || maxMotorRpm != lastMaxMotorRpm) {
    lcd.clear();
    displayMenuHeader("Speed Setting:");
    
    lcd.setCursor(0, 1);
    lcd.print("Max RPM:");
    
    lcd.setCursor(0, 2);
    lcd.print("> ");
    lcd.print(maxMotorRpm);
    lcd.print(" RPM");
    
    // Show range
    lcd.setCursor(0, 3);
    lcd.print("Range: 10-90 RPM");
    
    menuNeedsRefresh = false;
    lastMaxMotorRpm = maxMotorRpm;
  }
}

void handleSpeedSetting() {
  if (LEFT()) {
    maxMotorRpm = max(10, maxMotorRpm - 5);
    menuNeedsRefresh = true;
  } else if (RIGHT()) {
    maxMotorRpm = min(90, maxMotorRpm + 5);
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Save speed setting and send to motor controller
    maxMotorRpm = maxMotorRpm;
    
    // Save to preferences
    preferences.begin("agv-settings", false);
    preferences.putInt("maxMotorRpm", maxMotorRpm);
    preferences.end();
    
    // Back to Motor Settings menu
    lcd.clear();
    currentMenu = MENU_MOTOR_SETTINGS;
    selectedItem = 0;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}

void displayPidRpmSetting() {
  static int lastSelectedItem = -1;
  
  if (menuNeedsRefresh || selectedItem != lastSelectedItem) {
    lcd.clear();
    displayMenuHeader("PID RPM Setting:");
    
    String menuItems[2] = {
      "Right Motor",       // selectedItem 0 -> MENU_PID_RPM_RIGHT (48)
      "Left Motor"         // selectedItem 1 -> MENU_PID_RPM_LEFT (59)
    };
    
    for (int i = 0; i < 2; i++) {
      lcd.setCursor(0, i + 1);
      displayIndicator(i, selectedItem);
      lcd.print(menuItems[i]);
    }
    
    displayMenuFooter("A:Select B:Back");
    
    menuNeedsRefresh = false;
    lastSelectedItem = selectedItem;
  }
}

void handlePidRpmSetting() {
  if (UP()) {
    selectedItem = (selectedItem - 1 + 2) % 2;
    menuNeedsRefresh = true;
  } else if (DOWN()) {
    selectedItem = (selectedItem + 1) % 2;
    menuNeedsRefresh = true;
  } else if (START()) {
    lcd.clear();
    switch (selectedItem) {
      case 0:  // Right Motor
        currentMenu = MENU_PID_RPM_RIGHT;
        selectedItem = 0;  // Reset for parameter selection
        break;
      case 1:  // Left Motor
        currentMenu = MENU_PID_RPM_LEFT;
        selectedItem = 0;  // Reset for parameter selection
        break;
    }
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Back to Motor Settings menu
    lcd.clear();
    currentMenu = MENU_MOTOR_SETTINGS;
    selectedItem = 1;  // Return to PID RPM item
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}

// ===============================================
// ULTRASONIC FRONT TENGAH SETTINGS
// ===============================================
void displayUltrasonicFrontTengahSettings() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("Front Tengah");
  
  lcd.setCursor(0, 1);
  lcd.print("Min Safe Distance:");
  
  lcd.setCursor(0, 2);
  lcd.print(String(minSafeDistanceFront) + " cm");
  
  lcd.setCursor(0, 3);
  lcd.print("L/R:Adj A:Save B:Back");
}

void handleUltrasonicFrontTengahSettings() {
  if (LEFT()) {
    // Decrease by 1 cm
    if (minSafeDistanceFront > 5) {  // Minimum 5 cm
      minSafeDistanceFront--;
    }
  } else if (RIGHT()) {
    // Increase by 1 cm
    if (minSafeDistanceFront < 100) {  // Maximum 100 cm
      minSafeDistanceFront++;
    }
  } else if (START()) {
    // Save settings
    minSafeDistanceFront = minSafeDistanceFront;
    
    // Save to preferences
    preferences.begin("agv-settings", false);
    preferences.putUShort("SafeDistFront", minSafeDistanceFront);
    preferences.end();
    
    // Show confirmation
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Front Tengah saved!");
    lcd.setCursor(0, 2);
    lcd.print("Min distance: " + String(minSafeDistanceFront) + "cm");
    delay(MESSAGE_DISPLAY_DURATION);
    
    currentMenu = MENU_ULTRASONIC_FRONT;
    selectedItem = 0;
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Cancel changes
    minSafeDistanceFront = minSafeDistanceFront;
    currentMenu = MENU_ULTRASONIC_FRONT;
    selectedItem = 0;
    menuNeedsRefresh = true;
  }
}

// ===============================================
// ULTRASONIC FRONT SERONG SETTINGS
// ===============================================
void displayUltrasonicFrontSerongSettings() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("Front Serong");
  
  lcd.setCursor(0, 1);
  lcd.print("Min Safe Distance:");
  
  lcd.setCursor(0, 2);
  lcd.print(String(minSafeDistanceFrontSerong) + " cm");
  
  lcd.setCursor(0, 3);
  lcd.print("L/R:Adj A:Save B:Back");
}

void handleUltrasonicFrontSerongSettings() {
  if (LEFT()) {
    // Decrease by 1 cm
    if (minSafeDistanceFrontSerong > 5) {  // Minimum 5 cm
      minSafeDistanceFrontSerong--;
    }
  } else if (RIGHT()) {
    // Increase by 1 cm
    if (minSafeDistanceFrontSerong < 100) {  // Maximum 100 cm
      minSafeDistanceFrontSerong++;
    }
  } else if (START()) {
    // Save settings
    minSafeDistanceFrontSerong = minSafeDistanceFrontSerong;
    
    // Save to preferences
    preferences.begin("agv-settings", false);
    preferences.putUShort("SafeDistFrontS", minSafeDistanceFrontSerong);
    preferences.end();
    
    // Show confirmation
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Front Serong saved!");
    lcd.setCursor(0, 2);
    lcd.print("Min distance: " + String(minSafeDistanceFrontSerong) + "cm");
    delay(MESSAGE_DISPLAY_DURATION);
    
    currentMenu = MENU_ULTRASONIC_FRONT;
    selectedItem = 1;
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Cancel changes
    minSafeDistanceFrontSerong = minSafeDistanceFrontSerong;
    currentMenu = MENU_ULTRASONIC_FRONT;
    selectedItem = 1;
    menuNeedsRefresh = true;
  }
}

// ===============================================
// ULTRASONIC BACK TENGAH SETTINGS
// ===============================================
void displayUltrasonicBackTengahSettings() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("Back Tengah");
  
  lcd.setCursor(0, 1);
  lcd.print("Min Safe Distance:");
  
  lcd.setCursor(0, 2);
  lcd.print(String(minSafeDistanceBack) + " cm");
  
  lcd.setCursor(0, 3);
  lcd.print("L/R:Adj A:Save B:Back");
}

void handleUltrasonicBackTengahSettings() {
  if (LEFT()) {
    // Decrease by 1 cm
    if (minSafeDistanceBack > 5) {  // Minimum 5 cm
      minSafeDistanceBack--;
    }
  } else if (RIGHT()) {
    // Increase by 1 cm
    if (minSafeDistanceBack < 100) {  // Maximum 100 cm
      minSafeDistanceBack++;
    }
  } else if (START()) {
    // Save settings
    minSafeDistanceBack = minSafeDistanceBack;
    
    // Save to preferences
    preferences.begin("agv-settings", false);
    preferences.putUShort("SafeDistBack", minSafeDistanceBack);
    preferences.end();
    
    // Show confirmation
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Back Tengah saved!");
    lcd.setCursor(0, 2);
    lcd.print("Min distance: " + String(minSafeDistanceBack) + "cm");
    delay(MESSAGE_DISPLAY_DURATION);
    
    currentMenu = MENU_ULTRASONIC_BACK;
    selectedItem = 0;
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Cancel changes
    minSafeDistanceBack = minSafeDistanceBack;
    currentMenu = MENU_ULTRASONIC_BACK;
    selectedItem = 0;
    menuNeedsRefresh = true;
  }
}

// ===============================================
// ULTRASONIC BACK SERONG SETTINGS
// ===============================================
void displayUltrasonicBackSerongSettings() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("Back Serong");
  
  lcd.setCursor(0, 1);
  lcd.print("Min Safe Distance:");
  
  lcd.setCursor(0, 2);
  lcd.print(String(minSafeDistanceBackSerong) + " cm");
  
  lcd.setCursor(0, 3);
  lcd.print("L/R:Adj A:Save B:Back");
}

void handleUltrasonicBackSerongSettings() {
  if (LEFT()) {
    // Decrease by 1 cm
    if (minSafeDistanceBackSerong > 5) {  // Minimum 5 cm
      minSafeDistanceBackSerong--;
    }
  } else if (RIGHT()) {
    // Increase by 1 cm
    if (minSafeDistanceBackSerong < 100) {  // Maximum 100 cm
      minSafeDistanceBackSerong++;
    }
  } else if (START()) {
    // Save settings
    minSafeDistanceBackSerong = minSafeDistanceBackSerong;
    
    // Save to preferences
    preferences.begin("agv-settings", false);
    preferences.putUShort("SafeDistBackS", minSafeDistanceBackSerong);
    preferences.end();
    
    // Show confirmation
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Back Serong saved!");
    lcd.setCursor(0, 2);
    lcd.print("Min distance: " + String(minSafeDistanceBackSerong) + "cm");
    delay(MESSAGE_DISPLAY_DURATION);
    
    currentMenu = MENU_ULTRASONIC_BACK;
    selectedItem = 1;
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Cancel changes
    minSafeDistanceBackSerong = minSafeDistanceBackSerong;
    currentMenu = MENU_ULTRASONIC_BACK;
    selectedItem = 1;
    menuNeedsRefresh = true;
  }
}

// ===============================================
// RPM TUNING MENU FUNCTIONS
// ===============================================

void displayRpmTuningMenu() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("RPM Tuning");

  // Display Tune Both option
  lcd.setCursor(0, 1);
  if (selectedTuningItem == 0) {
    lcd.print("> Tune Both");
  } else {
    lcd.print("  Tune Both");
  }

  // Display Tune Right option
  lcd.setCursor(0, 2);
  if (selectedTuningItem == 1) {
    lcd.print("> Tune Right");
  } else {
    lcd.print("  Tune Right");
  }

  // Display Tune Left option
  lcd.setCursor(0, 3);
  if (selectedTuningItem == 2) {
    lcd.print("> Tune Left");
  } else {
    lcd.print("  Tune Left");
  }
  
  // Show controls hint at the right side of line 3
  lcd.setCursor(12, 3);
  lcd.print("A:OK B:<");
}

void handleRpmTuningMenu() {
  if (UP()) {
    if (selectedTuningItem > 0) {
      selectedTuningItem--;
      menuNeedsRefresh = true;
    }
  } else if (DOWN()) {
    if (selectedTuningItem < 2) {
      selectedTuningItem++;
      menuNeedsRefresh = true;
    }
  } else if (START()) {
    // Execute selected action
    switch (selectedTuningItem) {
      case 0: // Tune Both
        currentMenu = MENU_RPM_TUNE_START;
        selectedTuningSubItem = 0;
        menuNeedsRefresh = true;
        break;
        
      case 1: // Tune Right
        currentMenu = MENU_RPM_TUNE_RIGHT;
        selectedTuningSubItem = 0;
        menuNeedsRefresh = true;
        break;
        
      case 2: // Tune Left
        currentMenu = MENU_RPM_TUNE_LEFT;
        selectedTuningSubItem = 0;
        menuNeedsRefresh = true;
        break;
    }
  } else if (STOP()) {
    // Back to main menu
    currentMenu = MENU_MAIN;
    selectedItem = 16; // Keep "Tuning RPM" selected
    menuNeedsRefresh = true;
  }
}

void displayTuningStatus() {
  // Auto-refresh status every 2 seconds when in status view
  unsigned long currentMillis = millis();
  if (currentMillis - lastTuningStatusRequest >= TUNING_STATUS_INTERVAL) {
    sendTuningCommand("STATUS");
    lastTuningStatusRequest = currentMillis;
  }
  
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("Tuning Status");

  lcd.setCursor(0, 1);
  lcd.print("Status: " + tuningStatus);
  
  if (tuningStatus == "PROGRESS" || tuningStatus.startsWith("PROGRESS")) {
    lcd.setCursor(0, 2);
    lcd.print("Progress: " + String(tuningProgress) + "%");
    
    // Progress bar visualization
    lcd.setCursor(0, 3);
    int barLength = (tuningProgress * 20) / 100; // 20 chars max
    for (int i = 0; i < 20; i++) {
      if (i < barLength) {
        lcd.print("=");
      } else if (i == barLength && tuningProgress > 0) {
        lcd.print(">");
      } else {
        lcd.print(" ");
      }
    }
  } else if (tuningStatus == "IDLE") {
    lcd.setCursor(0, 2);
    lcd.print("Tuning tidak aktif");
    lcd.setCursor(0, 3);
    lcd.print("B:Back");
  } else if (tuningStatus == "STARTED") {
    lcd.setCursor(0, 2);
    lcd.print("Tuning dimulai");
    lcd.setCursor(0, 3);
    lcd.print("Tunggu beberapa");
  } else if (tuningStatus == "CANCELLED") {
    lcd.setCursor(0, 2);
    lcd.print("Tuning dibatalkan");
    lcd.setCursor(0, 3);
    lcd.print("B:Back");
  } else {
    lcd.setCursor(0, 2);
    lcd.print("Status: " + tuningStatus);
    lcd.setCursor(0, 3);
    lcd.print("B:Back");
  }
  
  // Handle back button
  if (STOP()) {
    currentMenu = MENU_RPM_TUNING;
    selectedTuningItem = 2; // Keep "Tuning Status" selected
    menuNeedsRefresh = true;
  }
}

void sendTuningCommand(String command) {
  // Send command to motor controller slave via Serial
  Serial.println(command);
  
  // Debug output
  Serial.println("Sending tuning command: " + command);
}

void parseTuningResponse(String response) {
  // Parse responses from motor controller slave
  response.trim();
  
  if (response.startsWith("AUTOTUNE:")) {
    String statusPart = response.substring(9); // Remove "AUTOTUNE:" prefix
    
    if (statusPart == "STARTED") {
      tuningStatus = "STARTED";
      tuningProgress = 0;
    } else if (statusPart == "ALREADY_RUNNING") {
      tuningStatus = "RUNNING";
    } else if (statusPart == "CANCELLED") {
      tuningStatus = "CANCELLED";
      tuningProgress = 0;
    } else if (statusPart == "NOT_RUNNING") {
      tuningStatus = "IDLE";
      tuningProgress = 0;
    } else if (statusPart == "IDLE") {
      tuningStatus = "IDLE";
      tuningProgress = 0;
    } else if (statusPart.startsWith("PROGRESS:")) {
      tuningStatus = "PROGRESS";
      String progressStr = statusPart.substring(9); // Remove "PROGRESS:" prefix
      tuningProgress = progressStr.toInt();
      // Ensure progress is within valid range
      tuningProgress = constrain(tuningProgress, 0, 100);
    } else if (statusPart.startsWith("COMPLETED:")) {
      tuningStatus = "COMPLETED";
      tuningProgress = 100;
      // Extract PID values if available
      String pidData = statusPart.substring(10); // Remove "COMPLETED:" prefix
      Serial.println("Auto-tuning completed with PID: " + pidData);
    }
    
    // If we're currently viewing status, refresh the display
    if (currentMenu == MENU_RPM_TUNE_STATUS) {
      menuNeedsRefresh = true;
    }
    
    // Debug output
    Serial.println("Tuning status updated: " + tuningStatus + " (" + String(tuningProgress) + "%)");
  } else if (response.startsWith("AUTOTUNE_RIGHT:")) {
    String statusPart = response.substring(15); // Remove "AUTOTUNE_RIGHT:" prefix
    
    if (statusPart == "STARTED") {
      tuningStatus = "RIGHT STARTED";
      tuningProgress = 0;
    } else if (statusPart.startsWith("COMPLETED:")) {
      tuningStatus = "RIGHT COMPLETED";
      tuningProgress = 100;
      String pidData = statusPart.substring(10); // Remove "COMPLETED:" prefix
    }
    
    if (currentMenu == MENU_RPM_TUNE_STATUS) {
      menuNeedsRefresh = true;
    }
    
    Serial.println("Right motor tuning status: " + tuningStatus);
  } else if (response.startsWith("AUTOTUNE_LEFT:")) {
    String statusPart = response.substring(14); // Remove "AUTOTUNE_LEFT:" prefix
    
    if (statusPart == "STARTED") {
      tuningStatus = "LEFT STARTED";
      tuningProgress = 0;
    } else if (statusPart.startsWith("COMPLETED:")) {
      tuningStatus = "LEFT COMPLETED";
      tuningProgress = 100;
      String pidData = statusPart.substring(10); // Remove "COMPLETED:" prefix
    }
    
    if (currentMenu == MENU_RPM_TUNE_STATUS) {
      menuNeedsRefresh = true;
    }
  }
}

// ====== PID RPM RIGHT MOTOR FUNCTIONS ======
void displayPidRpmRight() {
  static int lastSelectedParam = -1;
  static double lastValues[3] = {-1, -1, -1};
  
  if (menuNeedsRefresh || selectedItem != lastSelectedParam ||
      motorPidKpRight != lastValues[0] || motorPidKiRight != lastValues[1] || 
      motorPidKdRight != lastValues[2]) {
    
    lcd.clear();
    displayMenuHeader("PID Right Motor:");
    
    String params[3] = {"Kp:", "Ki:", "Kd:"};
    double values[3] = {motorPidKpRight, motorPidKiRight, motorPidKdRight};
    
    for (int i = 0; i < 3; i++) {
      lcd.setCursor(0, i + 1);
      if (i == selectedItem) {
        lcd.print("> ");
      } else {
        lcd.print("  ");
      }
      lcd.print(params[i]);
      lcd.print(values[i], 3);
    }
    
    menuNeedsRefresh = false;
    lastSelectedParam = selectedItem;
    lastValues[0] = motorPidKpRight;
    lastValues[1] = motorPidKiRight;
    lastValues[2] = motorPidKdRight;
  }
}

void handlePidRpmRight() {
  // Handle UP/DOWN navigation
  if (UP()) {
    selectedItem = (selectedItem - 1 + 3) % 3;
    menuNeedsRefresh = true;
  } else if (DOWN()) {
    selectedItem = (selectedItem + 1) % 3;
    menuNeedsRefresh = true;
  }

  // Create array of pointers to PID values
  double* pidValues[3] = {
    &motorPidKpRight,
    &motorPidKiRight,
    &motorPidKdRight
  };
  
  // Use reusable button handler with different increments per parameter
  if (selectedItem == 0) {
    // Kp: larger increment (0.1)
    handlePidButtonAdjustment(pidValues, selectedItem, 0.1f, 0.5f, 100);
  } else {
    // Ki, Kd: smaller increment (0.01)
    handlePidButtonAdjustment(pidValues, selectedItem, 0.01f, 0.1f, 100);
  }
  
  // Check if values changed for refresh
  static double lastKp = -1, lastKi = -1, lastKd = -1;
  if (motorPidKpRight != lastKp || motorPidKiRight != lastKi || motorPidKdRight != lastKd) {
    menuNeedsRefresh = true;
    lastKp = motorPidKpRight;
    lastKi = motorPidKiRight;
    lastKd = motorPidKdRight;
  }

  // Handle save and back
  if (START() || STOP()) {
    motorPidKpRight = motorPidKpRight;
    motorPidKiRight = motorPidKiRight;
    motorPidKdRight = motorPidKdRight;
    sendPidValuesRight(motorPidKpRight, motorPidKiRight, motorPidKdRight);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PID Right Sent!");
    lcd.setCursor(0, 1);
    lcd.print("Kp:" + String(motorPidKpRight, 2));
    lcd.setCursor(0, 2);
    lcd.print("Ki:" + String(motorPidKiRight, 3));
    lcd.setCursor(0, 3);
    lcd.print("Kd:" + String(motorPidKdRight, 3));
    delay(SHORT_DISPLAY_DURATION);
    
    lcd.clear();
    currentMenu = MENU_PID_RPM_SETTING;
    selectedItem = 0;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}

// ====== PID RPM LEFT MOTOR FUNCTIONS ======
void displayPidRpmLeft() {
  static int lastSelectedParam = -1;
  static double lastValues[3] = {-1, -1, -1};
  
  if (menuNeedsRefresh || selectedItem != lastSelectedParam ||
      motorPidKpLeft != lastValues[0] || motorPidKiLeft != lastValues[1] || 
      motorPidKdLeft != lastValues[2]) {
    
    lcd.clear();
    displayMenuHeader("PID Left Motor:");
    
    String params[3] = {"Kp:", "Ki:", "Kd:"};
    double values[3] = {motorPidKpLeft, motorPidKiLeft, motorPidKdLeft};
    
    for (int i = 0; i < 3; i++) {
      lcd.setCursor(0, i + 1);
      if (i == selectedItem) {
        lcd.print("> ");
      } else {
        lcd.print("  ");
      }
      lcd.print(params[i]);
      lcd.print(values[i], 3);
    }
    
    menuNeedsRefresh = false;
    lastSelectedParam = selectedItem;
    lastValues[0] = motorPidKpLeft;
    lastValues[1] = motorPidKiLeft;
    lastValues[2] = motorPidKdLeft;
  }
}

void handlePidRpmLeft() {
  if (UP()) {
    selectedItem = (selectedItem - 1 + 3) % 3;
    menuNeedsRefresh = true;
  } else if (DOWN()) {
    selectedItem = (selectedItem + 1) % 3;
    menuNeedsRefresh = true;
  } else if (LEFT()) {
    // Decrease selected parameter
    switch (selectedItem) {
      case 0:  // Kp
        motorPidKpLeft = max(0.0, motorPidKpLeft - 0.1);
        break;
      case 1:  // Ki
        motorPidKiLeft = max(0.0, motorPidKiLeft - 0.01);
        break;
      case 2:  // Kd
        motorPidKdLeft = max(0.0, motorPidKdLeft - 0.01);
        break;
    }
    menuNeedsRefresh = true;
  } else if (RIGHT()) {
    // Increase selected parameter
    switch (selectedItem) {
      case 0:  // Kp
        motorPidKpLeft = min(200.0, motorPidKpLeft + 0.1);
        break;
      case 1:  // Ki
        motorPidKiLeft = min(200.0, motorPidKiLeft + 0.01);
        break;
      case 2:  // Kd
        motorPidKdLeft = min(200.0, motorPidKdLeft + 0.01); 
        break;
    }
    menuNeedsRefresh = true;
  }

  // Create array of pointers to PID values
  double* pidValues[3] = {
    &motorPidKpLeft,
    &motorPidKiLeft,
    &motorPidKdLeft
  };
  
  // Use reusable button handler with different increments per parameter
  if (selectedItem == 0) {
    // Kp: larger increment (0.1)
    handlePidButtonAdjustment(pidValues, selectedItem, 0.1f, 0.5f, 100);
  } else {
    // Ki, Kd: smaller increment (0.01)
    handlePidButtonAdjustment(pidValues, selectedItem, 0.01f, 0.1f, 100);
  }
  
  // Check if values changed for refresh
  static double lastKp = -1, lastKi = -1, lastKd = -1;
  if (motorPidKpLeft != lastKp || motorPidKiLeft != lastKi || motorPidKdLeft != lastKd) {
    menuNeedsRefresh = true;
    lastKp = motorPidKpLeft;
    lastKi = motorPidKiLeft;
    lastKd = motorPidKdLeft;
  }

  // Handle save and back
  if (START() || STOP()) {
    motorPidKpLeft = motorPidKpLeft;
    motorPidKiLeft = motorPidKiLeft;
    motorPidKdLeft = motorPidKdLeft;
    sendPidValuesLeft(motorPidKpLeft, motorPidKiLeft, motorPidKdLeft);
    
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Left PID saved:");
    lcd.setCursor(0, 2);
    lcd.print("Kp=" + String(motorPidKpLeft, 2) + " Ki=" + String(motorPidKiLeft, 3));
    lcd.setCursor(0, 3);
    lcd.print("Kd=" + String(motorPidKdLeft, 3));
    delay(MESSAGE_DISPLAY_DURATION);
    
    currentMenu = MENU_PID_RPM_SETTING;
    selectedItem = 1;
    menuNeedsRefresh = true;
  }
}

// Function to display tuning start menu for specific motor
void displayTuningStartMenu(String motorName) {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  // Create header string properly
  String headerText = "Tune " + motorName;
  displayMenuHeader(headerText.c_str());

  // Display Start option
  lcd.setCursor(0, 1);
  if (selectedTuningSubItem == 0) {
    lcd.print("> Start");
  } else {
    lcd.print("  Start");
  }

  // Display Cancel option
  lcd.setCursor(0, 2);
  if (selectedTuningSubItem == 1) {
    lcd.print("> Cancel");
  } else {
    lcd.print("  Cancel");
  }

  // Display Status option
  lcd.setCursor(0, 3);
  if (selectedTuningSubItem == 2) {
    lcd.print("> Status");
  } else {
    lcd.print("  Status");
  }
  
  // Show controls hint
  lcd.setCursor(12, 3);
  lcd.print("A:OK B:<");
}

// Function to handle tuning start menu for specific motor
void handleTuningStartMenu(String command) {
  if (UP()) {
    if (selectedTuningSubItem > 0) {
      selectedTuningSubItem--;
      menuNeedsRefresh = true;
    }
  } else if (DOWN()) {
    if (selectedTuningSubItem < 2) {
      selectedTuningSubItem++;
      menuNeedsRefresh = true;
    }
  } else if (START()) {
    // Execute selected action
    switch (selectedTuningSubItem) {
      case 0: // Start Tuning
        sendTuningCommand(command);
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Memulai tuning...");
        lcd.setCursor(0, 2);
        lcd.print("Mohon tunggu...");
        
        delay(MESSAGE_DISPLAY_DURATION);  // Show message for 2 seconds
        
        menuNeedsRefresh = true;
        break;
        
      case 1: // Cancel Tuning
        sendTuningCommand("CANCEL");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Membatalkan");
        lcd.setCursor(0, 2);
        lcd.print("tuning...");
        
        delay(MESSAGE_DISPLAY_DURATION);  // Show message for 2 seconds
        
        menuNeedsRefresh = true;
        break;
        
      case 2: // Tuning Status
        sendTuningCommand("TUNESTATUS");
        currentMenu = MENU_RPM_TUNE_STATUS;
        lastTuningStatusRequest = millis();
        menuNeedsRefresh = true;
        break;
    }
  } else if (STOP()) {
    // Back to tuning menu
    currentMenu = MENU_RPM_TUNING;
    selectedTuningItem = 0;
    menuNeedsRefresh = true;
  }
}
