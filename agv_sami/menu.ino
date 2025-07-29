#include "menu.h"

// Menu states
#define MENU_MAIN 0
#define MENU_MOTOR_TEST 2
#define MENU_PID_SETTINGS 3
#define MENU_TARGET_SETTINGS 4
#define MENU_AGV_MODE 1
#define MENU_RFID_SETTINGS 5
#define MENU_MOTOR_SETTINGS 6
#define MENU_MOTOR_INVERT 7
#define MENU_MUSIC_SETTINGS 8
#define MENU_MUSIC_TEST 9
#define MENU_HOOK_TEST 10
#define MENU_RESET 11
#define MENU_MAGNET_CHECK 12
#define MENU_ULTRASONIC_CHECK 13
#define MENU_WIFI_SETTINGS 14

int selectedItem = 0;
int maxItems = 14;
int menuStartIndex = 0;        // For scrolling menu
const int maxMenuDisplay = 3;  // Max items shown at once (row 1-3, row 0 for header)
bool isAgvMode = false;

// Menu refresh flags - to prevent flickering
bool menuNeedsRefresh = true;
int lastSelectedItem = -1;
int lastMenuStartIndex = -1;

// PID settings
double tempKp = kpLinefollower;
double tempKi = kiLinefollower;
double tempKd = kdLinefollower;

// Motor settings
int tempBaseSpeed = baseSpeed;

// Target station settings
bool useAutoTarget = false;         // New variable to track target source
int manualTargetCount = 2;          // Default to 2 targets for manual mode
const int MAX_MANUAL_TARGETS = 10;  // Maximum number of manual targets allowed

// Selection variables
int selectedParam = 0;   // For PID settings menu
int selectedTarget = 0;  // For Target settings menu

// Button handling
unsigned long lastButtonPress = 0;
const unsigned long buttonDelay = 200;  // Delay in milliseconds between button presses


// Add these variables at the top with other global variables
unsigned long pidButtonHoldStart = 0;
float pidIncrement = 0.1f;
const float MAX_INCREMENT = 10.0f;
const unsigned long ACCELERATION_INTERVAL = 500;  // Time in ms to increase increment

// RFID menu variables
int selectedRfidItem = 0;
int selectedStationId = 1;
bool isWaitingForRfid = false;
unsigned long rfidScanTimeout = 0;
const unsigned long RFID_SCAN_TIMEOUT = 10000;  // 10 seconds timeout

// Target settings variables
unsigned long xButtonHoldStart = 0;
const unsigned long X_HOLD_DURATION = 3000;  // 3 seconds hold
bool isClearingStations = false;

// Motor invert settings
bool tempInvertY = invertMotorY;
bool tempInvertX = invertMotorX;
bool tempInvertKanan = invertMotorKanan;
bool tempInvertKiri = invertMotorKiri;
bool tempInvertHook = invertHook;

// Music mapping settings
int tempMusicStationPin = musicStationPin;
int tempMusicErrorPin = musicErrorPin;
int tempMusicDetectPin = musicDetectPin;
int tempMusicKomputerPin = musicKomputerPin;

// Motor invert menu variables
int selectedInvertItem = 0;  // 0=Y-axis, 1=X-axis, 2=Motor Kanan, 3=Motor Kiri, 4=Hook
const int maxInvertItems = 5;

// Music settings menu variables
int selectedMusicItem = 0;  // 0=Station, 1=Error, 2=Detect, 3=Komputer
const int maxMusicItems = 4;

// Motor test variables
int motorTestState = 0;  // 0=stop, 1=forward, 2=backward, 3=left, 4=right

// Hook test variables
int hookTestState = 0;  // 0=stop, 1=naik, 2=turun

// WiFi connection variables
bool isConnectingWifi = false;
bool wifiConnectionResult = false;
unsigned long wifiConnectStartTime = 0;
const unsigned long WIFI_CONNECT_TIMEOUT = 5000; // 10 seconds

// WiFi scroll variables
int wifiScrollIndex = 0;
const int WIFI_MAX_SCROLL = 3; // Maximum scroll positions timeout

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

void displayMainMenu() {
  // Menu items array
  String menuItems[14] = {
    "AGV Mode",
    "Motor Test",
    "PID Settings",
    "Target Settings",
    "RFID Settings",
    "Motor Settings",
    "Motor Invert",
    "Music Settings",
    "Music Test",
    "Hook Test",
    "Reset Settings",
    "Magnet Check",
    "Ultrasonic Check",
    "WiFi Settings"
  };

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
    if (menuStartIndex > 0) {
      lcd.print("^");  // Up arrow if can scroll up
    } else {
      lcd.print(" ");
    }

    lcd.setCursor(19, 3);
    if (menuStartIndex + maxMenuDisplay < maxItems) {
      lcd.print("v");  // Down arrow if can scroll down
    } else {
      lcd.print(" ");
    }

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

  // Display Ki
  lcd.setCursor(0, 2);
  if (selectedParam == 1) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Ki: ");
  lcd.print(tempKi);

  // Display Kd
  lcd.setCursor(0, 3);
  if (selectedParam == 2) lcd.print("> ");
  else lcd.print("  ");
  lcd.print("Kd: ");
  lcd.print(tempKd);

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

  if (stationsList.empty()) {
    lcd.setCursor(0, 2);
    lcd.print("No stations");
    lcd.setCursor(0, 3);
    lcd.print("Use HTTP API");
  } else {
    lcd.setCursor(0, 2);
    lcd.print("Total:");
    lcd.print(stationsList.size());

    lcd.setCursor(0, 3);
    for (size_t i = 0; i < stationsList.size() && i < 3; i++) {
      lcd.print(stationsList[i]);
      if (i < stationsList.size() - 1 && i < 2) lcd.print(",");
    }
    if (stationsList.size() > 3) {
      lcd.print("..");
    }
  }

  // Show controls
  lcd.setCursor(8, 3);
  lcd.print("X:Clear B:OK");
}

void displayRfidSettings() {
  displayMenuHeader("RFID Settings");

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

    return;
  }

  // RFID Menu items
  String rfidMenuItems[5] = {
    "Station: " + String(selectedStationId),
    "Scan RFID",
    "View All",
    "Delete Station",
    "Clear All"
  };

  // Simple display - show items with scrolling if needed
  int startIdx = max(0, min(selectedRfidItem - 1, 5 - 3));

  for (int i = 0; i < 3 && (startIdx + i) < 5; i++) {
    int itemIndex = startIdx + i;
    lcd.setCursor(0, i + 1);
    lcd.print("                ");  // Clear line
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
  lcd.setCursor(12, 3);
  lcd.print("A:OK");
}

void displayMotorSettings() {
  displayMenuHeader("Motor Settings");

  lcd.setCursor(0, 1);
  lcd.print("Base Speed (PWM):");

  lcd.setCursor(0, 2);
  lcd.print("> ");
  lcd.print(tempBaseSpeed);

  // Show range indicator
  lcd.setCursor(0, 3);
  lcd.print("Range: 100-4000");

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
  unsigned long currentMillis = millis();

  if (UP()) {
    selectedParam = (selectedParam - 1 + 3) % 3;
    pidButtonHoldStart = 0;
  } else if (DOWN()) {
    selectedParam = (selectedParam + 1) % 3;
    pidButtonHoldStart = 0;
  } else if (RIGHT()) {
    if (pidButtonHoldStart == 0) {
      pidButtonHoldStart = currentMillis;
      pidIncrement = 0.1f;
    }

    // Increase increment based on hold time
    if (currentMillis - pidButtonHoldStart > ACCELERATION_INTERVAL) {
      pidIncrement = min(MAX_INCREMENT, pidIncrement + 0.1f);
    }

    switch (selectedParam) {
      case 0: tempKp += pidIncrement; break;
      case 1: tempKi += pidIncrement; break;
      case 2: tempKd += pidIncrement; break;
    }
  } else if (LEFT()) {
    if (pidButtonHoldStart == 0) {
      pidButtonHoldStart = currentMillis;
      pidIncrement = 0.1f;
    }

    // Increase increment based on hold time
    if (currentMillis - pidButtonHoldStart > ACCELERATION_INTERVAL) {
      pidIncrement = min(MAX_INCREMENT, pidIncrement + 0.1f);
    }

    switch (selectedParam) {
      case 0: tempKp = max(0.0f, (float)(tempKp - pidIncrement)); break;
      case 1: tempKi = max(0.0f, (float)(tempKi - pidIncrement)); break;
      case 2: tempKd = max(0.0f, (float)(tempKd - pidIncrement)); break;
    }
  } else {
    // Reset when no button is pressed
    pidButtonHoldStart = 0;
    pidIncrement = 0.1f;
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
        // Call clearStationsData after 3 seconds
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Clearing stations...");

        clearStationsData();

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
    loadStationsListFromPreferences();
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
    selectedRfidItem = (selectedRfidItem - 1 + 5) % 5;
  } else if (DOWN()) {
    selectedRfidItem = (selectedRfidItem + 1) % 5;
  } else if (RIGHT()) {
    if (selectedRfidItem == 0) {
      // Change station ID
      selectedStationId = (selectedStationId % 10) + 1;
    }
  } else if (LEFT()) {
    if (selectedRfidItem == 0) {
      // Change station ID
      selectedStationId = selectedStationId == 1 ? 10 : selectedStationId - 1;
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
            if (START()) {
              delay(200);
              break;
            }
            delay(1);
          }
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
    tempBaseSpeed = max(100, tempBaseSpeed - 50);  // Minimum 100, decrease by 50
  } else if (RIGHT()) {
    tempBaseSpeed = min(4000, tempBaseSpeed + 50);  // Maximum 4000, increase by 50
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

    // Clear stationsList in memory
    stationsList.clear();

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

void handleMenu() {
  unsigned long currentMillis = millis();

  // If in AGV mode, only check for B button to exit
  if (isAgvMode) {
    if (STOP()) {
      isAgvMode = false;
      currentMenu = MENU_MAIN;
      modeBerhenti = true;
      force = false;
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
          if (selectedItem == 0) {  // AGV Mode
            isAgvMode = true;
            menuStartIndex = 0;  // Reset scroll position
            menuNeedsRefresh = true;
          } else if (selectedItem == 4) {  // RFID Settings (item 5)
            currentMenu = MENU_RFID_SETTINGS;
            menuNeedsRefresh = true;
          } else if (selectedItem == 5) {  // Motor Settings (item 6)
            currentMenu = MENU_MOTOR_SETTINGS;
            menuNeedsRefresh = true;
          } else if (selectedItem == 6) {  // Motor Invert (item 7)
            currentMenu = MENU_MOTOR_INVERT;
            menuNeedsRefresh = true;
          } else if (selectedItem == 7) {  // Music Settings (item 8)
            currentMenu = MENU_MUSIC_SETTINGS;
            menuNeedsRefresh = true;
          } else if (selectedItem == 8) {  // Music Test (item 9)
            currentMenu = MENU_MUSIC_TEST;
            menuNeedsRefresh = true;
          } else if (selectedItem == 9) {  // Hook Test (item 10)
            currentMenu = MENU_HOOK_TEST;
            menuNeedsRefresh = true;
          } else if (selectedItem == 10) {  // Reset Settings (item 11)
            currentMenu = MENU_RESET;
            menuNeedsRefresh = true;
          } else if (selectedItem == 11) {  // Magnet Check (item 12)
            currentMenu = MENU_MAGNET_CHECK;
            menuNeedsRefresh = true;
          } else if (selectedItem == 12) {  // Ultrasonic Check (item 13)
            currentMenu = MENU_ULTRASONIC_CHECK;
            menuNeedsRefresh = true;
          } else if (selectedItem == 13) {  // WiFi Settings (item 14)
            currentMenu = MENU_WIFI_SETTINGS;
            menuNeedsRefresh = true;
          } else {
            currentMenu = selectedItem + 1;
            menuNeedsRefresh = true;
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
      displayPidSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handlePidSettings();
        if (LEFT() || RIGHT() || UP() || DOWN() || STOP()) {
          lastButtonPress = currentMillis;
        }
      }
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
    music("station");
    lcd.setCursor(15, 1);
    lcd.print("ON ");
  } else if (DOWN()) {
    statusMusic = false;
    music("error");
    lcd.setCursor(15, 2);
    lcd.print("ON ");
  } else if (LEFT()) {
    statusMusic = false;
    music("detect");
    lcd.setCursor(15, 3);
    lcd.print("ON ");
  } else if (RIGHT()) {
    statusMusic = false;
    music("komputer");
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
    hook("naik");
  } else if (DOWN()) {
    hookTestState = 2;  // Set to turun
    hook("turun");
  } else if (STOP()) {
    hookTestState = 0;  // Stop
    hook("stop");       // Stop hook movement
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  } else {
    // Continue current state
    switch (hookTestState) {
      case 1:  // Continue naik
        hook("naik");
        break;
      case 2:  // Continue turun
        hook("turun");
        break;
      case 0:  // Stopped
      default:
        hook("stop");
        break;
    }
  }
}

void handleMagnetCheck() {
  // Selalu baca sensor saat menu ini aktif
  bacaSensor();

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
  loopUltrasonik();

  if (LEFT() && !isTimerActive(&ultrasonicSwitchTimer)) {
    Serial.println("[INFO] Switching to FRONT ultrasonic sensor");
    // Switch to front ultrasonic sensor
    switchUltrasonicSensor(true);
    startTimer(&ultrasonicSwitchTimer, 100);  // Non-blocking delay to prevent multiple triggers
  } else if (RIGHT() && !isTimerActive(&ultrasonicSwitchTimer)) {
    Serial.println("[INFO] Switching to BACK ultrasonic sensor");
    // Switch to back ultrasonic sensor
    switchUltrasonicSensor(false);
    startTimer(&ultrasonicSwitchTimer, 100);  // Non-blocking delay to prevent multiple triggers
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
            lcd.print("       ");
          }
        }
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
    // Scroll up
    wifiScrollIndex--;
    if (wifiScrollIndex < 0) {
      wifiScrollIndex = WIFI_MAX_SCROLL;
    }
    delay(200); // Debounce
  } else if (DOWN()) {
    // Scroll down
    wifiScrollIndex++;
    if (wifiScrollIndex > WIFI_MAX_SCROLL) {
      wifiScrollIndex = 0;
    }
    delay(200); // Debounce
  } else if (START()) {
    // Start WiFi connection
    isConnectingWifi = true;
    wifiConnectStartTime = millis();
    
    // Load WiFi config and attempt connection
    loadWifiConfig();
    WiFi.disconnect();
    delay(100);
    
    // Try to connect as client first
    WiFi.mode(WIFI_AP_STA);  // Enable both AP and STA mode
    if (strlen(staticIPStr) > 0) {
      IPAddress staticIP, gateway, subnet, dns;
      staticIP.fromString(staticIPStr);
      gateway.fromString(gatewayStr);
      subnet.fromString(subnetStr);
      dns.fromString(dnsStr);
      WiFi.config(staticIP, gateway, subnet, dns);
    }
    WiFi.begin(ssid, password);
    
    // Ensure AP is still active for web access
    WiFi.softAP("ESP32-AGV-Config", "12345678");
    WiFi.softAPConfig(IPAddress(192, 168, 121, 14), IPAddress(192, 168, 121, 14), IPAddress(255, 255, 255, 0));
    
  } else if (STOP()) {
    currentMenu = MENU_MAIN;
    menuStartIndex = 0;
    menuNeedsRefresh = true;
  }
}
