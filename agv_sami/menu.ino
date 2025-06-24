#include "menu.h"

// Menu states
#define MENU_MAIN 0
#define MENU_MOTOR_TEST 2
#define MENU_PID_SETTINGS 3
#define MENU_TARGET_SETTINGS 4
#define MENU_AGV_MODE 1
#define MENU_RESET 5
#define MENU_RFID_SETTINGS 6

int selectedItem = 0;
int maxItems = 6;
bool isAgvMode = false;

// PID settings
double tempKp = kpLinefollower;
double tempKi = kiLinefollower;
double tempKd = kdLinefollower;

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
    display.print("> ");
  } else {
    display.print("  ");
  }
}

void displayMenuHeader(const char* title) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(title);
}

void displayMenuFooter(const char* text) {
  display.setCursor(0, 50);
  display.println(text);
  display.display();
}

void setupMenu() {
  preferences.begin("agv-settings", false);
  
  // Load PID settings
  tempKp = preferences.getDouble("kpLinefollower", 70.0);
  tempKi = preferences.getDouble("kiLinefollower", 0.0);
  tempKd = preferences.getDouble("kdLinefollower", 0.0);
  
  // Apply PID values
  kpLinefollower = tempKp;
  kiLinefollower = tempKi;
  kdLinefollower = tempKd;
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

  // Apply PID values
  kpLinefollower = tempKp;
  kiLinefollower = tempKi;
  kdLinefollower = tempKd;

  // End preferences session
  preferences.end();
}

void displayMainMenu() {
  displayMenuHeader("AGV Menu:");

  display.setCursor(0, 10);
  displayIndicator(0, selectedItem);
  display.println("1. AGV Mode");

  display.setCursor(0, 18);
  displayIndicator(1, selectedItem);
  display.println("2. Motor Test");

  display.setCursor(0, 26);
  displayIndicator(2, selectedItem);
  display.println("3. PID Settings");

  display.setCursor(0, 34);
  displayIndicator(3, selectedItem);
  display.println("4. Target Settings");

  display.setCursor(0, 42);
  displayIndicator(4, selectedItem);
  display.println("5. RFID Settings");

  display.setCursor(0, 50);
  displayIndicator(5, selectedItem);
  display.println("6. Reset Settings");
  display.display();
}

void displayMotorTest() {
  displayMenuHeader("Motor Test");
  display.println("UP: Forward");
  display.println("DOWN: Backward");
  display.println("LEFT: Left Turn");
  display.println("RIGHT: Right Turn");
  displayMenuFooter("B: Back to Menu");
}

void displayPidSettings() {
  displayMenuHeader("PID Settings");

  display.setCursor(0, 10);
  displayIndicator(0, selectedParam);
  display.print("Kp: ");
  display.println(tempKp);

  display.setCursor(0, 20);
  displayIndicator(1, selectedParam);
  display.print("Ki: ");
  display.println(tempKi);

  display.setCursor(0, 30);
  displayIndicator(2, selectedParam);
  display.print("Kd: ");
  display.println(tempKd);

  displayMenuFooter("B: Save | X: Cancel");
}

void displayTargetSettings() {
  displayMenuHeader("Target Settings");

  if (isClearingStations) {
    display.setCursor(0, 20);
    display.println("Clearing stations...");
    
    // Show progress bar
    unsigned long elapsed = millis() - xButtonHoldStart;
    int progress = (elapsed * 100) / X_HOLD_DURATION;
    progress = min(progress, 100);
    
    display.setCursor(0, 30);
    display.print("Progress: ");
    display.print(progress);
    display.println("%");
    
    display.setCursor(0, 40);
    display.println("Release X to cancel");
    display.display();
    return;
  }

  display.setCursor(0, 10);
  display.println("Stations from HTTP:");
  
  if (stationsList.empty()) {
    display.setCursor(0, 20);
    display.println("No stations loaded");
    display.setCursor(0, 30);
    display.println("Use HTTP API to");
    display.setCursor(0, 38);
    display.println("add stations");
  } else {
    display.setCursor(0, 20);
    display.print("Total: ");
    display.println(stationsList.size());
    
    display.setCursor(0, 30);
    display.print("Stations: ");
    for (size_t i = 0; i < stationsList.size() && i < 5; i++) {
      display.print(stationsList[i]);
      if (i < stationsList.size() - 1 && i < 4) display.print(",");
    }
    if (stationsList.size() > 5) {
      display.print("...");
    }
  }

  displayMenuFooter("B: Back | X: Clear");
}

void displayRfidSettings() {
  displayMenuHeader("RFID Settings");

  if (isWaitingForRfid) {
    display.setCursor(0, 10);
    display.print("Scanning Station ");
    display.print(selectedStationId);
    display.println("...");
    
    display.setCursor(0, 20);
    display.println("Tap RFID card");
    
    display.setCursor(0, 30);
    int remainingTime = (RFID_SCAN_TIMEOUT - (millis() - rfidScanTimeout)) / 1000;
    display.print("Timeout: ");
    display.print(remainingTime);
    display.println("s");
    
    display.setCursor(0, 40);
    display.println("B: Cancel");
    
    display.display();
    return;
  }

  display.setCursor(0, 10);
  displayIndicator(0, selectedRfidItem);
  display.print("Station: ");
  display.println(selectedStationId);

  display.setCursor(0, 18);
  displayIndicator(1, selectedRfidItem);
  display.println("Scan RFID");

  display.setCursor(0, 26);
  displayIndicator(2, selectedRfidItem);
  display.println("View All");

  display.setCursor(0, 34);
  displayIndicator(3, selectedRfidItem);
  display.println("Delete Station");

  display.setCursor(0, 42);
  displayIndicator(4, selectedRfidItem);
  display.println("Clear All");

  displayMenuFooter("B: Back");
}

void displayResetMenu() {
  displayMenuHeader("Reset Settings");
  display.println("Press A to confirm");
  display.println("Press B to cancel");
  display.println("");
  display.println("This will reset:");
  display.println("- PID Settings");
  display.println("- Target Settings");
  display.println("- RFID Settings");
  display.println("- Button Calibration");
  display.display();
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
  } else if (X()) {
    // Batal menyimpan, kembali ke menu
    kpLinefollower = tempKp;
    kiLinefollower = tempKi;
    kdLinefollower = tempKd;
    currentMenu = MENU_MAIN;
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
        display.clearDisplay();
        display.setCursor(0, 20);
        display.println("Clearing stations...");
        display.display();
        
        clearStationsData();
        
        display.clearDisplay();
        display.setCursor(0, 20);
        display.println("Stations cleared!");
        display.display();
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
  }
}

void handleRfidSettings() {
  unsigned long currentMillis = millis();

  if (isWaitingForRfid) {
    // Check if new RFID was scanned
    if (newRfidScanned) {
      // RFID card detected, save it
      if (addRfidStation(selectedStationId, lastScannedRfid)) {
        display.clearDisplay();
        display.setCursor(0, 10);
        display.print("Station ");
        display.print(selectedStationId);
        display.println(" saved!");
        display.setCursor(0, 20);
        display.println("RFID: " + lastScannedRfid.substring(0, 8) + "...");
        display.display();
        delay(2000);
      } else {
        display.clearDisplay();
        display.setCursor(0, 10);
        display.println("Error saving!");
        display.display();
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
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("RFID Stations:");
          
          int displayLine = 10;
          bool hasData = false;
          for (int i = 0; i < rfidStationCount && displayLine < 55; i++) {
            if (rfidStations[i].isActive) {
              display.setCursor(0, displayLine);
              display.print("S");
              display.print(rfidStations[i].stationId);
              display.print(": ");
              String shortRfid = rfidStations[i].rfidId.substring(0, 8) + "...";
              display.println(shortRfid);
              displayLine += 8;
              hasData = true;
            }
          }
          
          if (!hasData) {
            display.setCursor(0, 20);
            display.println("No stations set");
          }
          
          display.setCursor(0, 55);
          display.println("Press any key");
          display.display();
          
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
            display.clearDisplay();
            display.setCursor(0, 20);
            display.print("Station ");
            display.print(selectedStationId);
            display.println(" deleted!");
            display.display();
            delay(1500);
          } else {
            display.clearDisplay();
            display.setCursor(0, 20);
            display.println("Station not found!");
            display.display();
            delay(1500);
          }
        }
        break;
        
      case 4: // Clear All
        {
          display.clearDisplay();
          display.setCursor(0, 10);
          display.println("Clear all RFID?");
          display.setCursor(0, 20);
          display.println("A: Yes  B: No");
          display.display();
          
          // Wait for confirmation
          while (true) {
            if (A()) {
              clearAllRfidStations();
              display.clearDisplay();
              display.setCursor(0, 20);
              display.println("All stations cleared!");
              display.display();
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
    tombolBoot = true;
    setup();  // Return to main menu
  } else if (B()) {
    currentMenu = MENU_MAIN;
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
          } else if (selectedItem == 4) {  // RFID Settings (item 5)
            currentMenu = MENU_RFID_SETTINGS;
          } else if (selectedItem == 5) {  // Reset Settings (item 6)
            currentMenu = MENU_RESET;
          } else {
            currentMenu = selectedItem + 1;
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
