/*
  BUTTONS.INO - Button Input Processing
  
  This file contains all button-related functions for AGV SAMI:
  - Button reading and debouncing
  - Button action processing
  - Menu navigation
*/

// Button state tracking
const int NUM_BUTTONS = 7;
bool buttonState[NUM_BUTTONS] = {false};
bool lastButtonState[NUM_BUTTONS] = {false};
unsigned long lastButtonDebounceTime[NUM_BUTTONS] = {0};
unsigned long buttonPressStartTime[NUM_BUTTONS] = {0};
bool buttonLongPress[NUM_BUTTONS] = {false};

// Button indices
#define BTN_UP 0
#define BTN_DOWN 1
#define BTN_LEFT 2
#define BTN_RIGHT 3
#define BTN_SELECT 4
#define BTN_START 5
#define BTN_STOP 6

// ==================== BUTTON PROCESSING ====================

void loopButtons() {
  // Read and process all buttons
  readButton(BTN_UP, BUTTON_UP_PIN);
  readButton(BTN_DOWN, BUTTON_DOWN_PIN);
  readButton(BTN_LEFT, BUTTON_LEFT_PIN);
  readButton(BTN_RIGHT, BUTTON_RIGHT_PIN);
  readButton(BTN_SELECT, BUTTON_SELECT_PIN);
  readButton(BTN_START, BUTTON_START_PIN);
  readButton(BTN_STOP, BUTTON_STOP_PIN);
}

void readButton(int buttonIndex, int buttonPin) {
  // Read the button state
  bool reading = digitalRead(buttonPin) == HIGH;
  
  // Check if the button state has changed
  if (reading != lastButtonState[buttonIndex]) {
    // Reset debounce timer
    lastButtonDebounceTime[buttonIndex] = millis();
  }
  
  // Check if debounce time has passed
  if ((millis() - lastButtonDebounceTime[buttonIndex]) > BUTTON_DEBOUNCE_DELAY) {
    // If the button state has changed
    if (reading != buttonState[buttonIndex]) {
      buttonState[buttonIndex] = reading;
      
      // Button pressed
      if (buttonState[buttonIndex]) {
        buttonPressStartTime[buttonIndex] = millis();
        buttonLongPress[buttonIndex] = false;
        DEBUG_PRINTF("Button %d pressed\n", buttonIndex);
      } 
      // Button released
      else {
        // Check if it was a short press (not a long press)
        if (!buttonLongPress[buttonIndex]) {
          handleButtonPress(buttonIndex, false);
        }
        DEBUG_PRINTF("Button %d released\n", buttonIndex);
      }
    }
    
    // Check for long press while button is held down
    if (buttonState[buttonIndex] && !buttonLongPress[buttonIndex]) {
      if ((millis() - buttonPressStartTime[buttonIndex]) > BUTTON_HOLD_DURATION) {
        buttonLongPress[buttonIndex] = true;
        handleButtonPress(buttonIndex, true);
        DEBUG_PRINTF("Button %d long press\n", buttonIndex);
      }
    }
  }
  
  // Save the current reading for next comparison
  lastButtonState[buttonIndex] = reading;
}

void handleButtonPress(int buttonIndex, bool isLongPress) {
  // Handle button press based on current mode
  if (systemState.isAgvMode) {
    // In AGV mode, buttons control the AGV directly
    handleAGVModeButtons(buttonIndex, isLongPress);
  } else {
    // In menu mode, buttons navigate the menu
    handleMenuModeButtons(buttonIndex, isLongPress);
  }
}

void handleAGVModeButtons(int buttonIndex, bool isLongPress) {
  switch (buttonIndex) {
    case BTN_UP:
      // Forward movement
      if (systemState.currentMovement != MOVEMENT_FORWARD) {
        moveForward(systemConfig.baseSpeed);
      }
      break;
      
    case BTN_DOWN:
      // Backward movement
      if (systemState.currentMovement != MOVEMENT_BACKWARD) {
        moveBackward(systemConfig.baseSpeed);
      }
      break;
      
    case BTN_LEFT:
      // Turn left
      if (systemState.currentMovement != MOVEMENT_LEFT) {
        turnLeft(systemConfig.baseSpeed);
      }
      break;
      
    case BTN_RIGHT:
      // Turn right
      if (systemState.currentMovement != MOVEMENT_RIGHT) {
        turnRight(systemConfig.baseSpeed);
      }
      break;
      
    case BTN_START:
      // Toggle hook
      toggleHook();
      break;
      
    case BTN_STOP:
      if (isLongPress) {
        // Long press: Switch to menu mode
        systemState.isAgvMode = false;
        stopMovement();
        displayMessage("Entering Menu", "", 1000);
      } else {
        // Short press: Stop movement
        stopMovement();
      }
      break;
  }
}

void handleMenuModeButtons(int buttonIndex, bool isLongPress) {
  switch (buttonIndex) {
    case BTN_UP:
      // Navigate up in menu
      navigateMenu(MENU_UP);
      break;
      
    case BTN_DOWN:
      // Navigate down in menu
      navigateMenu(MENU_DOWN);
      break;
      
    case BTN_LEFT:
      // Navigate back in menu
      navigateMenu(MENU_BACK);
      break;
      
    case BTN_RIGHT:
    case BTN_START:
      // Select menu item
      navigateMenu(MENU_SELECT);
      break;
      
    case BTN_STOP:
      if (isLongPress) {
        // Long press: Switch to AGV mode
        systemState.isAgvMode = true;
        displayMessage("Entering AGV Mode", "", 1000);
      } else {
        // Short press: Go back in menu
        navigateMenu(MENU_BACK);
      }
      break;
  }
}

// ==================== MENU NAVIGATION ====================

void navigateMenu(MenuAction action) {
  // Handle menu navigation based on current menu state
  switch (action) {
    case MENU_UP:
      if (selectedItem > 0) {
        selectedItem--;
      } else {
        selectedItem = maxItems - 1;
      }
      menuNeedsRefresh = true;
      break;
      
    case MENU_DOWN:
      if (selectedItem < maxItems - 1) {
        selectedItem++;
      } else {
        selectedItem = 0;
      }
      menuNeedsRefresh = true;
      break;
      
    case MENU_SELECT:
      handleMenuSelection();
      menuNeedsRefresh = true;
      break;
      
    case MENU_BACK:
      // Go back to main menu
      if (systemState.currentMenu != MENU_MAIN) {
        systemState.currentMenu = MENU_MAIN;
        selectedItem = 0;
      }
      menuNeedsRefresh = true;
      break;
  }
}

void handleMenuSelection() {
  // Handle menu item selection based on current menu state
  switch (systemState.currentMenu) {
    case MENU_MAIN:
      // Main menu selection
      switch (selectedItem) {
        case 0: // Mode AGV
          systemState.currentMenu = MENU_AGV_MODE;
          break;
        case 1: // Setup Stasiun
          systemState.currentMenu = MENU_STATION_SETUP;
          break;
        case 2: // Setup RFID
          systemState.currentMenu = MENU_RFID_SETUP;
          break;
        case 3: // Status Sensor
          systemState.currentMenu = MENU_SENSOR_STATUS;
          break;
        case 4: // Info Sistem
          systemState.currentMenu = MENU_SYSTEM_INFO;
          break;
        case 5: // Pengaturan
          systemState.currentMenu = MENU_SETTINGS;
          break;
      }
      selectedItem = 0;
      break;
      
    case MENU_AGV_MODE:
      // AGV mode selection
      switch (selectedItem) {
        case 0: // Manual mode
          systemState.currentMode = MODE_MANUAL;
          systemState.isAgvMode = true;
          displayMessage("Manual Mode", "Activated", 1000);
          break;
        case 1: // Line follow mode
          systemState.currentMode = MODE_LINE_FOLLOW;
          systemState.isAgvMode = true;
          displayMessage("Line Follow Mode", "Activated", 1000);
          break;
        case 2: // Station mode
          systemState.currentMode = MODE_STATION;
          systemState.isAgvMode = true;
          displayMessage("Station Mode", "Activated", 1000);
          break;
      }
      break;
      
    // Handle other menu selections
    // ...
  }
}

// ==================== UTILITY FUNCTIONS ====================

String getModeString() {
  switch (systemState.currentMode) {
    case MODE_IDLE: return "IDLE";
    case MODE_MANUAL: return "MANUAL";
    case MODE_LINE_FOLLOW: return "LINE";
    case MODE_STATION: return "STATION";
    default: return "UNKNOWN";
  }
}