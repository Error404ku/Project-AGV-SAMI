#include "menu.h"

// Menu states - Synchronized with menu.h
#define MENU_MAIN 0
#define MENU_AGV_MODE 1
#define MENU_MOTOR_TEST 2
#define MENU_PID_SETTINGS 3
#define MENU_TARGET_SETTINGS 4
#define MENU_RESET 5
#define MENU_RFID_SETTINGS 6
#define MENU_WIFI_SETTINGS 14
#define MENU_MOTOR_SETTINGS 18
#define MENU_MOTOR_INVERT 19
#define MENU_MUSIC_SETTINGS 20
#define MENU_MUSIC_TEST 21
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
  tempKpForward = kpLinefollowerForward;
  tempKiForward = kiLinefollowerForward;
  tempKdForward = kdLinefollowerForward;
  tempKpBackward = kpLinefollowerBackward;
  tempKiBackward = kiLinefollowerBackward;
  tempKdBackward = kdLinefollowerBackward;
  tempBaseSpeed = baseSpeed;
  tempInvertY = invertMotorY;
  tempInvertX = invertMotorX;
  tempInvertKanan = invertMotorKanan;
  tempInvertKiri = invertMotorKiri;
  tempInvertHook = invertHook;
  tempMusicStationPin = musicStationPin;
  tempMusicErrorPin = musicErrorPin;
  tempMusicDetectPin = musicDetectPin;
  tempMusicKomputerPin = musicKomputerPin;
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
  
  // Save Forward PID values
  preferences.putDouble("kpLinefollowerForward", tempKpForward);
  preferences.putDouble("kiLinefollowerForward", tempKiForward);
  preferences.putDouble("kdLinefollowerForward", tempKdForward);
  
  // Save Backward PID values
  preferences.putDouble("kpLinefollowerBackward", tempKpBackward);
  preferences.putDouble("kiLinefollowerBackward", tempKiBackward);
  preferences.putDouble("kdLinefollowerBackward", tempKdBackward);

  // Save Motor values
  preferences.putInt("baseSpeed", tempBaseSpeed);

  // Save Motor invert values
  preferences.putBool("invertY", tempInvertY);
  preferences.putBool("invertX", tempInvertX);
  preferences.putBool("invertKanan", tempInvertKanan);
  preferences.putBool("invertKiri", tempInvertKiri);
  preferences.putBool("invertHook", tempInvertHook);

  // Save Music mapping values
  preferences.putInt("musicStation", tempMusicStationPin);
  preferences.putInt("musicError", tempMusicErrorPin);
  preferences.putInt("musicDetect", tempMusicDetectPin);
  preferences.putInt("musicKomputer", tempMusicKomputerPin);

  // Apply PID values
  kpLinefollower = tempKp;
  kiLinefollower = tempKi;
  kdLinefollower = tempKd;
  
  // Apply Forward PID values
  kpLinefollowerForward = tempKpForward;
  kiLinefollowerForward = tempKiForward;
  kdLinefollowerForward = tempKdForward;
  
  // Apply Backward PID values
  kpLinefollowerBackward = tempKpBackward;
  kiLinefollowerBackward = tempKiBackward;
  kdLinefollowerBackward = tempKdBackward;

  // Debug: Print saved values in saveSettings
  Serial.println("=== All PID Values Saved in saveSettings() ===");
  Serial.println("Forward PID - Kp: " + String(tempKpForward) + ", Ki: " + String(tempKiForward) + ", Kd: " + String(tempKdForward));
  Serial.println("Backward PID - Kp: " + String(tempKpBackward) + ", Ki: " + String(tempKiBackward) + ", Kd: " + String(tempKdBackward));

  // Apply Motor values
  baseSpeed = tempBaseSpeed;

  // Apply Motor invert values
  invertMotorY = tempInvertY;
  invertMotorX = tempInvertX;
  invertMotorKanan = tempInvertKanan;
  invertMotorKiri = tempInvertKiri;
  invertHook = tempInvertHook;

  // Apply Music mapping values
  musicStationPin = tempMusicStationPin;
  musicErrorPin = tempMusicErrorPin;
  musicDetectPin = tempMusicDetectPin;
  musicKomputerPin = tempMusicKomputerPin;

  // End preferences session
  preferences.end();
}

void saveForwardPidSettings() {
  preferences.begin("agv-settings", false);
  delay(50);
  // Save Forward PID values
  preferences.putDouble("kpLinefollowerForward", tempKpForward);
  preferences.putDouble("kiLinefollowerForward", tempKiForward);
  preferences.putDouble("kdLinefollowerForward", tempKdForward);

  // Apply Forward PID values
  kpLinefollowerForward = tempKpForward;
  kiLinefollowerForward = tempKiForward;
  kdLinefollowerForward = tempKdForward;

  // Debug: Print saved values
  Serial.println("=== Forward PID Saved to Preferences ===");
  Serial.println("Saved Forward PID - Kp: " + String(tempKpForward) + ", Ki: " + String(tempKiForward) + ", Kd: " + String(tempKdForward));

  // End preferences session
  preferences.end();
}

void saveBackwardPidSettings() {
  preferences.begin("agv-settings", false);
  delay(50);
  // Save Backward PID values
  preferences.putDouble("kpLinefollowerBackward", tempKpBackward);
  preferences.putDouble("kiLinefollowerBackward", tempKiBackward);
  preferences.putDouble("kdLinefollowerBackward", tempKdBackward);

  // Apply Backward PID values
  kpLinefollowerBackward = tempKpBackward;
  kiLinefollowerBackward = tempKiBackward;
  kdLinefollowerBackward = tempKdBackward;

  // Debug: Print saved values
  Serial.println("=== Backward PID Saved to Preferences ===");
  Serial.println("Saved Backward PID - Kp: " + String(tempKpBackward) + ", Ki: " + String(tempKiBackward) + ", Kd: " + String(tempKdBackward));

  // End preferences session
  preferences.end();
}

void displayMainMenu() {
  // Menu items array
  String menuItems[15] = {
    "AGV Mode",           // selectedItem 0 -> MENU_AGV_MODE (1)
    "Reset AGV State",    // selectedItem 1 -> MENU_RESET_AGV_STATE (25)
    "Motor Test",         // selectedItem 2 -> MENU_MOTOR_TEST (2)
    "PID Settings",       // selectedItem 3 -> MENU_PID_SETTINGS (3)
    "Target Settings",    // selectedItem 4 -> MENU_TARGET_SETTINGS (4)
    "Reset Settings",     // selectedItem 5 -> MENU_RESET (5)
    "RFID Settings",      // selectedItem 6 -> MENU_RFID_SETTINGS (6)
    "Motor Settings",     // selectedItem 7 -> MENU_MOTOR_SETTINGS (18)
    "Motor Invert",       // selectedItem 8 -> MENU_MOTOR_INVERT (19)
    "Music Settings",     // selectedItem 9 -> MENU_MUSIC_SETTINGS (20)
    "Music Test",         // selectedItem 10 -> MENU_MUSIC_TEST (21)
    "Hook Test",          // selectedItem 11 -> MENU_HOOK_TEST (22)
    "Magnet Check",       // selectedItem 12 -> MENU_MAGNET_CHECK (23)
    "Ultrasonic Check",   // selectedItem 13 -> MENU_ULTRASONIC_CHECK (24)
    "WiFi Settings"       // selectedItem 14 -> MENU_WIFI_SETTINGS (14)
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
      newRfidScanned = false; // Reset flag RFID saat keluar dari AGV mode
      menuNeedsRefresh = true;
    }
    return;
  }

  switch (currentMenu) {
    case MENU_MAIN:
      displayMainMenu();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        if (UP()) {
          selectedItem = (selectedItem - 1 + maxItems) % maxItems;
          lastButtonPress = currentMillis;
        } else if (DOWN()) {
          selectedItem = (selectedItem + 1) % maxItems;
          lastButtonPress = currentMillis;
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
              currentMenu = MENU_MOTOR_TEST; // 2
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
              
            case 7:  // Motor Settings
              lcd.clear();
              currentMenu = MENU_MOTOR_SETTINGS; // 18
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
              
            case 14:  // WiFi Settings
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
      handleMotorTest();
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
      displayPidForwardSettings();
      handlePidForwardSettings();
      break;

    case MENU_PID_BACKWARD:
      displayPidBackwardSettings();
      handlePidBackwardSettings();
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

    case MENU_MOTOR_INVERT:
      {
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
      {
        displayMenuHeader("Music Settings");

        // Music mode labels and their assigned pins
        String musicModes[4] = { "Station", "Error", "Detect", "Komputer" };
        int* musicPins[4] = { &tempMusicStationPin, &tempMusicErrorPin, &tempMusicDetectPin, &tempMusicKomputerPin };

        for (int i = 0; i < 3 && i < maxMusicItems; i++) {
          lcd.setCursor(0, i + 1);

          // Clear line first
          lcd.print("                    ");
          lcd.setCursor(0, i + 1);

          // Show cursor for selected item
          if (i == selectedMusicItem) {
            lcd.print("> ");
          } else {
            lcd.print("  ");
          }

          // Show mode and pin assignment
          lcd.print(musicModes[i]);
          lcd.print(":");
          lcd.print("Pin");
          lcd.print(*musicPins[i] + 1);  // +1 to show 1-4 instead of 0-3

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

        // Show 4th item (Komputer) if selected
        if (selectedMusicItem == 3) {
          lcd.setCursor(0, 3);
          lcd.print("                    ");
          lcd.setCursor(0, 3);
          lcd.print("> Komputer:Pin");
          lcd.print(tempMusicKomputerPin + 1);
          lcd.setCursor(12, 3);
          lcd.print("A:OK B:Back");
        }

        if (currentMillis - lastButtonPress >= buttonDelay) {
          if (UP()) {
            selectedMusicItem = (selectedMusicItem - 1 + maxMusicItems) % maxMusicItems;
            lastButtonPress = currentMillis;
          } else if (DOWN()) {
            selectedMusicItem = (selectedMusicItem + 1) % maxMusicItems;
            lastButtonPress = currentMillis;
          } else if (LEFT()) {
            // Decrease pin assignment (cycle 0-3)
            switch (selectedMusicItem) {
              case 0: tempMusicStationPin = (tempMusicStationPin - 1 + 4) % 4; break;
              case 1: tempMusicErrorPin = (tempMusicErrorPin - 1 + 4) % 4; break;
              case 2: tempMusicDetectPin = (tempMusicDetectPin - 1 + 4) % 4; break;
              case 3: tempMusicKomputerPin = (tempMusicKomputerPin - 1 + 4) % 4; break;
            }
            lastButtonPress = currentMillis;
          } else if (RIGHT()) {
            // Increase pin assignment (cycle 0-3)
            switch (selectedMusicItem) {
              case 0: tempMusicStationPin = (tempMusicStationPin + 1) % 4; break;
              case 1: tempMusicErrorPin = (tempMusicErrorPin + 1) % 4; break;
              case 2: tempMusicDetectPin = (tempMusicDetectPin + 1) % 4; break;
              case 3: tempMusicKomputerPin = (tempMusicKomputerPin + 1) % 4; break;
            }
            lastButtonPress = currentMillis;
          } else if (START()) {
            // Save all music settings
            musicStationPin = tempMusicStationPin;
            musicErrorPin = tempMusicErrorPin;
            musicDetectPin = tempMusicDetectPin;
            musicKomputerPin = tempMusicKomputerPin;
            saveSettings();
            currentMenu = MENU_MAIN;
            selectedMusicItem = 0;
            menuStartIndex = 0;
            menuNeedsRefresh = true;
            lastButtonPress = currentMillis;
          } else if (STOP()) {
            // Cancel changes
            tempMusicStationPin = musicStationPin;
            tempMusicErrorPin = musicErrorPin;
            tempMusicDetectPin = musicDetectPin;
            tempMusicKomputerPin = musicKomputerPin;
            currentMenu = MENU_MAIN;
            selectedMusicItem = 0;
            menuStartIndex = 0;
            menuNeedsRefresh = true;
            lastButtonPress = currentMillis;
          }
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
      displayUltrasonicCheck();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleUltrasonicCheck();
        if (STOP()) {
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
void displayMotorTest() {
  displayMenuHeader("Motor Test");

  lcd.setCursor(0, 1);
  lcd.print("UP:Maju DOWN:Mundur");
  lcd.setCursor(0, 2);
  lcd.print("LF:Kiri RT:Kanan");
  lcd.setCursor(0, 3);

  // Show current motor state
  switch (motorTestState) {
    case 0: lcd.print("Status: STOP    "); break;
    case 1: lcd.print("Status: MAJU    "); break;
    case 2: lcd.print("Status: MUNDUR  "); break;
    case 3: lcd.print("Status: KIRI    "); break;
    case 4: lcd.print("Status: KANAN   "); break;
  }

  lcd.setCursor(15, 3);
  lcd.print("B:OK");
}

void displayPidSettings() {
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
    currentMenu = MENU_MAIN;
    selectedParam = 0;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}

void displayPidForwardSettings() {
  displayMenuHeader("PID Forward");

  // Display Kp
  lcd.setCursor(0, 1);
  if (selectedParam == 0) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kp: ");
  lcd.print(tempKpForward);
  lcd.print("       ");

  // Display Ki
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Ki: ");
  lcd.print(tempKiForward);
  lcd.print("       ");

  // Display Kd
  lcd.setCursor(0, 3);
  if (selectedParam == 2) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kd: ");
  lcd.print(tempKdForward);
  lcd.print("       ");

  // Show controls
  lcd.setCursor(12, 3);
  lcd.print("B:OK");
}

void displayPidBackwardSettings() {
  displayMenuHeader("PID Backward");

  // Display Kp
  lcd.setCursor(0, 1);
  if (selectedParam == 0) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kp: ");
  lcd.print(tempKpBackward);
  lcd.print("       ");

  // Display Ki
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Ki: ");
  lcd.print(tempKiBackward);
  lcd.print("       ");

  // Display Kd
  lcd.setCursor(0, 3);
  if (selectedParam == 2) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kd: ");
  lcd.print(tempKdBackward);
  lcd.print("       ");

  // Show controls
  lcd.setCursor(12, 3);
  lcd.print("B:OK");
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
      "View All",
      "Delete Station",
      "Clear All",
      "RFID Ujung",
      "RFID Warehouse",
      "Auto Input Station",
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
  displayMenuHeader("Motor Settings");

  lcd.setCursor(0, 1);
  lcd.print("Base Speed:");

  lcd.setCursor(0, 2);
  lcd.print("> ");
  // Convert PWM (100-4000) to percentage (0-100%)
  int percentage = map(tempBaseSpeed, 100, 4000, 0, 100);
  lcd.print(percentage);
  lcd.print("%");
  
  // Show PWM value in smaller text
  lcd.setCursor(8, 2);
  lcd.print("(");
  lcd.print(tempBaseSpeed);
  lcd.print("PWM)");

  // Show range indicator
  lcd.setCursor(0, 3);
  lcd.print("Range: 0-100%");

  // Show controls on last row corner
  lcd.setCursor(12, 3);
  lcd.print("B:OK");
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

void handleMotorTest() {
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
    pwmMotor(0, 0);
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
    return;
  }

  // Execute continuous motor movement based on state
  switch (motorTestState) {
    case 0:  // STOP
      pwmMotor(0, 0);
      break;
    case 1:  // FORWARD
      pwmMotor(-baseSpeed, baseSpeed);
      break;
    case 2:  // BACKWARD
      pwmMotor(baseSpeed, -baseSpeed);
      break;
    case 3:  // LEFT
      pwmMotor(baseSpeed, baseSpeed);
      break;
    case 4:  // RIGHT
      pwmMotor(-baseSpeed, -baseSpeed);
      break;
  }
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

void handlePidForwardSettings() {
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
        case 0: tempKpForward += pidIncrement; break;
        case 1: tempKiForward += pidIncrement; break;
        case 2: tempKdForward += pidIncrement; break;
      }
      // Auto-save to preferences immediately
      kpLinefollowerForward = tempKpForward;
      kiLinefollowerForward = tempKiForward;
      kdLinefollowerForward = tempKdForward;
      saveForwardPidSettings();
      lastRightPress = currentMillis;
    } else if (currentMillis - rightHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - increment by 1.0 every 100ms
      if (currentMillis - lastRightPress >= 100) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: tempKpForward += pidIncrement; break;
          case 1: tempKiForward += pidIncrement; break;
          case 2: tempKdForward += pidIncrement; break;
        }
        // Auto-save to preferences immediately
        kpLinefollowerForward = tempKpForward;
        kiLinefollowerForward = tempKiForward;
        kdLinefollowerForward = tempKdForward;
        saveForwardPidSettings();
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
        case 0: tempKpForward = max(0.0f, (float)(tempKpForward - pidIncrement)); break;
        case 1: tempKiForward = max(0.0f, (float)(tempKiForward - pidIncrement)); break;
        case 2: tempKdForward = max(0.0f, (float)(tempKdForward - pidIncrement)); break;
      }
      // Auto-save to preferences immediately
      kpLinefollowerForward = tempKpForward;
      kiLinefollowerForward = tempKiForward;
      kdLinefollowerForward = tempKdForward;
      saveForwardPidSettings();
      lastLeftPress = currentMillis;
    } else if (currentMillis - leftHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - decrement by 1.0 every 100ms
      if (currentMillis - lastLeftPress >= 100) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: tempKpForward = max(0.0f, (float)(tempKpForward - pidIncrement)); break;
          case 1: tempKiForward = max(0.0f, (float)(tempKiForward - pidIncrement)); break;
          case 2: tempKdForward = max(0.0f, (float)(tempKdForward - pidIncrement)); break;
        }
        // Auto-save to preferences immediately
        kpLinefollowerForward = tempKpForward;
        kiLinefollowerForward = tempKiForward;
        kdLinefollowerForward = tempKdForward;
        saveForwardPidSettings();
        lastLeftPress = currentMillis;
      }
    }
  } else {
    leftHolding = false;
  }

  if (STOP()) {
    kpLinefollowerForward = tempKpForward;
    kiLinefollowerForward = tempKiForward;
    kdLinefollowerForward = tempKdForward;
    saveForwardPidSettings();
    currentMenu = MENU_PID_SETTINGS;
    selectedParam = 0;
    menuNeedsRefresh = true;
  }
}

void handlePidBackwardSettings() {
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
        case 0: tempKpBackward += pidIncrement; break;
        case 1: tempKiBackward += pidIncrement; break;
        case 2: tempKdBackward += pidIncrement; break;
      }
      // Auto-save to preferences immediately
      kpLinefollowerBackward = tempKpBackward;
      kiLinefollowerBackward = tempKiBackward;
      kdLinefollowerBackward = tempKdBackward;
      saveBackwardPidSettings();
      lastRightPress = currentMillis;
    } else if (currentMillis - rightHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - increment by 1.0 every 100ms
      if (currentMillis - lastRightPress >= 100) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: tempKpBackward += pidIncrement; break;
          case 1: tempKiBackward += pidIncrement; break;
          case 2: tempKdBackward += pidIncrement; break;
        }
        // Auto-save to preferences immediately
        kpLinefollowerBackward = tempKpBackward;
        kiLinefollowerBackward = tempKiBackward;
        kdLinefollowerBackward = tempKdBackward;
        saveBackwardPidSettings();
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
        case 0: tempKpBackward = max(0.0f, (float)(tempKpBackward - pidIncrement)); break;
        case 1: tempKiBackward = max(0.0f, (float)(tempKiBackward - pidIncrement)); break;
        case 2: tempKdBackward = max(0.0f, (float)(tempKdBackward - pidIncrement)); break;
      }
      // Auto-save to preferences immediately
      kpLinefollowerBackward = tempKpBackward;
      kiLinefollowerBackward = tempKiBackward;
      kdLinefollowerBackward = tempKdBackward;
      saveBackwardPidSettings();
      lastLeftPress = currentMillis;
    } else if (currentMillis - leftHoldStart > ACCELERATION_INTERVAL) {
      // Button is being held - decrement by 1.0 every 100ms
      if (currentMillis - lastLeftPress >= 100) {
        pidIncrement = 1.0f;
        switch (selectedParam) {
          case 0: tempKpBackward = max(0.0f, (float)(tempKpBackward - pidIncrement)); break;
          case 1: tempKiBackward = max(0.0f, (float)(tempKiBackward - pidIncrement)); break;
          case 2: tempKdBackward = max(0.0f, (float)(tempKdBackward - pidIncrement)); break;
        }
        // Auto-save to preferences immediately
        kpLinefollowerBackward = tempKpBackward;
        kiLinefollowerBackward = tempKiBackward;
        kdLinefollowerBackward = tempKdBackward;
        saveBackwardPidSettings();
        lastLeftPress = currentMillis;
      }
    }
  } else {
    leftHolding = false;
  }

  if (STOP()) {
    kpLinefollowerBackward = tempKpBackward;
    kiLinefollowerBackward = tempKiBackward;
    kdLinefollowerBackward = tempKdBackward;
    saveBackwardPidSettings();
    currentMenu = MENU_PID_SETTINGS;
    selectedParam = 0;
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

      case 2:  // View All
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

      case 3:  // Delete Station
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

      case 4:  // Clear All
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

      case 5:  // RFID Ujung
        currentMenu = MENU_RFID_UJUNG;
        menuNeedsRefresh = true;
        break;

      case 6:  // RFID Warehouse
        currentMenu = MENU_RFID_WAREHOUSE;
        menuNeedsRefresh = true;
        break;

      case 7:  // Auto Input Station
        currentMenu = MENU_AUTO_INPUT_STATION;
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
  if (LEFT()) {
    // Decrease by 5% (equivalent to ~195 PWM)
    int currentPercentage = map(tempBaseSpeed, 100, 4000, 0, 100);
    currentPercentage = max(0, currentPercentage - 5);
    tempBaseSpeed = map(currentPercentage, 0, 100, 100, 4000);
  } else if (RIGHT()) {
    // Increase by 5% (equivalent to ~195 PWM)
    int currentPercentage = map(tempBaseSpeed, 100, 4000, 0, 100);
    currentPercentage = min(100, currentPercentage + 5);
    tempBaseSpeed = map(currentPercentage, 0, 100, 100, 4000);
  } else if (STOP()) {
    // Save motor settings
    baseSpeed = tempBaseSpeed;
    saveSettings();
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;  // Reset scroll position
    menuNeedsRefresh = true;
  }
  // else if (X()) {
  //   // Cancel changes
  //   tempBaseSpeed = baseSpeed;
  //   currentMenu = MENU_MAIN;
  //   menuStartIndex = 0; // Reset scroll position
  //   menuNeedsRefresh = true;
  // }
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
    
    // Reset Forward PID values to defaults
    tempKpForward = 70.0;
    tempKiForward = 0.0;
    tempKdForward = 0.0;
    
    // Reset Backward PID values to defaults
    tempKpBackward = 70.0;
    tempKiBackward = 0.0;
    tempKdBackward = 0.0;
    
    // Debug: Print reset values
    Serial.println("=== PID Values Reset to Defaults ===");
    Serial.println("Reset Forward PID - Kp: " + String(tempKpForward) + ", Ki: " + String(tempKiForward) + ", Kd: " + String(tempKdForward));
    Serial.println("Reset Backward PID - Kp: " + String(tempKpBackward) + ", Ki: " + String(tempKiBackward) + ", Kd: " + String(tempKdBackward));

    // Reset Motor values to defaults
    tempBaseSpeed = 1000;

    // Reset Motor invert values to defaults
    tempInvertY = false;
    tempInvertX = false;
    tempInvertKanan = false;
    tempInvertKiri = false;
    tempInvertHook = false;

    // Reset Music mapping values to defaults
    tempMusicStationPin = 0;   // pinMusic1
    tempMusicErrorPin = 1;     // pinMusic2
    tempMusicDetectPin = 2;    // pinMusic3
    tempMusicKomputerPin = 3;  // pinMusic4

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
  displayMenuHeader("Music Test");

  lcd.setCursor(0, 1);
  lcd.print("UP   : Station");
  lcd.setCursor(0, 2);
  lcd.print("DOWN : Error");
  lcd.setCursor(0, 3);
  lcd.print("LF:Detect RT:Komputer");
}

void handleMusicTest() {
  if (UP()) {
    statusMusic = false;
    music(MUSIC_MODE_STATION);
    lcd.setCursor(15, 1);
    lcd.print("ON ");
  } else if (DOWN()) {
    statusMusic = false;
    music(MUSIC_MODE_ERROR);
    lcd.setCursor(15, 2);
    lcd.print("ON ");
  } else if (LEFT()) {
    statusMusic = false;
    music(MUSIC_MODE_DETECT);
    lcd.setCursor(15, 3);
    lcd.print("ON ");
  } else if (RIGHT()) {
    statusMusic = false;
    music(MUSIC_MODE_KOMPUTER);
    lcd.setCursor(15, 3);
    lcd.print("ON ");
  } else if (STOP()) {
    stopMusic();
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;
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

  lcd.setCursor(0, 2);
  lcd.print("Segments:");
  if (totalSensorAktif > 0) {
    for (int i = 0; i < 16; i++) {
      if (jumlahMagnet[i]) {
        lcd.print(i + 1);
        lcd.print(",");
        break;  // Show only first few due to space
      }
    }
  } else {
    lcd.print("None");
  }
  lcd.print("        ");  // Clear remaining characters

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
  displayMenuHeader("Ultrasonic Check");

  lcd.setCursor(0, 1);
  lcd.print("P1:");
  lcd.print(ultrasonicDistances[0]);
  lcd.print(" P2:");
  lcd.print(ultrasonicDistances[1]);

  lcd.setCursor(0, 2);
  lcd.print("P3:");
  lcd.print(ultrasonicDistances[2]);
  lcd.print(" P4:");
  lcd.print(ultrasonicDistances[3]);
  lcd.print(" P5:");
  lcd.print(ultrasonicDistances[4]);

  lcd.setCursor(0, 3);
  if (obstacleDetected) {
    lcd.print("OBSTACLE! ");
  } else {
    lcd.print("Clear ");
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
  const unsigned long SWITCH_DEBOUNCE = 200; // 200ms debounce for switching
  
  loopUltrasonik();
  checkObstacles();

  unsigned long currentTime = millis();
  
  if (LEFT() && (currentTime - lastSwitchTime >= SWITCH_DEBOUNCE)) {
    Serial.println("[INFO] Switching to FRONT ultrasonic sensor");
    // Switch to front ultrasonic sensor
    setUltrasonicSlaveId(SLAVEID_ULTRASONIK_DEPAN);
    lastSwitchTime = currentTime;
  } else if (RIGHT() && (currentTime - lastSwitchTime >= SWITCH_DEBOUNCE)) {
    Serial.println("[INFO] Switching to BACK ultrasonic sensor");
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
  if (isConnectingWifi) {
    // Show connecting status
    lcd.setCursor(0, 0);
    lcd.print("Mencari WiFi        ");
    unsigned long elapsed = millis() - wifiConnectStartTime;
    
    lcd.setCursor(0, 2);
    if (WiFi.status() == WL_CONNECTED) {
      lcd.print("Berhasil!           ");
      isConnectingWifi = false;
      wifiConnectionResult = true;
    } else if (elapsed >= WIFI_CONNECT_TIMEOUT) {
      lcd.print("Gagal!              ");
      isConnectingWifi = false;
      wifiConnectionResult = false;
    } else {
      int dots = (elapsed / 500) % 4;
      lcd.print("Menunggu");
      for (int i = 0; i < dots; i++) {
        lcd.print(".");
      }
      for (int i = dots; i < 3; i++) {
        lcd.print(" ");
      }
      lcd.print("        ");
    }
    
    lcd.setCursor(0, 1);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print("                    ");
    return;
  }
  
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
  if (isConnectingWifi) {
    // Check if connection completed
    unsigned long elapsed = millis() - wifiConnectStartTime;
    if (WiFi.status() == WL_CONNECTED || elapsed >= WIFI_CONNECT_TIMEOUT) {
      isConnectingWifi = false;
    }
    return;
  }
  

  
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
      delay(50);
    }
    
  } else if (STOP()) {
    currentMenu = MENU_RFID_SETTINGS;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}
