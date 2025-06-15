#include "menu.h"

// Menu states
#define MENU_MAIN 0
#define MENU_MOTOR_TEST 2
#define MENU_PID_SETTINGS 3
#define MENU_TARGET_SETTINGS 4
#define MENU_AGV_MODE 1
#define MENU_RESET 5

// Menu variables
int currentMenu = MENU_MAIN;
int selectedItem = 0;
int maxItems = 5;
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

  manualTargetCount = preferences.getInt("manualTargetCount", 2);

  // Load saved target count

  // Load saved target stations
  for (int i = 0; i < manualTargetCount; i++) {
    char key[10];
    sprintf(key, "target%d", i + 1);
    targetStation[i] = preferences.getInt(key, i + 2);  // Default to 2,3,4,etc
  }
  // Load saved target source preference first
  // Load saved PID values
  tempKp = preferences.getDouble("kpLinefollower", 70.0);
  tempKi = preferences.getDouble("kiLinefollower", 0.0);
  tempKd = preferences.getDouble("kdLinefollower", 0.0);
  useAutoTarget = preferences.getBool("useAutoTarget", false);
  Serial.print("Loaded useAutoTarget: ");
  Serial.println(useAutoTarget);


  // If using computer targets, update targetStation with computer values
  if (useAutoTarget) {
    for (int i = 0; i < 4; i++) {
      targetStation[i] = targetStationFromKomputer[i];
    }
  }

  // Apply loaded values
  kpLinefollower = tempKp;
  kiLinefollower = tempKi;
  kdLinefollower = tempKd;

  // End preferences session
  preferences.end();
}

void saveSettings() {
  preferences.begin("agv-settings", false);
  delay(50);
  // Save PID values
  preferences.putDouble("kpLinefollower", tempKp);
  preferences.putDouble("kiLinefollower", tempKi);
  preferences.putDouble("kdLinefollower", tempKd);

  // Save target count
  preferences.putInt("manualTargetCount", manualTargetCount);
  // Save target source preference first
  preferences.putBool("useAutoTarget", useAutoTarget);
  Serial.print("Saving useAutoTarget: ");
  Serial.println(useAutoTarget);
  // Save all manual targets
  for (int i = 0; i < manualTargetCount; i++) {
    char key[10];
    sprintf(key, "target%d", i + 1);
    preferences.putInt(key, targetStation[i]);
  }

  // Clear any remaining target slots
  for (int i = manualTargetCount; i < MAX_MANUAL_TARGETS; i++) {
    char key[10];
    sprintf(key, "target%d", i + 1);
    preferences.remove(key);
  }

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

  display.setCursor(0, 20);
  displayIndicator(1, selectedItem);
  display.println("2. Motor Test");

  display.setCursor(0, 30);
  displayIndicator(2, selectedItem);
  display.println("3. PID Settings");

  display.setCursor(0, 40);
  displayIndicator(3, selectedItem);
  display.println("4. Target Settings");

  display.setCursor(0, 50);
  displayIndicator(4, selectedItem);
  display.println("5. Reset Settings");
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

  display.setCursor(0, 10);
  displayIndicator(0, selectedTarget);
  display.print("Source: ");
  display.println(useAutoTarget ? "Computer" : "Manual");

  if (!useAutoTarget) {
    // Display manual targets
    for (int i = 0; i < manualTargetCount; i++) {
      display.setCursor(0, 20 + (i * 10));
      displayIndicator(i + 1, selectedTarget);
      display.print("Target ");
      display.print(i + 1);
      display.print(": ");
      display.println(targetStation[i]);
    }

    // Display add/remove target options
    display.setCursor(0, 20 + (manualTargetCount * 10));
    displayIndicator(manualTargetCount + 1, selectedTarget);
    if (manualTargetCount < MAX_MANUAL_TARGETS) {
      display.println("+ Add Target");
    } else {
      display.println("Max Targets");
    }

    // Display delete target option if there are targets
    if (manualTargetCount > 1) {
      display.setCursor(0, 20 + ((manualTargetCount + 1) * 10));
      displayIndicator(manualTargetCount + 2, selectedTarget);
      display.println("- Delete Target");
    }
  } else {
    // Display computer targets
    display.setCursor(0, 20);
    display.print("Computer Targets:");
    display.setCursor(0, 30);
    for (int i = 0; i < 4; i++) {  // Assuming computer always has 4 targets
      display.print(targetStationFromKomputer[i]);
      if (i < 3) display.print(",");
    }
  }

  displayMenuFooter("B: Save | X: Cancel");
}

void displayResetMenu() {
  displayMenuHeader("Reset Settings");
  display.println("Press A to confirm");
  display.println("Press B to cancel");
  display.println("");
  display.println("This will reset:");
  display.println("- PID Settings");
  display.println("- Target Settings");
  display.println("- Button Calibration");
  display.display();
}

void handleMotorTest() {
  if (UP()) {
    pwmMotor(baseSpeed, baseSpeed);
  } else if (DOWN()) {
    pwmMotor(-baseSpeed, -baseSpeed);
  } else if (LEFT()) {
    pwmMotor(-baseSpeed, baseSpeed);
  } else if (RIGHT()) {
    pwmMotor(baseSpeed, -baseSpeed);
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
  if (UP()) {
    int maxSelection = useAutoTarget ? 1 : (manualTargetCount > 1 ? manualTargetCount + 3 : manualTargetCount + 2);
    selectedTarget = (selectedTarget - 1 + maxSelection) % maxSelection;
  } else if (DOWN()) {
    int maxSelection = useAutoTarget ? 1 : (manualTargetCount > 1 ? manualTargetCount + 3 : manualTargetCount + 2);
    selectedTarget = (selectedTarget + 1) % maxSelection;
  } else if (RIGHT()) {
    if (selectedTarget == 0) {
      // Toggle between computer and manual
      useAutoTarget = !useAutoTarget;
      Serial.print("Toggled useAutoTarget to: ");
      Serial.println(useAutoTarget);
      selectedTarget = 0;  // Reset selection when switching modes
      if (useAutoTarget) {
        // Update targetStation with computer values when switching to computer mode
        for (int i = 0; i < 4; i++) {
          targetStation[i] = targetStationFromKomputer[i];
        }
      }
      // saveSettings();  // Save source preference immediately
    } else if (!useAutoTarget) {
      if (selectedTarget <= manualTargetCount) {
        // Increment selected target
        targetStation[selectedTarget - 1]++;
        // saveSettings();  // Save immediately after changing a target value
      } else if (selectedTarget == manualTargetCount + 1 && manualTargetCount < MAX_MANUAL_TARGETS) {
        // Add new target
        manualTargetCount++;
        targetStation[manualTargetCount - 1] = 0;
        selectedTarget = manualTargetCount;  // Select the newly added target
        // saveSettings();                      // Save immediately after adding a new target
      }
    }
  } else if (LEFT()) {
    if (selectedTarget == 0) {
      // Toggle between computer and manual
      useAutoTarget = !useAutoTarget;
      Serial.print("Toggled useAutoTarget to: ");
      Serial.println(useAutoTarget);
      selectedTarget = 0;  // Reset selection when switching modes
      if (useAutoTarget) {
        // Update targetStation with computer values when switching to computer mode
        for (int i = 0; i < 4; i++) {
          targetStation[i] = targetStationFromKomputer[i];
        }
      }
      // saveSettings();  // Save source preference immediately
    } else if (!useAutoTarget) {
      if (selectedTarget <= manualTargetCount) {
        // Decrement selected target
        targetStation[selectedTarget - 1] = max(0, targetStation[selectedTarget - 1] - 1);
        // saveSettings();  // Save immediately after changing a target value
      } else if (selectedTarget == manualTargetCount + 2 && manualTargetCount > 1) {
        // Delete the last target
        manualTargetCount--;
        selectedTarget = min(selectedTarget, manualTargetCount + 1);  // Adjust selection
        // saveSettings();                                               // Save immediately after deleting a target
      }
    }
  } else if (B()) {
    if (useAutoTarget) {
      // Copy all computer targets
      for (int i = 0; i < 4; i++) {
        targetStation[i] = targetStationFromKomputer[i];
      }
    }
    saveSettings();
    currentMenu = MENU_MAIN;
  } else if (X()) {
    // Batal simpan, kembalikan nilai manualTargetCount dan targetStation
    preferences.begin("agv-settings", false);
    manualTargetCount = preferences.getInt("manualTargetCount", 2);
    for (int i = 0; i < manualTargetCount; i++) {
      char key[10];
      sprintf(key, "target%d", i + 1);
      targetStation[i] = preferences.getInt(key, i + 2);
    }
    useAutoTarget = preferences.getBool("useAutoTarget", false);
    preferences.end();

    currentMenu = MENU_MAIN;
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

    // Reset target stations to defaults
    manualTargetCount = 2;
    for (int i = 0; i < manualTargetCount; i++) {
      targetStation[i] = i + 2;
    }

    // Save default values
    saveSettings();

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
