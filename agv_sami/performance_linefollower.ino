// ===================================================================
// LINE FOLLOWER PERFORMANCE OPTIMIZATION IMPLEMENTATION
// ===================================================================

#include "performance_linefollower.h"

// Rate limiting functions
bool shouldReadMagnet() {
    
    unsigned long currentTime = millis();
    if (currentTime - lastMagnetRead >= MAGNET_READ_INTERVAL_MS) {
        lastMagnetRead = currentTime;
        return true;
    }
    return false;
}

bool shouldReadUltrasonic() {
    unsigned long currentTime = millis();
    if (currentTime - lastUltrasonicRead >= ULTRASONIC_READ_INTERVAL_MS) {
        lastUltrasonicRead = currentTime;
        return true;
    }
    return false;
}

bool shouldReadRfid() {    
    unsigned long currentTime = millis();
    if (currentTime - lastRfidRead >= RFID_READ_INTERVAL_MS) {
        lastRfidRead = currentTime;
        return true;
    }
    return false;
}

void resetSensorTimers() {
    unsigned long currentTime = millis();
    lastMagnetRead = currentTime;
    lastUltrasonicRead = currentTime;
    lastRfidRead = currentTime;
}

// Function to toggle rate limiting
void setRateLimiting(bool enabled) {
    enableSensorRateLimiting = enabled;
}