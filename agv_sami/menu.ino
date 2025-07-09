#include "menu.h"

// Menu states
#define MENU_MAIN 0
#define MENU_MOTOR_TEST 2
#define MENU_PID_SETTINGS 3
#define MENU_TARGET_SETTINGS 4
#define MENU_AGV_MODE 1
#define MENU_RFID_SETTINGS 5
#define MENU_MOTOR_SETTINGS 6
#define MENU_RESET 7

int selectedItem = 0;
int maxItems = 7;
int menuStartIndex = 0; // For scrolling menu
const int maxMenuDisplay = 3; // Max items shown at once (row 1-3, row 0 for header)
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
const unsigned long RFID_SCAN_TIMEOUT = 10000; // 10 seconds timeout

// Target settings variables
unsigned long xButtonHoldStart = 0;
const unsigned long X_HOLD_DURATION = 3000; // 3 seconds hold
bool isClearingStations = false;

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

void setupMenu() {
  preferences.begin("agv-settings", false);
  
  // Load PID settings
  tempKp = preferences.getDouble("kpLinefollower", 70.0);
  tempKi = preferences.getDouble("kiLinefollower", 0.0);
  tempKd = preferences.getDouble("kdLinefollower", 0.0);
  
  // Load Motor settings
  tempBaseSpeed = preferences.getInt("baseSpeed", 1000);
  
  // Apply PID values
  kpLinefollower = tempKp;
  kiLinefollower = tempKi;
  kdLinefollower = tempKd;
  
  // Apply Motor values
  baseSpeed = tempBaseSpeed;
  
  preferences.end();
  
  // Load RFID stations
  loadRfidStations();
  
  // Load stations list from HTTP preferences
  loadStationsFromPreferences();
  
  Serial.print("Loaded stationsList size: ");
  Serial.println(stationsList.size());
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

  // Apply PID values
  kpLinefollower = tempKp;
  kiLinefollower = tempKi;
  kdLinefollower = tempKd;
  
  // Apply Motor values
  baseSpeed = tempBaseSpeed;

  // End preferences session
  preferences.end();
}

void displayMainMenu() {
  // Menu items array
  String menuItems[7] = {
    "AGV Mode",
    "Motor Test", 
    "PID Settings",
    "Target Settings",
    "RFID Settings",
    "Motor Settings",
    "Reset Settings"
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
    lcd.clear(); // Only clear when really needed
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
      lcd.print("^"); // Up arrow if can scroll up
    } else {
      lcd.print(" ");
    }
    
    lcd.setCursor(19, 3);
    if (menuStartIndex + maxMenuDisplay < maxItems) {
      lcd.print("v"); // Down arrow if can scroll down
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
  lcd.print("UP    : Forward");
  lcd.setCursor(0, 2);
  lcd.print("DOWN  : Backward");
  lcd.setCursor(0, 3);
  lcd.print("LF/RT : Turn   B:OK");
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
    lcd.print("                "); // Clear line
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
    pwmMotor(baseSpeed, -baseSpeed);
  } else if (DOWN()) {
    pwmMotor(-baseSpeed, baseSpeed);
  } else if (LEFT()) {
    pwmMotor(baseSpeed, baseSpeed);
  } else if (RIGHT()) {
    pwmMotor(-baseSpeed, -baseSpeed);
  } else if (B()) {
    pwmMotor(0, 0);
    currentMenu = MENU_MAIN;
    menuStartIndex = 0; // Reset scroll position
    menuNeedsRefresh = true;
  } else {
    pwmMotor(0, 0);
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

  if (B()) {
    kpLinefollower = tempKp;
    kiLinefollower = tempKi;
    kdLinefollower = tempKd;
    saveSettings();
    currentMenu = MENU_MAIN;
    menuStartIndex = 0; // Reset scroll position
    menuNeedsRefresh = true;
  } else if (X()) {
    // Batal menyimpan, kembali ke menu
    kpLinefollower = tempKp;
    kiLinefollower = tempKi;
    kdLinefollower = tempKd;
    currentMenu = MENU_MAIN;
    menuStartIndex = 0; // Reset scroll position
    menuNeedsRefresh = true;
  }
}

void handleTargetSettings() {
  unsigned long currentMillis = millis();
  
  if (X()) {
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
    // X button released, cancel clearing
    if (isClearingStations) {
      isClearingStations = false;
      xButtonHoldStart = 0;
    }
  }
  
  if (B()) {
    // Load latest stations from preferences
    loadStationsFromPreferences();
    isClearingStations = false;
    xButtonHoldStart = 0;
    currentMenu = MENU_MAIN;
    menuStartIndex = 0; // Reset scroll position
    menuNeedsRefresh = true;
  }
}

void handleRfidSettings() {
  unsigned long currentMillis = millis();

  if (isWaitingForRfid) {
    // Check if new RFID was scanned
    if (newRfidScanned) {
      // RFID card detected, save it
      if (addRfidStation(selectedStationId, lastScannedRfid)) {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Station ");
        lcd.print(selectedStationId);
        lcd.print(" saved!");
        lcd.setCursor(0, 2);
        lcd.print("RFID: ");
        lcd.print(lastScannedRfid.substring(0, 8));
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
      selectedRfidItem = 1; // Stay on scan option for next scan
    }
    
    // Check for timeout or cancel
    if ((currentMillis - rfidScanTimeout > RFID_SCAN_TIMEOUT) || B()) {
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
  } else if (A()) {
    switch (selectedRfidItem) {
      case 1: // Scan RFID
        isWaitingForRfid = true;
        rfidScanTimeout = currentMillis;
        newRfidScanned = false; // Reset flag
        break;
        
      case 2: // View All
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
            if (B()) {
              delay(200);
              break;
            }
            delay(1);
          }
        }
        break;
        
      case 3: // Delete Station
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
        
      case 4: // Clear All
        {
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("Clear all RFID?");
          lcd.setCursor(0, 2);
          lcd.print("A: Yes  B: No");
          
          // Wait for confirmation
          while (true) {
            if (A()) {
              clearAllRfidStations();
              lcd.clear();
              lcd.setCursor(0, 1);
              lcd.print("All stations");
              lcd.setCursor(0, 2);
              lcd.print("cleared!");
              delay(1500);
              break;
            } else if (B()) {
              break;
            }
            delay(50);
          }
        }
        break;
    }
  } else if (B()) {
    currentMenu = MENU_MAIN;
    selectedRfidItem = 0;
    menuStartIndex = 0; // Reset scroll position
    menuNeedsRefresh = true;
  }
}

void handleMotorSettings() {
  if (LEFT()) {
    tempBaseSpeed = max(100, tempBaseSpeed - 50);  // Minimum 100, decrease by 50
  } else if (RIGHT()) {
    tempBaseSpeed = min(4000, tempBaseSpeed + 50); // Maximum 4000, increase by 50
  } else if (B()) {
    // Save motor settings
    baseSpeed = tempBaseSpeed;
    saveSettings();
    currentMenu = MENU_MAIN;
    menuStartIndex = 0; // Reset scroll position
    menuNeedsRefresh = true;
  } else if (X()) {
    // Cancel changes
    tempBaseSpeed = baseSpeed;
    currentMenu = MENU_MAIN;
    menuStartIndex = 0; // Reset scroll position
    menuNeedsRefresh = true;
  }
}

void handleResetMenu() {
  if (A()) {
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
    menuStartIndex = 0; // Reset scroll position
    menuNeedsRefresh = true;
    tombolBoot = true;
    setup();  // Return to main menu
  } else if (B()) {
    currentMenu = MENU_MAIN;
    menuStartIndex = 0; // Reset scroll position
    menuNeedsRefresh = true;
  }
}

void handleMenu() {
  unsigned long currentMillis = millis();

  // If in AGV mode, only check for B button to exit
  if (isAgvMode) {
    if (B()) {
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
        } else if (A()) {
          if (selectedItem == 0) {  // AGV Mode
            isAgvMode = true;
            menuStartIndex = 0; // Reset scroll position
            menuNeedsRefresh = true;
          } else if (selectedItem == 4) {  // RFID Settings (item 5)
            currentMenu = MENU_RFID_SETTINGS;
            menuNeedsRefresh = true;
          } else if (selectedItem == 5) {  // Motor Settings (item 6)
            currentMenu = MENU_MOTOR_SETTINGS;
            menuNeedsRefresh = true;
          } else if (selectedItem == 6) {  // Reset Settings (item 7)
            currentMenu = MENU_RESET;
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
        if (LEFT() || RIGHT() || UP() || DOWN() || B()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_TARGET_SETTINGS:
      displayTargetSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleTargetSettings();
        if (LEFT() || RIGHT() || UP() || DOWN() || B()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_RFID_SETTINGS:
      displayRfidSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleRfidSettings();
        if (LEFT() || RIGHT() || UP() || DOWN() || A() || B()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_MOTOR_SETTINGS:
      displayMotorSettings();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleMotorSettings();
        if (LEFT() || RIGHT() || B() || X()) {
          lastButtonPress = currentMillis;
        }
      }
      break;

    case MENU_RESET:
      displayResetMenu();
      if (currentMillis - lastButtonPress >= buttonDelay) {
        handleResetMenu();
        if (A() || B()) {
          lastButtonPress = currentMillis;
        }
      }
      break;
  }
}
