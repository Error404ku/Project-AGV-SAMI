#include "menu.h"

// Using menu.h definitions only - removed duplicates
#define MENU_MUSIC_ON 40
#define MENU_MUSIC_OBSTACLE 41
#define MENU_MUSIC_STATION 42
#define MENU_MUSIC_OUTOFLINE 43
#define MENU_HOOK_TEST 22
#define MENU_MAGNET_CHECK 23
#define MENU_ULTRASONIC_CHECK 24
#define MENU_RESET_AGV_STATE 25

// Local menu constants for RFID submenus
#define MENU_RFID_UJUNG 15
#define MENU_RFID_WAREHOUSE 16
#define MENU_AUTO_INPUT_STATION 17
#define MENU_TERMINAL_DROP 26
#define MENU_TERMINAL_PICKUP 27
// ===================================================================
// MENU VARIABLES SUDAH DIPINDAHKAN KE config.h
// ===================================================================

// Konstanta MAX_MANUAL_TARGETS sudah didefinisikan di config.h

// Fungsi untuk inisialisasi temporary variables dari nilai asli
void initMenuTempVariables() {
  tempKp = kpLinefollower;
  tempKi = kiLinefollower;
  tempKd = kdLinefollower;

  tempKpForwardWithMassa = kpLinefollowerForwardWithMassa;
  tempKiForwardWithMassa = kiLinefollowerForwardWithMassa;
  tempKdForwardWithMassa = kdLinefollowerForwardWithMassa;
  tempKpForwardDefault = kpLinefollowerForwardDefault;
  tempKiForwardDefault = kiLinefollowerForwardDefault;
  tempKdForwardDefault = kdLinefollowerForwardDefault;
  tempKpBackwardWithMassa = kpLinefollowerBackwardWithMassa;
  tempKiBackwardWithMassa = kiLinefollowerBackwardWithMassa;
  tempKdBackwardWithMassa = kdLinefollowerBackwardWithMassa;
  tempKpBackwardDefault = kpLinefollowerBackwardDefault;
  tempKiBackwardDefault = kiLinefollowerBackwardDefault;
  tempKdBackwardDefault = kdLinefollowerBackwardDefault;
  tempBaseSpeed = baseSpeed;
  tempInvertY = invertMotorY;
  tempInvertX = invertMotorX;
  tempInvertKanan = invertMotorKanan;
  tempInvertKiri = invertMotorKiri;
  tempInvertHook = invertHook;
  tempMusicOnPin = musicOnPin;
  tempMusicObstaclePin = musicObstaclePin;
  tempMusicStationPin = musicStationPin;
  tempMusicOutOfLinePin = musicOutOfLinePin;
  selectedMusicPin = 0;
  tempMinSafeDistanceFront = minSafeDistanceFront;
  tempMinSafeDistanceBack = minSafeDistanceBack;
  
  // Initialize Motor Control temp variables
  tempMaxMotorRpm = maxMotorRpm;
  tempMotorPidKp = motorPidKp;
  tempMotorPidKi = motorPidKi;
  tempMotorPidKd = motorPidKd;
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

void saveSettings() {
  preferences.begin("agv-settings", false);
  delay(50);
  // Save PID values
  preferences.putDouble("kpLinefollower", tempKp);
  preferences.putDouble("kiLinefollower", tempKi);
  preferences.putDouble("kdLinefollower", tempKd);
  

  
  // Save Forward PID WithMassa values
  preferences.putDouble("kpFwdMassa", tempKpForwardWithMassa);
  preferences.putDouble("kiFwdMassa", tempKiForwardWithMassa);
  preferences.putDouble("kdFwdMassa", tempKdForwardWithMassa);
  
  // Save Forward PID Default values
  preferences.putDouble("kpFwdDefault", tempKpForwardDefault);
  preferences.putDouble("kiFwdDefault", tempKiForwardDefault);
  preferences.putDouble("kdFwdDefault", tempKdForwardDefault);
  
  // Save Backward PID WithMassa values
  preferences.putDouble("kpBwdMassa", tempKpBackwardWithMassa);
  preferences.putDouble("kiBwdMassa", tempKiBackwardWithMassa);
  preferences.putDouble("kdBwdMassa", tempKdBackwardWithMassa);
  
  // Save Backward PID Default values
  preferences.putDouble("kpBwdDefault", tempKpBackwardDefault);
  preferences.putDouble("kiBwdDefault", tempKiBackwardDefault);
  preferences.putDouble("kdBwdDefault", tempKdBackwardDefault);

  // Save Motor values
  preferences.putInt("baseSpeed", tempBaseSpeed);

  // Save Motor invert values
  preferences.putBool("invertY", tempInvertY);
  preferences.putBool("invertX", tempInvertX);
  preferences.putBool("invertKanan", tempInvertKanan);
  preferences.putBool("invertKiri", tempInvertKiri);
  preferences.putBool("invertHook", tempInvertHook);

  // Save Music mapping values
  preferences.putInt("musicOn", tempMusicOnPin);
  preferences.putInt("musicObstacle", tempMusicObstaclePin);
  preferences.putInt("musicStation", tempMusicStationPin);
  preferences.putInt("musicOutOfLine", tempMusicOutOfLinePin);
  preferences.putInt("musicWarning", tempMusicWarningPin);
  
  // Save Ultrasonic settings
  preferences.putUShort("SafeDistFront", tempMinSafeDistanceFront);
  preferences.putUShort("SafeDistBack", tempMinSafeDistanceBack);

  // Save Motor Control settings
  preferences.putInt("maxMotorRpm", tempMaxMotorRpm);
  preferences.putDouble("motorPidKp", tempMotorPidKp);
  preferences.putDouble("motorPidKi", tempMotorPidKi);
  preferences.putDouble("motorPidKd", tempMotorPidKd);

  // Apply PID values
  kpLinefollower = tempKp;
  kiLinefollower = tempKi;
  kdLinefollower = tempKd;
  

  
  // Apply Forward PID WithMassa values
  kpLinefollowerForwardWithMassa = tempKpForwardWithMassa;
  kiLinefollowerForwardWithMassa = tempKiForwardWithMassa;
  kdLinefollowerForwardWithMassa = tempKdForwardWithMassa;
  
  // Apply Forward PID Default values
  kpLinefollowerForwardDefault = tempKpForwardDefault;
  kiLinefollowerForwardDefault = tempKiForwardDefault;
  kdLinefollowerForwardDefault = tempKdForwardDefault;
  
  // Apply Backward PID WithMassa values
  kpLinefollowerBackwardWithMassa = tempKpBackwardWithMassa;
  kiLinefollowerBackwardWithMassa = tempKiBackwardWithMassa;
  kdLinefollowerBackwardWithMassa = tempKdBackwardWithMassa;
  
  // Apply Backward PID Default values
  kpLinefollowerBackwardDefault = tempKpBackwardDefault;
  kiLinefollowerBackwardDefault = tempKiBackwardDefault;
  kdLinefollowerBackwardDefault = tempKdBackwardDefault;

  // Debug: Print saved values in saveSettings
  // Serial.println() - removed for production

  // Apply Motor values
  baseSpeed = tempBaseSpeed;

  // Apply Motor invert values
  invertMotorY = tempInvertY;
  invertMotorX = tempInvertX;
  invertMotorKanan = tempInvertKanan;
  invertMotorKiri = tempInvertKiri;
  invertHook = tempInvertHook;

  // Apply Music mapping values
  musicOnPin = tempMusicOnPin;
  musicObstaclePin = tempMusicObstaclePin;
  musicStationPin = tempMusicStationPin;
  musicOutOfLinePin = tempMusicOutOfLinePin;
  
  // Apply Ultrasonic settings
  minSafeDistanceFront = tempMinSafeDistanceFront;
  minSafeDistanceBack = tempMinSafeDistanceBack;

  // End preferences session
  preferences.end();
}



void displayMainMenu() {
  // Menu items array
  String menuItems[16] = {
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
    "WiFi Settings"       // selectedItem 15 -> MENU_WIFI_SETTINGS (14)
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

  // If in AGV mode, only check for B button to exit
  if (isAgvMode) {
    if (STOP()) {
      isAgvMode = false;
      currentMenu = MENU_MAIN;
      agvMode(AGV_STATE_STOP);
      resetDisplayFlags(); // Reset semua flag display
      newRfidScanned = false; // Reset flag RFID saat keluar dari AGV mode
      menuNeedsRefresh = true;
    }
    return;
  }

  switch (currentMenu) {
    case MENU_MAIN:
      displayMainMenu();
      pwmMotor(0, 0);  // Use PWM stop command
      stopMusic();
      digitalWrite(lampPin, HIGH);
      if (currentMillis - lastButtonPress >= buttonDelay) {
        if (UP()) {
          selectedItem = (selectedItem - 1 + maxItems) % maxItems;
          lastButtonPress = currentMillis;
        } else if (DOWN()) {
          selectedItem = (selectedItem + 1) % maxItems;
          lastButtonPress = currentMillis;
        } else if (STOP()) {
          currentMenu = MENU_MAIN;
          selectedItem = 0;
          menuNeedsRefresh = true;  
        } else if (START()) {
          // Map selectedItem to corresponding menu constants
          switch (selectedItem) {
            case 0:  // AGV Mode
              isAgvMode = true;
              newRfidScanned = false; // Reset flag RFID saat masuk ke AGV mode
              lcd.clear(); // Membersihkan tampilan saat masuk ke mode AGV
              menuStartIndex = 0;  // Reset scroll position
              menuNeedsRefresh = true;
              break;
              
            case 1:  // Reset AGV State
              lcd.clear();
              currentMenu = MENU_RESET_AGV_STATE; // 25
              menuNeedsRefresh = true;
              break;
              
            case 2:  // Motor Test
              lcd.clear();
              selectedItem = 0;  // Reset selection for submenu
              currentMenu = MENU_MOTOR_TEST; // 2 - Go to submenu
              menuNeedsRefresh = true;
              break;
              
            case 3:  // PID Settings
              lcd.clear();
              initMenuTempVariables();  // Initialize temporary variables from global values
              currentMenu = MENU_PID_SETTINGS; // 3
              menuNeedsRefresh = true;
              break;
              
            case 4:  // Target Settings
              lcd.clear();
              currentMenu = MENU_TARGET_SETTINGS; // 4
              menuNeedsRefresh = true;
              break;
              
            case 5:  // Reset Settings
              lcd.clear();
              currentMenu = MENU_RESET; // 5
              menuNeedsRefresh = true;
              break;
              
            case 6:  // RFID Settings
              lcd.clear();
              currentMenu = MENU_RFID_SETTINGS; // 6
              menuNeedsRefresh = true;
              break;
              
            case 7:  // Motor Settings - Updated to show sub menu
              lcd.clear();
              currentMenu = MENU_MOTOR_SETTINGS; // 18
              selectedItem = 0;  // Reset selection for submenu
              menuNeedsRefresh = true;
              break;
              
            case 8:  // Motor Invert
              lcd.clear();
              currentMenu = MENU_MOTOR_INVERT; // 19
              menuNeedsRefresh = true;
              break;
              
            case 9:  // Music Settings
              lcd.clear();
              currentMenu = MENU_MUSIC_SETTINGS; // 20
              menuNeedsRefresh = true;
              break;
              
            case 10:  // Music Test
              lcd.clear();
              currentMenu = MENU_MUSIC_TEST; // 21
              menuNeedsRefresh = true;
              break;
              
            case 11:  // Hook Test
              lcd.clear();
              currentMenu = MENU_HOOK_TEST; // 22
              menuNeedsRefresh = true;
              break;
              
            case 12:  // Magnet Check
              lcd.clear();
              currentMenu = MENU_MAGNET_CHECK; // 23
              menuNeedsRefresh = true;
              break;
              
            case 13:  // Ultrasonic Check
              lcd.clear();
              currentMenu = MENU_ULTRASONIC_CHECK; // 24
              menuNeedsRefresh = true;
              break;
              
            case 14:  // Ultrasonic Settings
              lcd.clear();
              initMenuTempVariables();  // Initialize temporary variables
              tempMinSafeDistanceFront = minSafeDistanceFront;  // Initialize temp variables
              tempMinSafeDistanceBack = minSafeDistanceBack;
              selectedItem = 0;  // Reset selection
              currentMenu = MENU_ULTRASONIC_SETTINGS; // 30
              menuNeedsRefresh = true;
              break;
              
            case 15:  // WiFi Settings
              lcd.clear();
              currentMenu = MENU_WIFI_SETTINGS; // 14
              menuNeedsRefresh = true;
              break;
              
            default:
              // Fallback (should not happen with 15 items)
              currentMenu = MENU_MAIN;
              menuNeedsRefresh = true;
              break;
          }
          lastButtonPress = currentMillis;
        }
      }
      break;
    case MENU_MOTOR_TEST:
      displayMotorTest();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleMotorTest();
        if (UP() || DOWN() || START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_MOTOR_TEST_PWM:
      displayMotorTestPWM();
      handleMotorTestPWM();
      break;

    case MENU_MOTOR_TEST_RPM:
      requestRpmDataFromSlave();
      displayMotorTestRPM();
      handleMotorTestRPM();
      break;

    case MENU_PID_SETTINGS:
      displayPidSubmenu();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handlePidSubmenu();
        if (UP() || DOWN() || START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_PID_FORWARD:
      displayPidForwardSubmenu();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handlePidForwardSubmenu();
        if (UP() || DOWN() || START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_PID_BACKWARD:
      displayPidBackwardSubmenu();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handlePidBackwardSubmenu();
        if (UP() || DOWN() || START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
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
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleTargetSettings();
        if (LEFT() || RIGHT() || UP() || DOWN() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_RFID_SETTINGS:
      displayRfidSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleRfidSettings();
        if (LEFT() || RIGHT() || UP() || DOWN() || START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_MOTOR_SETTINGS:
      displayMotorSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleMotorSettings();
        if (LEFT() || RIGHT() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_SPEED_SETTING:
      displaySpeedSetting();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleSpeedSetting();
        if (LEFT() || RIGHT() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_PID_RPM_SETTING:
      displayPidRpmSetting();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handlePidRpmSetting();
        if (LEFT() || RIGHT() || UP() || DOWN() || START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_MOTOR_INVERT:
      {
        static bool displayInitialized = false;
        static int lastSelectedInvertItem = -1;
        static bool lastInvertValues[5] = {false, false, false, false, false};
        
        // Check if display needs refresh
        bool needsRefresh = !displayInitialized || 
                           selectedInvertItem != lastSelectedInvertItem ||
                           tempInvertY != lastInvertValues[0] ||
                           tempInvertX != lastInvertValues[1] ||
                           tempInvertKanan != lastInvertValues[2] ||
                           tempInvertKiri != lastInvertValues[3] ||
                           tempInvertHook != lastInvertValues[4];
        
        if (needsRefresh) {
          displayMenuHeader("Motor Invert");

          // Calculate what items to show (3 items max, with scrolling)
          int startIdx = max(0, min(selectedInvertItem - 1, maxInvertItems - 3));

          String invertLabels[5] = { "Y-Axis", "X-Axis", "M-Kanan", "M-Kiri", "Hook" };
          bool* invertValues[5] = { &tempInvertY, &tempInvertX, &tempInvertKanan, &tempInvertKiri, &tempInvertHook };

          for (int i = 0; i < 3 && (startIdx + i) < maxInvertItems; i++) {
            int itemIndex = startIdx + i;
            lcd.setCursor(0, i + 1);

            // Clear line first
            lcd.print("                    ");
            lcd.setCursor(0, i + 1);

            // Show cursor for selected item
            if (itemIndex == selectedInvertItem) {
              lcd.print("> ");
            } else {
              lcd.print("  ");
            }

            // Show label and value
            lcd.print(invertLabels[itemIndex]);
            lcd.print(": ");
            lcd.print(*invertValues[itemIndex] ? "Yes" : "No");

            // Show controls on the right
            if (i == 0) {
              lcd.setCursor(12, i + 1);
              lcd.print("UP/DN:Nav");
            } else if (i == 1) {
              lcd.setCursor(12, i + 1);
              lcd.print("LF/RT:Set");
            } else if (i == 2) {
              lcd.setCursor(12, i + 1);
              lcd.print("A:OK B:Back");
            }
          }
          
          // Update tracking variables
          displayInitialized = true;
          lastSelectedInvertItem = selectedInvertItem;
          lastInvertValues[0] = tempInvertY;
          lastInvertValues[1] = tempInvertX;
          lastInvertValues[2] = tempInvertKanan;
          lastInvertValues[3] = tempInvertKiri;
          lastInvertValues[4] = tempInvertHook;
        }

        if (currentMillis - lastButtonPress >= buttonDelay) {
          if (UP()) {
            selectedInvertItem = (selectedInvertItem - 1 + maxInvertItems) % maxInvertItems;
            lastButtonPress = currentMillis;
          } else if (DOWN()) {
            selectedInvertItem = (selectedInvertItem + 1) % maxInvertItems;
            lastButtonPress = currentMillis;
          } else if (LEFT() || RIGHT()) {
            // Toggle selected item
            switch (selectedInvertItem) {
              case 0: tempInvertY = !tempInvertY; break;
              case 1: tempInvertX = !tempInvertX; break;
              case 2: tempInvertKanan = !tempInvertKanan; break;
              case 3: tempInvertKiri = !tempInvertKiri; break;
              case 4: tempInvertHook = !tempInvertHook; break;
            }
            lastButtonPress = currentMillis;
          } else if (START()) {
            // Save all settings
            invertMotorY = tempInvertY;
            invertMotorX = tempInvertX;
            invertMotorKanan = tempInvertKanan;
            invertMotorKiri = tempInvertKiri;
            invertHook = tempInvertHook;
            saveSettings();
            // Reset display cache
            displayInitialized = false;
            currentMenu = MENU_MAIN;
            selectedInvertItem = 0;
            menuStartIndex = 0;
            menuNeedsRefresh = true;
            lastButtonPress = currentMillis;
          } else if (STOP()) {
            // Cancel changes
            tempInvertY = invertMotorY;
            tempInvertX = invertMotorX;
            tempInvertKanan = invertMotorKanan;
            tempInvertKiri = invertMotorKiri;
            tempInvertHook = invertHook;
            // Reset display cache
            displayInitialized = false;
            currentMenu = MENU_MAIN;
            selectedInvertItem = 0;
            menuStartIndex = 0;
            menuNeedsRefresh = true;
            lastButtonPress = currentMillis;
          }
        }
      }
      break;

    case MENU_MUSIC_SETTINGS:
      displayMusicSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleMusicSettings();
        if (UP() || DOWN() || START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_MUSIC_TEST:
      displayMusicTest();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleMusicTest();
        if (UP() || DOWN() || LEFT() || RIGHT() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_MUSIC_ON:
      displayMusicSubmenu("On Music", &tempMusicOnPin);
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleMusicSubmenu(&tempMusicOnPin);
      }
      break;

    case MENU_MUSIC_OBSTACLE:
      displayMusicSubmenu("Obstacle Music", &tempMusicObstaclePin);
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleMusicSubmenu(&tempMusicObstaclePin);
      }
      break;

    case MENU_MUSIC_STATION:
      displayMusicSubmenu("Station Music", &tempMusicStationPin);
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleMusicSubmenu(&tempMusicStationPin);
      }
      break;

    case MENU_MUSIC_OUTOFLINE:
      displayMusicSubmenu("OutOfLine Music", &tempMusicOutOfLinePin);
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleMusicSubmenu(&tempMusicOutOfLinePin);
      }
      break;

    case MENU_MUSIC_WARNING:
      displayMusicSubmenu("Warning Music", &tempMusicWarningPin);
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleMusicSubmenu(&tempMusicWarningPin);
      }
      break;

    case MENU_HOOK_TEST:
      displayHookTest();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleHookTest();
        if (UP() || DOWN() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_RESET:
      displayResetMenu();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleResetMenu();
        if (START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;
      
    case MENU_RESET_AGV_STATE:
      displayResetAgvStateMenu();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleResetAgvStateMenu();
        if (START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_MAGNET_CHECK:
      displayMagnetCheck();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleMagnetCheck();
        if (STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;
 
    case MENU_ULTRASONIC_CHECK:
    checkObstacles();
      displayUltrasonicCheck();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleUltrasonicCheck();
        if (STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_ULTRASONIC_SETTINGS:
      displayUltrasonicSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleUltrasonicSettings();
        if (UP() || DOWN() || START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_ULTRASONIC_FRONT:
      displayUltrasonicFrontSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleUltrasonicFrontSettings();
        if (LEFT() || RIGHT() || START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_ULTRASONIC_BACK:
      displayUltrasonicBackSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleUltrasonicBackSettings();
        if (LEFT() || RIGHT() || START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_ULTRASONIC_FRONT_TENGAH:
      displayUltrasonicFrontTengahSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleUltrasonicFrontTengahSettings();
        if (LEFT() || RIGHT() || START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_ULTRASONIC_FRONT_SERONG:
      displayUltrasonicFrontSerongSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleUltrasonicFrontSerongSettings();
        if (LEFT() || RIGHT() || START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_ULTRASONIC_BACK_TENGAH:
      displayUltrasonicBackTengahSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleUltrasonicBackTengahSettings();
        if (LEFT() || RIGHT() || START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_ULTRASONIC_BACK_SERONG:
      displayUltrasonicBackSerongSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleUltrasonicBackSerongSettings();
        if (LEFT() || RIGHT() || START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_WIFI_SETTINGS:
      displayWifiSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleWifiSettings();
        if (START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_RFID_UJUNG:
      displayRfidUjung();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleRfidUjung();
        if (START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_RFID_WAREHOUSE:
      displayRfidWarehouse();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleRfidWarehouse();
        if (START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_AUTO_INPUT_STATION:
      displayAutoInputStation();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleAutoInputStation();
        if (START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;
      
    case MENU_TERMINAL_DROP:
      displayTerminalDrop();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleTerminalDrop();
        if (START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;
      
    case MENU_TERMINAL_PICKUP:
      displayTerminalPickup();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleTerminalPickup();
        if (START() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
      break;
      
  }
}
// ====== MOTOR TEST SUBMENU FUNCTIONS ======
void displayMotorTest() {
  static int lastSelectedItem = -1;
  
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

void displayPidSettings() {
  // Clear display if menu needs refresh
  if (menuNeedsRefresh) {
    lcd.clear();
    menuNeedsRefresh = false;
  }
  
  displayMenuHeader("PID Settings");

  // Display Kp
  lcd.setCursor(0, 1);
  if (selectedParam == 0) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kp: ");
  lcd.print(tempKp);
  lcd.print("       ");

  // Display Ki
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Ki: ");
  lcd.print(tempKi);
  lcd.print("       ");

  // Display Kd
  lcd.setCursor(0, 3);
  if (selectedParam == 2) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kd: ");
  lcd.print(tempKd);
  lcd.print("       ");

  // Show controls
  lcd.setCursor(12, 3);
  lcd.print("B:OK");
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
      initMenuTempVariables();  // Initialize temporary variables from global values
      currentMenu = MENU_PID_FORWARD;
      selectedParam = 0; // Reset for PID parameter selection
      menuNeedsRefresh = true;
    } else if (selectedParam == 1) {
      // Backward PID
      lcd.clear();
      initMenuTempVariables();  // Initialize temporary variables from global values
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
      initMenuTempVariables();  // Initialize temporary variables from global values
      currentMenu = MENU_PID_FORWARD_WITHMASSA;
      selectedParam = 0; // Reset for PID parameter selection
      menuNeedsRefresh = true;
    } else if (selectedParam == 1) {
      // Default PID
      lcd.clear();
      initMenuTempVariables();  // Initialize temporary variables from global values
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
      initMenuTempVariables();  // Initialize temporary variables from global values
      currentMenu = MENU_PID_BACKWARD_WITHMASSA;
      selectedParam = 0; // Reset for PID parameter selection
      menuNeedsRefresh = true;
    } else if (selectedParam == 1) {
      // Default PID
      lcd.clear();
      initMenuTempVariables();  // Initialize temporary variables from global values
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
  lcd.print(tempKpForwardWithMassa);
  lcd.print("       ");

  // Display Ki
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Ki: ");
  lcd.print(tempKiForwardWithMassa);
  lcd.print("       ");

  // Display Kd
  lcd.setCursor(0, 3);
  if (selectedParam == 2) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kd: ");
  lcd.print(tempKdForwardWithMassa);
  lcd.print("       ");

  // Show controls
  lcd.setCursor(12, 3);
  lcd.print("B:OK");
}

void handlePidForwardWithMassaSettings() {
  static unsigned long lastRightPress = 0;
  static unsigned long lastLeftPress = 0;
  static unsigned long rightHoldStart = 0;
  static unsigned long leftHoldStart = 0;
  static bool rightHolding = false;
  static bool leftHolding = false;
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
        case 0: tempKpForwardWithMassa += pidIncrement; break;
        case 1: tempKiForwardWithMassa += pidIncrement; break;
        case 2: tempKdForwardWithMassa += pidIncrement; break;
      }
      // Update values in memory
      kpLinefollowerForwardWithMassa = tempKpForwardWithMassa;
      kiLinefollowerForwardWithMassa = tempKiForwardWithMassa;
      kdLinefollowerForwardWithMassa = tempKdForwardWithMassa;
      lastRightPress = currentMillis;
    } else if (currentMillis - rightHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - increment by 1.0 every 100ms
      if (currentMillis - lastRightPress >= 100) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: tempKpForwardWithMassa += pidIncrement; break;
          case 1: tempKiForwardWithMassa += pidIncrement; break;
          case 2: tempKdForwardWithMassa += pidIncrement; break;
        }
        // Update values in memory
        kpLinefollowerForwardWithMassa = tempKpForwardWithMassa;
        kiLinefollowerForwardWithMassa = tempKiForwardWithMassa;
        kdLinefollowerForwardWithMassa = tempKdForwardWithMassa;
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
        case 0: tempKpForwardWithMassa = max(0.0, tempKpForwardWithMassa - pidIncrement); break;
        case 1: tempKiForwardWithMassa = max(0.0, tempKiForwardWithMassa - pidIncrement); break;
        case 2: tempKdForwardWithMassa = max(0.0, tempKdForwardWithMassa - pidIncrement); break;
      }
      // Update values in memory
      kpLinefollowerForwardWithMassa = tempKpForwardWithMassa;
      kiLinefollowerForwardWithMassa = tempKiForwardWithMassa;
      kdLinefollowerForwardWithMassa = tempKdForwardWithMassa;
      lastLeftPress = currentMillis;
    } else if (currentMillis - leftHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - decrement by 1.0 every 100ms
      if (currentMillis - lastLeftPress >= 100) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: tempKpForwardWithMassa = max(0.0, tempKpForwardWithMassa - pidIncrement); break;
          case 1: tempKiForwardWithMassa = max(0.0, tempKiForwardWithMassa - pidIncrement); break;
          case 2: tempKdForwardWithMassa = max(0.0, tempKdForwardWithMassa - pidIncrement); break;
        }
        // Update values in memory
        kpLinefollowerForwardWithMassa = tempKpForwardWithMassa;
        kiLinefollowerForwardWithMassa = tempKiForwardWithMassa;
        kdLinefollowerForwardWithMassa = tempKdForwardWithMassa;
        lastLeftPress = currentMillis;
      }
    }
  } else {
    leftHolding = false;
  }

  if (STOP()) {
    // Save settings before going back
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
  lcd.print(tempKpForwardDefault);
  lcd.print("       ");

  // Display Ki
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Ki: ");
  lcd.print(tempKiForwardDefault);
  lcd.print("       ");

  // Display Kd
  lcd.setCursor(0, 3);
  if (selectedParam == 2) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kd: ");
  lcd.print(tempKdForwardDefault);
  lcd.print("       ");

  // Show controls
  lcd.setCursor(12, 3);
  lcd.print("B:OK");
}

void handlePidForwardDefaultSettings() {
  static unsigned long lastRightPress = 0;
  static unsigned long lastLeftPress = 0;
  static unsigned long rightHoldStart = 0;
  static unsigned long leftHoldStart = 0;
  static bool rightHolding = false;
  static bool leftHolding = false;
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
        case 0: tempKpForwardDefault += pidIncrement; break;
        case 1: tempKiForwardDefault += pidIncrement; break;
        case 2: tempKdForwardDefault += pidIncrement; break;
      }
      // Update values in memory
      kpLinefollowerForwardDefault = tempKpForwardDefault;
      kiLinefollowerForwardDefault = tempKiForwardDefault;
      kdLinefollowerForwardDefault = tempKdForwardDefault;
      lastRightPress = currentMillis;
    } else if (currentMillis - rightHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - increment by 1.0 every 100ms
      if (currentMillis - lastRightPress >= 100) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: tempKpForwardDefault += pidIncrement; break;
          case 1: tempKiForwardDefault += pidIncrement; break;
          case 2: tempKdForwardDefault += pidIncrement; break;
        }
        // Update values in memory
        kpLinefollowerForwardDefault = tempKpForwardDefault;
        kiLinefollowerForwardDefault = tempKiForwardDefault;
        kdLinefollowerForwardDefault = tempKdForwardDefault;
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
        case 0: tempKpForwardDefault = max(0.0, tempKpForwardDefault - pidIncrement); break;
        case 1: tempKiForwardDefault = max(0.0, tempKiForwardDefault - pidIncrement); break;
        case 2: tempKdForwardDefault = max(0.0, tempKdForwardDefault - pidIncrement); break;
      }
      // Update values in memory
      kpLinefollowerForwardDefault = tempKpForwardDefault;
      kiLinefollowerForwardDefault = tempKiForwardDefault;
      kdLinefollowerForwardDefault = tempKdForwardDefault;
      lastLeftPress = currentMillis;
    } else if (currentMillis - leftHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - decrement by 1.0 every 100ms
      if (currentMillis - lastLeftPress >= 100) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: tempKpForwardDefault = max(0.0, tempKpForwardDefault - pidIncrement); break;
          case 1: tempKiForwardDefault = max(0.0, tempKiForwardDefault - pidIncrement); break;
          case 2: tempKdForwardDefault = max(0.0, tempKdForwardDefault - pidIncrement); break;
        }
        // Update values in memory
        kpLinefollowerForwardDefault = tempKpForwardDefault;
        kiLinefollowerForwardDefault = tempKiForwardDefault;
        kdLinefollowerForwardDefault = tempKdForwardDefault;
        lastLeftPress = currentMillis;
      }
    }
  } else {
    leftHolding = false;
  }

  if (STOP()) {
    // Save settings before going back
    saveSettings();
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
  lcd.print(tempKpBackwardWithMassa);
  lcd.print("       ");

  // Display Ki
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Ki: ");
  lcd.print(tempKiBackwardWithMassa);
  lcd.print("       ");

  // Display Kd
  lcd.setCursor(0, 3);
  if (selectedParam == 2) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kd: ");
  lcd.print(tempKdBackwardWithMassa);
  lcd.print("       ");

  // Show controls
  lcd.setCursor(12, 3);
  lcd.print("B:OK");
}

void handlePidBackwardWithMassaSettings() {
  static unsigned long lastRightPress = 0;
  static unsigned long lastLeftPress = 0;
  static unsigned long rightHoldStart = 0;
  static unsigned long leftHoldStart = 0;
  static bool rightHolding = false;
  static bool leftHolding = false;
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
        case 0: tempKpBackwardWithMassa += pidIncrement; break;
        case 1: tempKiBackwardWithMassa += pidIncrement; break;
        case 2: tempKdBackwardWithMassa += pidIncrement; break;
      }
      // Update values in memory
      kpLinefollowerBackwardWithMassa = tempKpBackwardWithMassa;
      kiLinefollowerBackwardWithMassa = tempKiBackwardWithMassa;
      kdLinefollowerBackwardWithMassa = tempKdBackwardWithMassa;
      lastRightPress = currentMillis;
    } else if (currentMillis - rightHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - increment by 1.0 every 100ms
      if (currentMillis - lastRightPress >= 100) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: tempKpBackwardWithMassa += pidIncrement; break;
          case 1: tempKiBackwardWithMassa += pidIncrement; break;
          case 2: tempKdBackwardWithMassa += pidIncrement; break;
        }
        // Update values in memory
        kpLinefollowerBackwardWithMassa = tempKpBackwardWithMassa;
        kiLinefollowerBackwardWithMassa = tempKiBackwardWithMassa;
        kdLinefollowerBackwardWithMassa = tempKdBackwardWithMassa;
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
        case 0: tempKpBackwardWithMassa = max(0.0, tempKpBackwardWithMassa - pidIncrement); break;
        case 1: tempKiBackwardWithMassa = max(0.0, tempKiBackwardWithMassa - pidIncrement); break;
        case 2: tempKdBackwardWithMassa = max(0.0, tempKdBackwardWithMassa - pidIncrement); break;
      }
      // Update values in memory
      kpLinefollowerBackwardWithMassa = tempKpBackwardWithMassa;
      kiLinefollowerBackwardWithMassa = tempKiBackwardWithMassa;
      kdLinefollowerBackwardWithMassa = tempKdBackwardWithMassa;
      lastLeftPress = currentMillis;
    } else if (currentMillis - leftHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - decrement by 1.0 every 100ms
      if (currentMillis - lastLeftPress >= 100) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: tempKpBackwardWithMassa = max(0.0, tempKpBackwardWithMassa - pidIncrement); break;
          case 1: tempKiBackwardWithMassa = max(0.0, tempKiBackwardWithMassa - pidIncrement); break;
          case 2: tempKdBackwardWithMassa = max(0.0, tempKdBackwardWithMassa - pidIncrement); break;
        }
        // Update values in memory
        kpLinefollowerBackwardWithMassa = tempKpBackwardWithMassa;
        kiLinefollowerBackwardWithMassa = tempKiBackwardWithMassa;
        kdLinefollowerBackwardWithMassa = tempKdBackwardWithMassa;
        lastLeftPress = currentMillis;
      }
    }
  } else {
    leftHolding = false;
  }

  if (STOP()) {
    // Save settings before going back
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
  lcd.print(tempKpBackwardDefault);
  lcd.print("       ");

  // Display Ki
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Ki: ");
  lcd.print(tempKiBackwardDefault);
  lcd.print("       ");

  // Display Kd
  lcd.setCursor(0, 3);
  if (selectedParam == 2) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kd: ");
  lcd.print(tempKdBackwardDefault);
  lcd.print("       ");

  // Show controls
  lcd.setCursor(12, 3);
  lcd.print("B:OK");
}

void handlePidBackwardDefaultSettings() {
  static unsigned long lastRightPress = 0;
  static unsigned long lastLeftPress = 0;
  static unsigned long rightHoldStart = 0;
  static unsigned long leftHoldStart = 0;
  static bool rightHolding = false;
  static bool leftHolding = false;
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
        case 0: tempKpBackwardDefault += pidIncrement; break;
        case 1: tempKiBackwardDefault += pidIncrement; break;
        case 2: tempKdBackwardDefault += pidIncrement; break;
      }
      // Update values in memory
      kpLinefollowerBackwardDefault = tempKpBackwardDefault;
      kiLinefollowerBackwardDefault = tempKiBackwardDefault;
      kdLinefollowerBackwardDefault = tempKdBackwardDefault;
      lastRightPress = currentMillis;
    } else if (currentMillis - rightHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - increment by 1.0 every 100ms
      if (currentMillis - lastRightPress >= 100) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: tempKpBackwardDefault += pidIncrement; break;
          case 1: tempKiBackwardDefault += pidIncrement; break;
          case 2: tempKdBackwardDefault += pidIncrement; break;
        }
        // Update values in memory
        kpLinefollowerBackwardDefault = tempKpBackwardDefault;
        kiLinefollowerBackwardDefault = tempKiBackwardDefault;
        kdLinefollowerBackwardDefault = tempKdBackwardDefault;
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
        case 0: tempKpBackwardDefault = max(0.0, tempKpBackwardDefault - pidIncrement); break;
        case 1: tempKiBackwardDefault = max(0.0, tempKiBackwardDefault - pidIncrement); break;
        case 2: tempKdBackwardDefault = max(0.0, tempKdBackwardDefault - pidIncrement); break;
      }
      // Update values in memory
      kpLinefollowerBackwardDefault = tempKpBackwardDefault;
      kiLinefollowerBackwardDefault = tempKiBackwardDefault;
      kdLinefollowerBackwardDefault = tempKdBackwardDefault;
      lastLeftPress = currentMillis;
    } else if (currentMillis - leftHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - decrement by 1.0 every 100ms
      if (currentMillis - lastLeftPress >= 100) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: tempKpBackwardDefault = max(0.0, tempKpBackwardDefault - pidIncrement); break;
          case 1: tempKiBackwardDefault = max(0.0, tempKiBackwardDefault - pidIncrement); break;
          case 2: tempKdBackwardDefault = max(0.0, tempKdBackwardDefault - pidIncrement); break;
        }
        // Update values in memory
        kpLinefollowerBackwardDefault = tempKpBackwardDefault;
        kiLinefollowerBackwardDefault = tempKiBackwardDefault;
        kdLinefollowerBackwardDefault = tempKdBackwardDefault;
        lastLeftPress = currentMillis;
      }
    }
  } else {
    leftHolding = false;
  }

  if (STOP()) {
    // Save settings before going back
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
  static int lastRemainingTime = -1; // Declare as static to retain value across calls
  static int lastSelectedRfidItem = -1;
  static int lastSelectedStationId = -1;
  static bool lastMenuDrawn = false;
  
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
  static unsigned long lastRightPress = 0;
  static unsigned long lastLeftPress = 0;
  static unsigned long rightHoldStart = 0;
  static unsigned long leftHoldStart = 0;
  static bool rightHolding = false;
  static bool leftHolding = false;
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
        case 0: tempKp += pidIncrement; break;
        case 1: tempKi += pidIncrement; break;
        case 2: tempKd += pidIncrement; break;
      }
      lastRightPress = currentMillis;
    } else if (currentMillis - rightHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - increment by 1.0 every 100ms
      if (currentMillis - lastRightPress >= 100) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: tempKp += pidIncrement; break;
          case 1: tempKi += pidIncrement; break;
          case 2: tempKd += pidIncrement; break;
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
        case 0: tempKp = max(0.0f, (float)(tempKp - pidIncrement)); break;
        case 1: tempKi = max(0.0f, (float)(tempKi - pidIncrement)); break;
        case 2: tempKd = max(0.0f, (float)(tempKd - pidIncrement)); break;
      }
      lastLeftPress = currentMillis;
    } else if (currentMillis - leftHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - decrement by 1.0 every 100ms
      if (currentMillis - lastLeftPress >= 100) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: tempKp = max(0.0f, (float)(tempKp - pidIncrement)); break;
          case 1: tempKi = max(0.0f, (float)(tempKi - pidIncrement)); break;
          case 2: tempKd = max(0.0f, (float)(tempKd - pidIncrement)); break;
        }
        lastLeftPress = currentMillis;
      }
    }
  } else {
    leftHolding = false;
  }

  if (STOP()) {
    kpLinefollower = tempKp;
    kiLinefollower = tempKi;
    kdLinefollower = tempKd;
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

void handleRfidSettings() {
  unsigned long currentMillis = millis();

  if (isWaitingForRfid) {
    // Check if new RFID was scanned
    if (newRfidScanned) {
      // RFID card detected, save it
      if (addRfidStation(selectedStationId, String(lastScannedRfidOptimized))) {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Station ");
        lcd.print(selectedStationId);
        lcd.print(" saved!");
        lcd.setCursor(0, 2);
        lcd.print("RFID: ");
        // Display first 8 characters of RFID (optimized)
        char rfidDisplay[9];  // 8 chars + null terminator
        strncpy(rfidDisplay, lastScannedRfidOptimized, 8);
        rfidDisplay[8] = '\0';
        lcd.print(rfidDisplay);
        lcd.print("...");
        delay(2000);
      } else {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Error saving!");
        delay(2000);
      }

      // Reset scanning state
      isWaitingForRfid = false;
      newRfidScanned = false;
      selectedRfidItem = 1;  // Stay on scan option for next scan
    }

    // Check for timeout or cancel
    if ((currentMillis - rfidScanTimeout > RFID_SCAN_TIMEOUT) || STOP()) {
      isWaitingForRfid = false;
      newRfidScanned = false;
    }

    return;
  }

  if (UP()) {
    selectedRfidItem = (selectedRfidItem - 1 + 10) % 10;
  } else if (DOWN()) {
    selectedRfidItem = (selectedRfidItem + 1) % 10;
  } else if (RIGHT()) {
    if (selectedRfidItem == 0) {
      // Change station ID
      selectedStationId = (selectedStationId % MAX_RFID_STATIONS) + 1;
      menuNeedsRefresh = true;  // Refresh tampilan setelah perubahan
    }
  } else if (LEFT()) {
    if (selectedRfidItem == 0) {
      // Change station ID
      selectedStationId = selectedStationId == 1 ? MAX_RFID_STATIONS : selectedStationId - 1;
      menuNeedsRefresh = true;  // Refresh tampilan setelah perubahan
    }
  } else if (START()) {
    switch (selectedRfidItem) {
      case 1:  // Scan RFID
        isWaitingForRfid = true;
        rfidScanTimeout = currentMillis;
        newRfidScanned = false;  // Reset flag
        break;

      case 2:  // Auto Input Station
        currentMenu = MENU_AUTO_INPUT_STATION;
        menuNeedsRefresh = true;
        break;

      case 3:  // View All
        {
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

          // Wait for any button press
          while (true) {
            if (START() || STOP()) {
              delay(200);
              break;
            }
            delay(1);
          }
          menuNeedsRefresh = true;  // Refresh menu after exiting view all
        }
        break;

      case 4:  // Delete Station
        {
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
        break;

      case 5:  // Clear All
        {
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
            delay(50);
          }
        }
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
    // Begin preferences session
    preferences.begin("agv-settings", false);

    // Clear all preferences
    preferences.clear();

    // Reset PID values to defaults
    tempKp = 70.0;
    tempKi = 0.0;
    tempKd = 0.0;
    
    // Reset Forward PID WithMassa values to defaults
    tempKpForwardWithMassa = 70.0;
    tempKiForwardWithMassa = 0.0;
    tempKdForwardWithMassa = 0.0;
    
    // Reset Forward PID Default values to defaults
    tempKpForwardDefault = 70.0;
    tempKiForwardDefault = 0.0;
    tempKdForwardDefault = 0.0;
    
    // Reset Backward PID WithMassa values to defaults
    tempKpBackwardWithMassa = 70.0;
    tempKiBackwardWithMassa = 0.0;
    tempKdBackwardWithMassa = 0.0;
    
    // Reset Backward PID Default values to defaults
    tempKpBackwardDefault = 70.0;
    tempKiBackwardDefault = 0.0;
    tempKdBackwardDefault = 0.0;
    
    // Debug: Print reset values
    // Serial.println() - removed for production
    // Serial.println() - removed for production
    // Serial.println() - removed for production
    // Serial.println() - removed for production
    // Serial.println() - removed for production

    // Reset Motor values to defaults
    tempBaseSpeed = 1000;

    // Reset Motor invert values to defaults
    tempInvertY = false;
    tempInvertX = false;
    tempInvertKanan = false;
    tempInvertKiri = false;
    tempInvertHook = false;

    // Reset Music mapping values to defaults
    tempMusicOnPin = 0;        // pinMusic1
    tempMusicObstaclePin = 1;  // pinMusic2
    tempMusicStationPin = 2;   // pinMusic3
    tempMusicOutOfLinePin = 3; // pinMusic4
    
    // Reset Ultrasonic settings to defaults
    tempMinSafeDistanceFront = 30;
    tempMinSafeDistanceBack = 20;  // Default safe distance

    // Save default values
    saveSettings();

    // Clear RFID stations too
    clearAllRfidStations();

    // Clear stations list from HTTP preferences
    stationsPreferences.begin(STATIONS_NAMESPACE, false);
    stationsPreferences.clear();
    stationsPreferences.end();

    // Clear targetStationsList in memory
    targetStationsList.clear();

    // End preferences session
    preferences.end();
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
    moveStateAgv = AGV_STATE_MOVE_FORWARD;
    savemoveStateAGVToPreferences(moveStateAgv);
    saveCurrentStateAGVToPreferences(currentStateAgv);
    // Show confirmation message
    lcd.clear();
    displayMenuHeader("AGV State Reset");
    lcd.setCursor(0, 1);
    lcd.print("AGV state has been");
    lcd.setCursor(0, 2);
    lcd.print("reset to STOP");
    delay(2000); // Show message for 2 seconds
    
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
  static bool displayInitialized = false;
  static int lastSelectedMusicItem = -1;
  static int musicTestMenuStartIndex = 0;
  static int lastMusicTestMenuStartIndex = -1;
  
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
  static bool displayInitialized = false;
  
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
  static unsigned long lastDisplayUpdate = 0;
  const unsigned long DISPLAY_UPDATE_INTERVAL = 200; // 200ms interval for display update
  
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
    startTimer(&magnetSwitchTimer, 100);  // Non-blocking delay to prevent multiple triggers
  } else if (RIGHT()) {
    // Switch to back magnet sensor
    switchMagnetSensor(false);
    startTimer(&magnetSwitchTimer, 100);  // Non-blocking delay to prevent multiple triggers
  } else if (STOP()) {
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
  // Display updates automatically since displayMagnetCheck reads current sensor values
}

void handleUltrasonicCheck() {
  static unsigned long lastSwitchTime = 0;
  static unsigned long lastSensorRead = 0;
  const unsigned long SWITCH_DEBOUNCE = 200; // 200ms debounce for switching
  const unsigned long SENSOR_READ_INTERVAL = 100; // 100ms interval for sensor reading
  
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
  
  lcd.setCursor(0, 2);
  lcd.print("A:Scan B:View C:Del ");
  
  lcd.setCursor(0, 3);
  lcd.print("STOP:Back           ");
}

void handleRfidUjung() {
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
        displayRfidUjung();
        return;
      }
      
      if (newRfidScanned) {
        String rfidData = String(lastScannedRfidOptimized);
        newRfidScanned = false;

        
        // Hanya simpan 1 data - ganti data lama jika ada
        rfidUjungList[0].ujungId = 1;
        rfidUjungList[0].rfidId = rfidData;
        rfidUjungList[0].isActive = true;
        rfidUjungCount = 1;  // Selalu 1 data saja
        saveRfidUjungToPreferences();
        
        // Sinkronisasi dengan ujungRfidId untuk logika AGV
        saveUjungRfid(rfidData);
        
        lcd.setCursor(0, 1);
        lcd.print("RFID Saved!         ");
        lcd.setCursor(0, 2);
        lcd.print(rfidData.substring(0, 16));
        lcd.print("    ");
        delay(2000);
        
        displayRfidUjung();
        return;
      }
      delay(100);
    }
    
    lcd.setCursor(0, 1);
    lcd.print("Scan Timeout!       ");
    delay(2000);
    displayRfidUjung();
    
  } else if (LEFT()) { // View data
    if (rfidUjungCount == 0) {
      lcd.setCursor(0, 1);
      lcd.print("No Data Available   ");
      delay(2000);
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
      lcd.print(rfidUjungList[viewIndex].rfidId.substring(0, 16));
      lcd.print("    ");
      
      lcd.setCursor(0, 3);
      lcd.print("^v:Nav STOP:Back    ");
      
      if (UP() && viewIndex > 0) {
        viewIndex--;
        delay(200);
      } else if (DOWN() && viewIndex < rfidUjungCount - 1) {
        viewIndex++;
        delay(200);
      } else if (STOP()) {
        displayRfidUjung();
        return;
      }
      delay(50);
    }
    
  } else if (RIGHT()) { // Delete all
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
        delay(2000);
        displayRfidUjung();
        return;
      } else if (LEFT() || STOP()) {
        displayRfidUjung();
        return;
      }
      delay(50);
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
        delay(2000);
        
        displayRfidWarehouse();
        return;
      }
      delay(100);
    }
    
    lcd.setCursor(0, 1);
    lcd.print("Scan Timeout!       ");
    delay(2000);
    displayRfidWarehouse();
    
  } else if (LEFT()) { // View data
    if (rfidWarehouseCount == 0) {
      lcd.setCursor(0, 1);
      lcd.print("No Data Available   ");
      delay(2000);
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
        delay(200);
      } else if (DOWN() && viewIndex < rfidWarehouseCount - 1) {
        viewIndex++;
        delay(200);
      } else if (STOP()) {
        displayRfidWarehouse();
        return;
      }
      delay(50);
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
        delay(2000);
        displayRfidWarehouse();
        return;
      } else if (LEFT() || STOP()) {
        displayRfidWarehouse();
        return;
      }
      delay(50);
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
          delay(2000);
        } else if (findRfidStationByRfidId(rfidData) != -1) {
          lcd.setCursor(0, 1);
          lcd.print("Station Exists!     ");
          delay(2000);
        } else {
          lcd.setCursor(0, 1);
          lcd.print("Storage Full!       ");
          delay(2000);
        }
        
        displayAutoInputStation();
        return;
      }
      delay(100);
    }
    
    lcd.setCursor(0, 1);
    lcd.print("Scan Timeout!       ");
    delay(2000);
    displayAutoInputStation();
    
  } else if (LEFT()) { // View data
    if (rfidStationCount == 0) {
      lcd.setCursor(0, 1);
      lcd.print("No Data Available   ");
      delay(2000);
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
        delay(200);
      } else if (DOWN() && viewIndex < rfidStationCount - 1) {
        viewIndex++;
        delay(200);
      } else if (STOP()) {
        displayAutoInputStation();
        return;
      }
      delay(50);
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
        delay(2000);
        displayAutoInputStation();
        return;
      } else if (LEFT() || STOP()) {
        displayAutoInputStation();
        return;
      }
      delay(50);
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
        delay(2000);
        
        newRfidScanned = false;
        displayTerminalDrop();
        return;
      } else if (LEFT() || STOP()) {
        displayTerminalDrop();
        return;
      }
      delay(50);
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
        delay(2000);
        
        newRfidScanned = false;
        displayTerminalPickup();
        return;
      } else if (LEFT() || STOP()) {
        displayTerminalPickup();
        return;
      }
      delay(100);
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
  static bool displayInitialized = false;
  static int lastSelectedMusicPin = -1;
  
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
      musicOnPin = tempMusicOnPin;
      musicObstaclePin = tempMusicObstaclePin;
      musicStationPin = tempMusicStationPin;
      musicOutOfLinePin = tempMusicOutOfLinePin;
      musicWarningPin = tempMusicWarningPin;
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
  static bool displayInitialized = false;
  static int lastSelectedMusicItem = -1;
  static int musicMenuStartIndex = 0;
  static int lastMusicMenuStartIndex = -1;
  
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
  static bool displayInitialized = false;
  
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
        selectedMusicPin = tempMusicOnPin;
        currentMenu = MENU_MUSIC_ON; 
        break;
      case 1: 
        selectedMusicPin = tempMusicObstaclePin;
        currentMenu = MENU_MUSIC_OBSTACLE; 
        break;
      case 2: 
        selectedMusicPin = tempMusicStationPin;
        currentMenu = MENU_MUSIC_STATION; 
        break;
      case 3: 
        selectedMusicPin = tempMusicOutOfLinePin;
        currentMenu = MENU_MUSIC_OUTOFLINE; 
        break;
      case 4: 
        selectedMusicPin = tempMusicWarningPin;
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
  static int lastTempMaxMotorRpm = -1;
  
  if (menuNeedsRefresh || tempMaxMotorRpm != lastTempMaxMotorRpm) {
    lcd.clear();
    displayMenuHeader("Speed Setting:");
    
    lcd.setCursor(0, 1);
    lcd.print("Max RPM:");
    
    lcd.setCursor(0, 2);
    lcd.print("> ");
    lcd.print(tempMaxMotorRpm);
    lcd.print(" RPM");
    
    // Show range
    lcd.setCursor(0, 3);
    lcd.print("Range: 10-90 RPM");
    
    menuNeedsRefresh = false;
    lastTempMaxMotorRpm = tempMaxMotorRpm;
  }
}

void handleSpeedSetting() {
  if (LEFT()) {
    tempMaxMotorRpm = max(10, tempMaxMotorRpm - 5);
    menuNeedsRefresh = true;
  } else if (RIGHT()) {
    tempMaxMotorRpm = min(90, tempMaxMotorRpm + 5);
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Save speed setting and send to motor controller
    maxMotorRpm = tempMaxMotorRpm;
    
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
  static int lastSelectedParam = -1;
  static double lastValues[3] = {-1, -1, -1};
  
  if (menuNeedsRefresh || selectedItem != lastSelectedParam ||
      tempMotorPidKp != lastValues[0] || tempMotorPidKi != lastValues[1] || 
      tempMotorPidKd != lastValues[2]) {
    
    lcd.clear();
    displayMenuHeader("PID Setting:");
    
    String params[3] = {"Kp:", "Ki:", "Kd:"};
    double values[3] = {tempMotorPidKp, tempMotorPidKi, tempMotorPidKd};
    
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
    lastValues[0] = tempMotorPidKp;
    lastValues[1] = tempMotorPidKi;
    lastValues[2] = tempMotorPidKd;
  }
}

void handlePidRpmSetting() {
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
        tempMotorPidKp = max(0.0, tempMotorPidKp - 0.1);
        break;
      case 1:  // Ki
        tempMotorPidKi = max(0.0, tempMotorPidKi - 0.01);
        break;
      case 2:  // Kd
        tempMotorPidKd = max(0.0, tempMotorPidKd - 0.01);
        break;
    }
    menuNeedsRefresh = true;
  } else if (RIGHT()) {
    // Increase selected parameter
    switch (selectedItem) {
      case 0:  // Kp
        tempMotorPidKp = min(100.0, tempMotorPidKp + 0.1);
        break;
      case 1:  // Ki
        tempMotorPidKi = min(50.0, tempMotorPidKi + 0.01);
        break;
      case 2:  // Kd
        tempMotorPidKd = min(50.0, tempMotorPidKd + 0.01);
        break;
    }
    menuNeedsRefresh = true;
  } else if (START() || STOP() ) {
    // Save PID settings and send to motor controller
    motorPidKp = tempMotorPidKp;
    motorPidKi = tempMotorPidKi;
    motorPidKd = tempMotorPidKd;

    // Send PID values using the dedicated function
    sendPidValues(motorPidKp, motorPidKi, motorPidKd);
    
    // Show confirmation
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PID Command Sent!");
    lcd.setCursor(0, 1);
    lcd.print("Kp:" + String(motorPidKp, 2));
    lcd.setCursor(0, 2);
    lcd.print("Ki:" + String(motorPidKi, 3));
    lcd.setCursor(0, 3);
    lcd.print("Kd:" + String(motorPidKd, 3));
    delay(1000);
    // menuNeedsRefresh = true;
    
    // Back to Motor Settings menu
    lcd.clear();
    currentMenu = MENU_MOTOR_SETTINGS;
    selectedItem = 1;
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
  lcd.print(String(tempMinSafeDistanceFront) + " cm");
  
  lcd.setCursor(0, 3);
  lcd.print("L/R:Adj A:Save B:Back");
}

void handleUltrasonicFrontTengahSettings() {
  if (LEFT()) {
    // Decrease by 1 cm
    if (tempMinSafeDistanceFront > 5) {  // Minimum 5 cm
      tempMinSafeDistanceFront--;
    }
  } else if (RIGHT()) {
    // Increase by 1 cm
    if (tempMinSafeDistanceFront < 100) {  // Maximum 100 cm
      tempMinSafeDistanceFront++;
    }
  } else if (START()) {
    // Save settings
    minSafeDistanceFront = tempMinSafeDistanceFront;
    
    // Save to preferences
    preferences.begin("agv-settings", false);
    preferences.putUShort("SafeDistFront", tempMinSafeDistanceFront);
    preferences.end();
    
    // Show confirmation
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Front Tengah saved!");
    lcd.setCursor(0, 2);
    lcd.print("Min distance: " + String(tempMinSafeDistanceFront) + "cm");
    delay(2000);
    
    currentMenu = MENU_ULTRASONIC_FRONT;
    selectedItem = 0;
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Cancel changes
    tempMinSafeDistanceFront = minSafeDistanceFront;
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
  lcd.print(String(tempMinSafeDistanceFrontSerong) + " cm");
  
  lcd.setCursor(0, 3);
  lcd.print("L/R:Adj A:Save B:Back");
}

void handleUltrasonicFrontSerongSettings() {
  if (LEFT()) {
    // Decrease by 1 cm
    if (tempMinSafeDistanceFrontSerong > 5) {  // Minimum 5 cm
      tempMinSafeDistanceFrontSerong--;
    }
  } else if (RIGHT()) {
    // Increase by 1 cm
    if (tempMinSafeDistanceFrontSerong < 100) {  // Maximum 100 cm
      tempMinSafeDistanceFrontSerong++;
    }
  } else if (START()) {
    // Save settings
    minSafeDistanceFrontSerong = tempMinSafeDistanceFrontSerong;
    
    // Save to preferences
    preferences.begin("agv-settings", false);
    preferences.putUShort("SafeDistFrontS", tempMinSafeDistanceFrontSerong);
    preferences.end();
    
    // Show confirmation
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Front Serong saved!");
    lcd.setCursor(0, 2);
    lcd.print("Min distance: " + String(tempMinSafeDistanceFrontSerong) + "cm");
    delay(2000);
    
    currentMenu = MENU_ULTRASONIC_FRONT;
    selectedItem = 1;
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Cancel changes
    tempMinSafeDistanceFrontSerong = minSafeDistanceFrontSerong;
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
  lcd.print(String(tempMinSafeDistanceBack) + " cm");
  
  lcd.setCursor(0, 3);
  lcd.print("L/R:Adj A:Save B:Back");
}

void handleUltrasonicBackTengahSettings() {
  if (LEFT()) {
    // Decrease by 1 cm
    if (tempMinSafeDistanceBack > 5) {  // Minimum 5 cm
      tempMinSafeDistanceBack--;
    }
  } else if (RIGHT()) {
    // Increase by 1 cm
    if (tempMinSafeDistanceBack < 100) {  // Maximum 100 cm
      tempMinSafeDistanceBack++;
    }
  } else if (START()) {
    // Save settings
    minSafeDistanceBack = tempMinSafeDistanceBack;
    
    // Save to preferences
    preferences.begin("agv-settings", false);
    preferences.putUShort("SafeDistBack", tempMinSafeDistanceBack);
    preferences.end();
    
    // Show confirmation
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Back Tengah saved!");
    lcd.setCursor(0, 2);
    lcd.print("Min distance: " + String(tempMinSafeDistanceBack) + "cm");
    delay(2000);
    
    currentMenu = MENU_ULTRASONIC_BACK;
    selectedItem = 0;
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Cancel changes
    tempMinSafeDistanceBack = minSafeDistanceBack;
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
  lcd.print(String(tempMinSafeDistanceBackSerong) + " cm");
  
  lcd.setCursor(0, 3);
  lcd.print("L/R:Adj A:Save B:Back");
}

void handleUltrasonicBackSerongSettings() {
  if (LEFT()) {
    // Decrease by 1 cm
    if (tempMinSafeDistanceBackSerong > 5) {  // Minimum 5 cm
      tempMinSafeDistanceBackSerong--;
    }
  } else if (RIGHT()) {
    // Increase by 1 cm
    if (tempMinSafeDistanceBackSerong < 100) {  // Maximum 100 cm
      tempMinSafeDistanceBackSerong++;
    }
  } else if (START()) {
    // Save settings
    minSafeDistanceBackSerong = tempMinSafeDistanceBackSerong;
    
    // Save to preferences
    preferences.begin("agv-settings", false);
    preferences.putUShort("SafeDistBackS", tempMinSafeDistanceBackSerong);
    preferences.end();
    
    // Show confirmation
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Back Serong saved!");
    lcd.setCursor(0, 2);
    lcd.print("Min distance: " + String(tempMinSafeDistanceBackSerong) + "cm");
    delay(2000);
    
    currentMenu = MENU_ULTRASONIC_BACK;
    selectedItem = 1;
    menuNeedsRefresh = true;
  } else if (STOP()) {
    // Cancel changes
    tempMinSafeDistanceBackSerong = minSafeDistanceBackSerong;
    currentMenu = MENU_ULTRASONIC_BACK;
    selectedItem = 1;
    menuNeedsRefresh = true;
  }
}
