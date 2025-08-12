// ===================================================================
//                    MEMORY OPTIMIZATION TIPS
// ===================================================================

// 1. String Management Best Practices
void goodStringPractice() {
    int value = 123; // Example value
    
    // ❌ BAD - creates temporary String objects
    String result = "Status: " + String(value) + " at " + String(millis());
    
    // ✅ GOOD - use char arrays with snprintf
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Status: %d at %lu", value, millis());
}

// 2. JSON Memory Management
void handleHttpRequest() {
    DynamicJsonDocument doc(1024); // Fixed size
    
    // Process JSON
    doc["status"] = "ok";
    
    // ✅ IMPORTANT: Clear when done
    doc.clear();
    doc.shrinkToFit();
}

// 3. WiFi Buffer Management
void cleanupWifiBuffers() {
    WiFi.disconnect();
    delay(100);
    WiFi.reconnect();
    // This helps clear internal buffers
}

// 4. Periodic Memory Cleanup
void performMemoryCleanup() {
    static uint32_t lastCleanup = 0;
    
    if (millis() - lastCleanup > 300000) { // Every 5 minutes
        // Force garbage collection
        ESP.restart(); // Only if really needed
        lastCleanup = millis();
    }
}

// 5. Memory-Efficient Display Updates
void efficientDisplayUpdate() {
    int value = 456; // Example value
    
    // ❌ BAD - creates String objects
    lcd.print(String("Status: ") + String(value));
    
    // ✅ GOOD - direct printing
    lcd.print("Status: ");
    lcd.print(value);
}
