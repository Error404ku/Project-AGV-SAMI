// ===================================================================
//                    LIGHTWEIGHT DISPLAY FIX
// ===================================================================

#include "config.h"

// Simple display protection variables
static volatile bool displayBusy = false;
static uint32_t lastDisplayOperation = 0;
static const uint32_t DISPLAY_COOLDOWN = 50; // 50ms between operations

// ===================================================================
//                    SAFE DISPLAY FUNCTIONS
// ===================================================================

bool isDisplaySafe() {
    uint32_t now = millis();
    
    // Check cooldown period
    if (now - lastDisplayOperation < DISPLAY_COOLDOWN) {
        return false;
    }
    
    // Check if display is busy
    if (displayBusy) {
        return false;
    }
    
    return true;
}

void safeDisplayOperation(void (*operation)()) {
    if (!isDisplaySafe()) return;
    
    displayBusy = true;
    lastDisplayOperation = millis();
    
    // Execute the operation
    operation();
    
    displayBusy = false;
}

// Safe display functions
void safeLcdClear() {
    if (!isDisplaySafe()) return;
    
    displayBusy = true;
    lcd.clear();
    delay(2); // Ensure LCD processes command
    displayBusy = false;
    lastDisplayOperation = millis();
}

void safeLcdPrint(int col, int row, const String& text) {
    if (!isDisplaySafe()) return;
    
    displayBusy = true;
    
    // Sanitize text - remove non-printable characters
    String cleanText = text;
    for (int i = 0; i < cleanText.length(); i++) {
        char c = cleanText[i];
        if (c < 32 || c > 126) {
            cleanText[i] = ' '; // Replace with space
        }
    }
    
    // Trim to LCD width
    if (cleanText.length() > 20) {
        cleanText = cleanText.substring(0, 20);
    }
    
    lcd.setCursor(col, row);
    lcd.print(cleanText);
    
    // Pad with spaces to clear old content
    for (int i = cleanText.length(); i < 20 - col; i++) {
        lcd.print(" ");
    }
    
    displayBusy = false;
    lastDisplayOperation = millis();
}

void safeLcdPrint(int col, int row, const char* text) {
    safeLcdPrint(col, row, String(text));
}

// Enhanced display function - lightweight version
void lightweightDisplayPrint() {
    static AgvState lastState = AGV_STATE_NULL;
    static uint32_t lastUpdate = 0;
    
    // Rate limiting - update max every 200ms
    if (millis() - lastUpdate < 200) return;
    
    // Only update if state changed or forced update
    if (currentStateAgv != lastState) {
        safeLcdClear();
        safeLcdPrint(0, 0, "AGV Mode:");
        safeLcdPrint(0, 1, agvStateToString(currentStateAgv));
        
        lastState = currentStateAgv;
        lastUpdate = millis();
    }
}

// ===================================================================
//                    STACK USAGE MONITORING (LIGHTWEIGHT)
// ===================================================================

void lightweightStackMonitor() {
    static uint32_t lastCheck = 0;
    
    // Check every 30 seconds only
    if (millis() - lastCheck < 30000) return;
    lastCheck = millis();
    
    // Enhanced heap monitoring
    size_t freeHeap = ESP.getFreeHeap();
    size_t minFreeHeap = ESP.getMinFreeHeap();
    size_t totalHeap = ESP.getHeapSize();
    
    Serial.printf("[STACK] Free Heap: %d bytes, Min Free: %d bytes\n", freeHeap, minFreeHeap);
    
    // Enhanced safety warnings with specific thresholds
    if (minFreeHeap < 100000) { // <100KB - Critical
        Serial.println("[CRITICAL] Memory critically low! System may become unstable!");
        safeLcdClear();
        safeLcdPrint(0, 0, "MEMORY WARNING!");
        safeLcdPrint(0, 1, "Low RAM detected");
    } else if (minFreeHeap < 150000) { // <150KB - Warning
        Serial.println("[WARNING] Memory getting low! Monitor closely!");
    } else if (minFreeHeap < 200000) { // <200KB - Caution
        Serial.println("[CAUTION] Memory usage increasing. Keep monitoring.");
    } else {
        Serial.println("[OK] Memory usage is safe.");
    }
    
    // Memory trend analysis
    static size_t lastMinFree = minFreeHeap;
    if (lastMinFree > 0) {
        int32_t trend = (int32_t)minFreeHeap - (int32_t)lastMinFree;
        if (trend < -10000) { // Memory dropping fast (>10KB)
            Serial.printf("[TREND] Memory usage increasing rapidly! (%d bytes drop)\n", -trend);
        } else if (trend > 5000) { // Memory recovering
            Serial.printf("[TREND] Memory usage improving (%d bytes recovered)\n", trend);
        }
    }
    lastMinFree = minFreeHeap;
    
    // Memory fragmentation check
    size_t maxBlock = ESP.getMaxAllocHeap();
    float fragmentation = 100.0 - (float)maxBlock * 100.0 / freeHeap;
    if (fragmentation > 50.0) {
        Serial.printf("[FRAGMENTATION] High fragmentation: %.1f%%\n", fragmentation);
    }
}

// ===================================================================
//                    WATCHDOG MANAGEMENT
// ===================================================================

void resetWatchdogSafely() {
    static uint32_t lastReset = 0;
    
    // Reset watchdog every 1 second max
    if (millis() - lastReset > 1000) {
        esp_task_wdt_reset();
        lastReset = millis();
    }
}

// ===================================================================
//                    DISPLAY CORRUPTION RECOVERY
// ===================================================================

void checkDisplayHealth() {
    static uint32_t lastHealthCheck = 0;
    static int corruptionCount = 0;
    
    // Check every 10 seconds
    if (millis() - lastHealthCheck < 10000) return;
    lastHealthCheck = millis();
    
    // Test I2C communication
    Wire.beginTransmission(LCD_ADDRESS);
    uint8_t error = Wire.endTransmission();
    
    if (error != 0) {
        corruptionCount++;
        Serial.printf("[DISPLAY] I2C Error %d, Count: %d\n", error, corruptionCount);
        
        // Attempt recovery after 3 consecutive errors
        if (corruptionCount >= 3) {
            Serial.println("[DISPLAY] Attempting recovery...");
            
            // Reinitialize I2C and LCD
            Wire.end();
            delay(100);
            Wire.begin(sdaPin, sclPin);
            delay(100);
            lcd.init();
            lcd.backlight();
            
            corruptionCount = 0;
            Serial.println("[DISPLAY] Recovery attempted");
        }
    } else {
        corruptionCount = 0; // Reset count on success
    }
}

// ===================================================================
//                    INITIALIZATION
// ===================================================================

void initLightweightMonitoring() {
    displayBusy = false;
    lastDisplayOperation = 0;
    Serial.println("[MONITOR] Lightweight monitoring initialized");
}
