#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>

// Pin Motor 1
#define MOTOR1_D1   5
#define MOTOR1_D2   4
#define MOTOR1_PWM  6

// Pin Motor 2
#define MOTOR2_D1   7
#define MOTOR2_D2   15
#define MOTOR2_PWM  16

// Serial communication pins - Update to match Master
#define RX_PIN 41  // Tetap sama seperti sebelumnya
#define TX_PIN 42  // Tetap sama seperti sebelumnya

// PWM settings - 12-bit resolution
#define PWM_FREQ 5000
#define PWM_RESOLUTION 12  // 12-bit PWM (0-4095)
#define PWM_CHANNEL_1 0
#define PWM_CHANNEL_2 1

#define EncoderKananPinA 40
#define EncoderKananPinB 39
#define EncoderKiriPinA 37
#define EncoderKiriPinB 38

int encKananA;
int encKananB;
int encKiriA;
int encKiriB;
    
// Variables declarations (defined in main .ino file)
extern String inputString;
extern bool stringComplete;

// Encoder Variables
extern volatile long enc_kanan;
extern volatile long enc_kiri;

// RPM Variables
extern int rpm_depan_kanan;
extern int rpm_depan_kiri; 
extern int perRotasi;  // pulses per rotation

// Timing Variables for RPM calculation
extern unsigned long milisrpm;
extern const unsigned long intervalrpm;

// Variables declarations (defined in main .ino file)
extern String inputString;
extern bool stringComplete;

// Encoder Variables
extern volatile long enc_kanan;
extern volatile long enc_kiri;

// RPM Variables
extern int rpm_depan_kanan;
extern int rpm_depan_kiri;
extern int perRotasi;  // pulses per rotation

// Timing Variables for RPM calculation
extern unsigned long milisrpm;
extern const unsigned long intervalrpm;

// PID Configuration Structure
struct PIDConfig {
  double kp;
  double ki;
  double kd;
};

// PID Data Structure - ENHANCED with Best Practices
struct PIDData {
  double error = 0.0;
  double previousError = 0.0;
  double integral = 0.0;
  double derivative = 0.0;
  double previousInput = 0.0;        // For derivative on measurement (prevents derivative kick)
  double filteredDerivative = 0.0;   // For low-pass filtered derivative
};

// Tuning Target Enum
enum TuningTarget {
  TUNE_BOTH,     // Tuning kedua motor (mode lama)
  TUNE_RIGHT,    // Hanya motor kanan
  TUNE_LEFT      // Hanya motor kiri
};

// PID Configuration
#define numOutputs 4  // Number of PID controllers (for 2 motors: kanan dan kiri)

// Error Codes
#define ERROR_PID_CALCULATION 1001
#define ERROR_OVERFLOW 1002
#define ERROR_INVALID_INPUT 1003
#define ERROR_INVALID_OUTPUT 1004

// =============================================
// BEST PRACTICE PID PARAMETERS - Industry Standards
// =============================================

// =============================================
// SAFE MODE TOGGLE - Set to true for conservative PID behavior
// =============================================
const bool ENABLE_SAFE_MODE = true;  // 🔴 AKTIFKAN untuk motor test yang macet-macet

// 1. DERIVATIVE FILTERING (Low-pass filter to reduce noise)
// Reference: NI White Paper - "Derivative action is sensitive to noise"
const float DERIVATIVE_FILTER_N = 10.0;        // Filter coefficient (typical: 5-20)
const float DERIVATIVE_FILTER_ALPHA = 0.1;     // Will be calculated: alpha = 1/(1 + N)

// 2. SETPOINT WEIGHTING (2-DOF PID - reduces overshoot)
// Reference: Wikipedia PID - "Setpoint weighting adds adjustable factors"
const float SETPOINT_WEIGHT_P_FAST = 0.5;     // 🚀 High weight untuk fast rise (0→target)
const float SETPOINT_WEIGHT_P_SLOW = 0.3;     // 🎯 Low weight untuk near target (reduce overshoot)
const float SETPOINT_WEIGHT_TRANSITION = 0.7; // Transition at 70% of error (30% dari target)
const float SETPOINT_WEIGHT_D = 0.0;          // c = 0 eliminates derivative kick on setpoint change

// 3. ANTI-WINDUP (Back-calculation method)
// Reference: Åström & Hägglund - "Prevents integral buildup during saturation"
const float ANTI_WINDUP_TRACKING_TIME_RATIO = 1.0;  // 🔧 INCREASED to 1.0 untuk less aggressive

// 4. ADAPTIVE SAMPLING RATE
// Reference: Control Theory - "Sampling rate should be 10-20x faster than system dynamics"
struct AdaptiveSamplingConfig {
  unsigned long minInterval = 50;    // 20 Hz for fast transient (high RPM difference)
  unsigned long maxInterval = 200;   // 5 Hz for steady-state (near target)
  unsigned long currentInterval = 100;  // Current adaptive interval
  int lastRPMDifference = 0;         // Track last RPM error
};

// 5. PERFORMANCE METRICS (ISE, IAE, ITAE - Standard control theory)
// Reference: "Optimal Control" - Standard performance indices
struct PerformanceMetrics {
  float ISE = 0.0;    // Integral Square Error - penalizes large errors heavily
  float IAE = 0.0;    // Integral Absolute Error - balanced penalty
  float ITAE = 0.0;   // Integral Time-weighted Absolute Error - penalizes persistent errors
  float peakError = 0.0;     // Maximum error during test
  float settlingTime = 0.0;  // Time to reach ±2% of target
  float steadyStateError = 0.0;  // Final error after settling
  int sampleCount = 0;       // Number of samples collected
};


// RPM Configuration
#define minrpm -90
#define zerorpm 0
#define maxrpm 90    // RPM maksimal adalah 90

// PWM Configuration for PID - 12-bit PWM (0-4095)
#define pwm_zero 0
#define pwm_min -4000  // PWM minimum (reverse direction)
#define pwm_max 4000   // PWM maximum (forward direction)

// External declarations only
extern int leftSpeed;
extern int rightSpeed;

// Adaptive Sampling Rate - extern declaration
extern AdaptiveSamplingConfig adaptiveSampling;

// Performance Metrics - extern declaration
extern PerformanceMetrics performanceMetricsRight;
extern PerformanceMetrics performanceMetricsLeft;

// PID Data array definition - extern declaration only
extern PIDData pidData[numOutputs];
extern PIDConfig pidConfig;  // Global PID configuration (backward compatibility)

// Individual Motor PID Configuration
extern PIDConfig pidConfigRight;  // PID configuration for right motor
extern PIDConfig pidConfigLeft;   // PID configuration for left motor

Preferences preferences;    // Preferences object for storing PID parameters

// Motor PWM variables - extern declarations
extern int pwmKanan;
extern int pwmKiri;

// RPM variables - extern declarations
extern int perRotasi;  // pulses per rotation
extern unsigned long milisrpm;
extern const unsigned long intervalrpm;

// Persistent RPM Control Variables
extern float targetRpmKanan;  // Target RPM untuk motor kanan
extern float targetRpmKiri;   // Target RPM untuk motor kiri
extern bool rpmControlActive; // Flag untuk kontrol RPM berkelanjutan

// Serial variables - extern declarations
extern String inputString;
extern bool stringComplete;

// Encoder Variables - extern declarations
extern volatile long enc_kanan;
extern volatile long enc_kiri;

// Legacy PWM command variables - extern declarations
extern int leftSpeed;
extern int rightSpeed;

// Function declarations
void logError(int errorCode, const char* message); // PID RPM function
void loadPIDParameters();           // Load PID from preferences
void savePIDParametersRight();      // Save Right Motor PID to preferences
void savePIDParametersLeft();       // Save Left Motor PID to preferences
void handleSerialCommand(String command);  // Handle serial commands
void printHelp();                   // Print help information
void stopAllMotors();              // Stop all motors
void setupMotorPins();             // Setup motor pins
void setupEncoders();              // Setup encoder interrupts
void pembacaan_RPM();              // Read RPM from encoders
void serialEvent();                // Handle serial input
void processCommand(String command); // Process complete commands
bool parseCommand(String command, int &leftSpeed, int &rightSpeed); // Parse legacy L<val>R<val> commands
void sendPIDToMaster();

// Auto-tuner function declarations
void handleAutoTuning();              // Handle auto-tuning state machine
void startAutoTuning();               // Start auto-tuning process
void startAutoTuningRight();          // Start auto-tuning for right motor only
void startAutoTuningLeft();           // Start auto-tuning for left motor only
void startAutoTuningGeneric(TuningTarget target);  // Generic tuning function
void cancelAutoTuning();              // Cancel auto-tuning process
bool isTuningActive();                // Check if tuning is active
int getTuningProgress();              // Get tuning progress percentage

#endif // CONFIG_H