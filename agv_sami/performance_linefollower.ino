// ===================================================================
// LINE FOLLOWER PERFORMANCE OPTIMIZATION IMPLEMENTATION
// ===================================================================

#include "performance_linefollower.h"

// Helper function untuk rate limiting
bool checkRateLimit(unsigned long& lastReadTime, unsigned long interval) {
    if (!enableSensorRateLimiting) return true;
    
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= interval) {
        lastReadTime = currentTime;
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