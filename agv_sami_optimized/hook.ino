/*
  HOOK.INO - Hook Control System
  
  This file contains all hook-related functions for AGV SAMI:
  - Hook position control (up/down)
  - Hook status monitoring
  - Limit switch handling
  - Hook safety features
*/

#include "config.h"
#include "types.h"

// Hook status tracking
HookStatus currentHookStatus = HOOK_STATUS_UNKNOWN;
unsigned long hookOperationStartTime = 0;
bool hookOperationInProgress = false;
unsigned long lastHookStatusUpdate = 0;

// ==================== HOOK CONTROL FUNCTIONS ====================

void setHookUp() {
  if (!isHookSafe()) {
    logError(ERROR_HOOK_SAFETY, "Hook operation not safe");
    return;
  }
  
  // Check if already at top
  if (digitalRead(HOOK_LIMIT_UP) == LOW) {
    currentHookStatus = HOOK_STATUS_UP;
    return;
  }
  
  // Start moving up
  digitalWrite(HOOK_RELAY, systemConfig.invertHook ? LOW : HIGH);
  hookOperationStartTime = millis();
  hookOperationInProgress = true;
  currentHookStatus = HOOK_STATUS_MOVING_UP;
  
  Serial.println("Hook: Moving up");
}

void setHookDown() {
  if (!isHookSafe()) {
    logError(ERROR_HOOK_SAFETY, "Hook operation not safe");
    return;
  }
  
  // Check if already at bottom
  if (digitalRead(HOOK_LIMIT_DOWN) == LOW) {
    currentHookStatus = HOOK_STATUS_DOWN;
    return;
  }
  
  // Start moving down
  digitalWrite(HOOK_RELAY, systemConfig.invertHook ? HIGH : LOW);
  hookOperationStartTime = millis();
  hookOperationInProgress = true;
  currentHookStatus = HOOK_STATUS_MOVING_DOWN;
  
  Serial.println("Hook: Moving down");
}

void toggleHook() {
  if (currentHookStatus == HOOK_STATUS_UP || currentHookStatus == HOOK_STATUS_MOVING_UP) {
    setHookDown();
  } else {
    setHookUp();
  }
}

void updateHookStatus() {
  if (!hookOperationInProgress) {
    return;
  }
  
  unsigned long currentTime = millis();
  
  // Check for timeout
  if (currentTime - hookOperationStartTime > HOOK_TIMEOUT_MS) {
    digitalWrite(HOOK_RELAY, LOW);
    hookOperationInProgress = false;
    currentHookStatus = HOOK_STATUS_ERROR;
    logError(ERROR_HOOK_TIMEOUT, "Hook operation timeout");
    return;
  }
  
  // Check limit switches
  bool limitUpPressed = (digitalRead(HOOK_LIMIT_UP) == LOW);
  bool limitDownPressed = (digitalRead(HOOK_LIMIT_DOWN) == LOW);
  
  if (currentHookStatus == HOOK_STATUS_MOVING_UP && limitUpPressed) {
    digitalWrite(HOOK_RELAY, LOW);
    hookOperationInProgress = false;
    currentHookStatus = HOOK_STATUS_UP;
    Serial.println("Hook: Reached top position");
    
    // Delay at limit switch
    delay(HOOK_DELAY_AT_LIMIT);
    
  } else if (currentHookStatus == HOOK_STATUS_MOVING_DOWN && limitDownPressed) {
    digitalWrite(HOOK_RELAY, LOW);
    hookOperationInProgress = false;
    currentHookStatus = HOOK_STATUS_DOWN;
    Serial.println("Hook: Reached bottom position");
    
    // Delay at limit switch
    delay(HOOK_DELAY_AT_LIMIT);
  }
  
  // Update status periodically
  if (currentTime - lastHookStatusUpdate > HOOK_STATUS_UPDATE_INTERVAL) {
    lastHookStatusUpdate = currentTime;
    
    // Additional status checks can be added here
    if (limitUpPressed && limitDownPressed) {
      // Both limit switches pressed - error condition
      emergencyStopHook();
      logError(ERROR_HOOK_LIMIT_SWITCH, "Both limit switches activated");
    }
  }
}

// ==================== HOOK SAFETY FUNCTIONS ====================

bool isHookSafe() {
  // Check if hook operation is safe
  // Return false if there are obstacles or safety concerns
  
  // Check for obstacles when moving hook down
  if (systemState.currentHookState == HOOK_MOVING_UP || systemState.currentHookState == HOOK_MOVING_DOWN) {
    // Add safety checks here if needed
    return true;
  }
  
  return !sensorData.obstacleDetected && systemState.isInitialized;
}

void emergencyStopHook() {
  digitalWrite(HOOK_RELAY, LOW);
  hookOperationInProgress = false;
  currentHookStatus = HOOK_STATUS_ERROR;
  
  // Log emergency stop
  logError(ERROR_HOOK_EMERGENCY, "Emergency hook stop activated");
  
  // Play error sound
  music("error");
}

// ==================== HOOK DIAGNOSTIC FUNCTIONS ====================

bool testHookSystem() {
  Serial.println("=== Hook System Test ===");
  
  // Test limit switches
  bool limitUpState = digitalRead(HOOK_LIMIT_UP);
  bool limitDownState = digitalRead(HOOK_LIMIT_DOWN);
  
  Serial.print("Limit Switch Up: ");
  Serial.println(limitUpState ? "Released" : "Pressed");
  Serial.print("Limit Switch Down: ");
  Serial.println(limitDownState ? "Released" : "Pressed");
  
  // Test relay
  Serial.println("Testing hook relay...");
  digitalWrite(HOOK_RELAY, HIGH);
  delay(1000);
  digitalWrite(HOOK_RELAY, LOW);
  
  Serial.println("Hook system test completed");
  return true;
}

String getHookStatusString() {
  switch (currentHookStatus) {
    case HOOK_STATUS_UP: return "UP";
    case HOOK_STATUS_DOWN: return "DOWN";
    case HOOK_STATUS_MOVING_UP: return "MOVING UP";
    case HOOK_STATUS_MOVING_DOWN: return "MOVING DOWN";
    case HOOK_STATUS_UNKNOWN: return "UNKNOWN";
    case HOOK_STATUS_ERROR: return "ERROR";
    default: return "INVALID";
  }
}

void displayHookStatus() {
  Serial.println("=== Hook Status ===");
  Serial.print("Current Status: ");
  Serial.println(getHookStatusString());
  Serial.print("Operation in Progress: ");
  Serial.println(hookOperationInProgress ? "Yes" : "No");
  
  if (hookOperationInProgress) {
    unsigned long elapsed = millis() - hookOperationStartTime;
    Serial.print("Operation Time: ");
    Serial.print(elapsed);
    Serial.println(" ms");
  }
  
  Serial.print("Limit Up: ");
  Serial.println(digitalRead(HOOK_LIMIT_UP) == LOW ? "Pressed" : "Released");
  Serial.print("Limit Down: ");
  Serial.println(digitalRead(HOOK_LIMIT_DOWN) == LOW ? "Pressed" : "Released");
}

// ==================== HOOK INITIALIZATION ====================

void setupHook() {
  // Initialize hook pins
  pinMode(HOOK_RELAY, OUTPUT);
  pinMode(HOOK_LIMIT_UP, INPUT_PULLUP);
  pinMode(HOOK_LIMIT_DOWN, INPUT_PULLUP);
  
  // Set initial hook relay state
  digitalWrite(HOOK_RELAY, LOW);
  
  // Initialize hook status based on limit switches
  if (digitalRead(HOOK_LIMIT_UP) == HIGH) {
    systemState.hookMovementStatus = HOOK_UP;
  } else if (digitalRead(HOOK_LIMIT_DOWN) == HIGH) {
    systemState.hookMovementStatus = HOOK_DOWN;
  } else {
    systemState.hookMovementStatus = HOOK_IDLE; // Or some other default state
  }
  
  DEBUG_PRINTF("Hook system initialized. Initial status: %s\n", getHookStatusString().c_str());
}

// ==================== COMPATIBILITY FUNCTIONS ====================

// These functions maintain compatibility with existing code
void hookUp() {
  setHookUp();
}

void hookDown() {
  setHookDown();
}

void hookToggle() {
  toggleHook();
}

int getHookStatus() {
  return (int)currentHookStatus;
}