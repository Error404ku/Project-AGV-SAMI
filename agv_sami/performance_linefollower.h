// ===================================================================
// LINE FOLLOWER PERFORMANCE OPTIMIZATION
// ===================================================================

#ifndef PERFORMANCE_LINEFOLLOWER_H
#define PERFORMANCE_LINEFOLLOWER_H

// Rate limiting for sensor readings
#define MAGNET_READ_INTERVAL_MS 0     // Real-time reading - no rate limiting
#define ULTRASONIC_READ_INTERVAL_MS 100 // Read ultrasonic every 100ms
#define RFID_READ_INTERVAL_MS 50      // Read RFID every 50ms

// Timing variables
static unsigned long lastMagnetRead = 0;
static unsigned long lastUltrasonicRead = 0;
static unsigned long lastRfidRead = 0;

// Performance flags
static bool enableSensorRateLimiting = true;

// Function prototypes
bool shouldReadMagnet();
bool shouldReadUltrasonic();
bool shouldReadRfid();
void resetSensorTimers();

#endif