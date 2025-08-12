// ===================================================================
//                    ENHANCED DISPLAY MANAGER
// ===================================================================

#include "config.h"

// Display state management
struct DisplayState {
    bool isInitialized = false;
    bool isCorrupted = false;
    uint32_t lastUpdate = 0;
    uint32_t corruptionCount = 0;
    char lastLine0[21] = {0};
    char lastLine1[21] = {0};
    char lastLine2[21] = {0};
    char lastLine3[21] = {0};
};

static DisplayState displayState;
static SemaphoreHandle_t displayMutex = NULL;

// ===================================================================
//                    DISPLAY CORRUPTION DETECTION & RECOVERY
// ===================================================================

void initializeDisplayManager() {
    // Create display mutex if using FreeRTOS    
    Serial.println("[DISPLAY] Enhanced Display Manager initialized");
    displayState.isInitialized = true;
    displayState.corruptionCount = 0;
}

bool acquireDisplayLock(uint32_t timeout_ms = 100) {
    return true; // In standard mode, always allow access
}

void releaseDisplayLock() {
}

bool detectDisplayCorruption() {
    // Check if display is responding
    Wire.beginTransmission(0x27);
    uint8_t error = Wire.endTransmission();
    
    if (error != 0) {
        displayState.isCorrupted = true;
        displayState.corruptionCount++;
        Serial.printf("[DISPLAY] Corruption detected! I2C Error: %d, Count: %d\n", 
                     error, displayState.corruptionCount);
        return true;
    }
    
    return false;
}

bool recoverDisplay() {
    Serial.println("[DISPLAY] Attempting display recovery...");
    
    // Reinitialize I2C
    Wire.end();
    delay(100);
    Wire.begin(sdaPin, sclPin);
    delay(100);
    
    // Reinitialize LCD
    lcd.init();
    delay(50);
    lcd.backlight();
    delay(50);
    lcd.clear();
    delay(100);
    
    // Test display
    lcd.setCursor(0, 0);
    lcd.print("Display Recovery");
    delay(100);
    
    // Verify recovery
    Wire.beginTransmission(0x27);
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
        Serial.println("[DISPLAY] Recovery successful!");
        displayState.isCorrupted = false;
        
        // Clear test message
        lcd.clear();
        return true;
    } else {
        Serial.printf("[DISPLAY] Recovery failed! Error: %d\n", error);
        return false;
    }
}

// ===================================================================
//                    SAFE DISPLAY OPERATIONS
// ===================================================================

void safeDisplayWrite(int col, int row, const char* text, bool forceClear = false) {
    if (!acquireDisplayLock()) {
        Serial.println("[DISPLAY] Failed to acquire lock");
        return;
    }
    
    // Check for corruption
    if (detectDisplayCorruption()) {
        if (!recoverDisplay()) {
            releaseDisplayLock();
            return;
        }
    }
    
    // Validate parameters
    if (col < 0 || col >= 20 || row < 0 || row >= 4 || text == NULL) {
        releaseDisplayLock();
        return;
    }
    
    // Create clean buffer
    char cleanText[21];
    sanitizeDisplayText(text, cleanText, 20);
    
    // Check if content has changed
    char* lastLine = getLastLineBuffer(row);
    if (!forceClear && strcmp(lastLine, cleanText) == 0) {
        releaseDisplayLock();
        return; // No change needed
    }
    
    // Update display
    lcd.setCursor(col, row);
    lcd.print(cleanText);
    
    // Pad with spaces to clear old content
    int textLen = strlen(cleanText);
    for (int i = textLen; i < 20 - col; i++) {
        lcd.print(" ");
    }
    
    // Update last known state
    strcpy(lastLine, cleanText);
    displayState.lastUpdate = millis();
    
    releaseDisplayLock();
}

void safeDisplayClear() {
    if (!acquireDisplayLock()) return;
    
    if (detectDisplayCorruption()) {
        if (!recoverDisplay()) {
            releaseDisplayLock();
            return;
        }
    }
    
    lcd.clear();
    
    // Clear last known state
    memset(displayState.lastLine0, 0, sizeof(displayState.lastLine0));
    memset(displayState.lastLine1, 0, sizeof(displayState.lastLine1));
    memset(displayState.lastLine2, 0, sizeof(displayState.lastLine2));
    memset(displayState.lastLine3, 0, sizeof(displayState.lastLine3));
    
    releaseDisplayLock();
}

void sanitizeDisplayText(const char* input, char* output, int maxLength) {
    int len = strlen(input);
    if (len > maxLength) len = maxLength;
    
    for (int i = 0; i < len; i++) {
        char c = input[i];
        // Allow printable ASCII characters (32-126) and common extended characters
        if ((c >= 32 && c <= 126) || c == 0xDF || c == 0xB0) { // Include degree symbol
            output[i] = c;
        } else {
            output[i] = ' '; // Replace invalid characters with space
        }
    }
    output[len] = '\0';
}

char* getLastLineBuffer(int row) {
    switch (row) {
        case 0: return displayState.lastLine0;
        case 1: return displayState.lastLine1;
        case 2: return displayState.lastLine2;
        case 3: return displayState.lastLine3;
        default: return displayState.lastLine0;
    }
}

// ===================================================================
//                    ENHANCED DISPLAY FUNCTIONS
// ===================================================================

void enhancedDisplayPrint() {
    static AgvState lastDisplayedState = AGV_STATE_NULL;
    static uint32_t lastUpdate = 0;
    
    // Rate limiting - update every 100ms max
    if (millis() - lastUpdate < 100) return;
    lastUpdate = millis();
    
    // Only update if state changed
    if (currentStateAgv != lastDisplayedState) {
        safeDisplayWrite(0, 0, "AGV Mode:", true);
        
        String stateString = agvStateToString(currentStateAgv);
        safeDisplayWrite(0, 1, stateString.c_str());
        
        lastDisplayedState = currentStateAgv;
    }
    
    // Add system status on line 2
    char statusLine[21];
        snprintf(statusLine, sizeof(statusLine), "Std Mode: %.1fKB free", ESP.getFreeHeap() / 1024.0);
    safeDisplayWrite(0, 2, statusLine);
    
    // Add error status on line 3
    if (errorValue != 99) {
        snprintf(statusLine, sizeof(statusLine), "Magnet Error: %d", errorValue);
        safeDisplayWrite(0, 3, statusLine);
    } else {
        safeDisplayWrite(0, 3, "All Systems OK");
    }
}

void displaySystemInfo() {
    if (!acquireDisplayLock()) return;
    
    safeDisplayClear();
    
    // Line 0: System mode
    safeDisplayWrite(0, 0, "Mode: Standard");
    
    // Line 1: Memory info
    char memInfo[21];
    snprintf(memInfo, sizeof(memInfo), "Free: %.1fKB", ESP.getFreeHeap() / 1024.0);
    safeDisplayWrite(0, 1, memInfo);
    
    // Line 2: WiFi status
    safeDisplayWrite(0, 2, WiFi.status() == WL_CONNECTED ? "WiFi: Connected" : "WiFi: Disconnected");
    
    // Line 3: Corruption count
    char corruptInfo[21];
    snprintf(corruptInfo, sizeof(corruptInfo), "Display Errors: %d", displayState.corruptionCount);
    safeDisplayWrite(0, 3, corruptInfo);
    
    releaseDisplayLock();
}

// ===================================================================
//                    DISPLAY DIAGNOSTICS
// ===================================================================

void runDisplayDiagnostic() {
    Serial.println("\n=== DISPLAY DIAGNOSTIC ===");
    
    // Test I2C communication
    Wire.beginTransmission(0x27);
    uint8_t error = Wire.endTransmission();
    Serial.printf("I2C Communication: %s (Error: %d)\n", 
                 error == 0 ? "OK" : "FAILED", error);
    
    // Test display operations
    safeDisplayClear();
    safeDisplayWrite(0, 0, "Test Line 0");
    safeDisplayWrite(0, 1, "Test Line 1");
    safeDisplayWrite(0, 2, "Test Line 2");
    safeDisplayWrite(0, 3, "Test Line 3");
    
    delay(2000);
    
    // Test character sanitization
    safeDisplayWrite(0, 0, "ASCII: !@#$%^&*()");
    safeDisplayWrite(0, 1, "Extended: åßç∂é"); // Should be cleaned
    
    delay(2000);
    safeDisplayClear();
    
    Serial.printf("Display corruption count: %d\n", displayState.corruptionCount);
    Serial.println("Display diagnostic completed");
}
