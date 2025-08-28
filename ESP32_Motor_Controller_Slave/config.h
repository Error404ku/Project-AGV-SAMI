#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>

// Pin Motor 1
#define MOTOR1_D1   4
#define MOTOR1_D2   5
#define MOTOR1_PWM  6

// Pin Motor 2
#define MOTOR2_D1   7
#define MOTOR2_D2   15
#define MOTOR2_PWM  16

// Serial communication pins - Update to match Master
#define RX_PIN 41  // Tetap sama seperti sebelumnya
#define TX_PIN 42  // Tetap sama seperti sebelumnya

// PWM settings - 12-bit resolution
#define PWM_FREQ 1000
#define PWM_RESOLUTION 12  // 12-bit PWM (0-4095)
#define PWM_CHANNEL_1 0
#define PWM_CHANNEL_2 1

#define EncoderKananPin 39 //45
#define EncoderKiriPin 37

// Variables declarations (defined in main .ino file)
extern String inputString;
extern bool stringComplete;

// Encoder Variables
extern volatile long enc_kanan;
extern volatile long enc_kiri;

// RPM Variables
extern float rpm_depan_kanan;
extern float rpm_depan_kiri; 
extern int perRotasi;  // pulses per rotation

// Timing Variables for RPM calculation
extern unsigned long milisrpm;
extern const unsigned long intervalrpm;

// PID Configuration Structure
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

#define EncoderKananPin 39 //45
#define EncoderKiriPin 37

// Variables declarations (defined in main .ino file)
extern String inputString;
extern bool stringComplete;

// Encoder Variables
extern volatile long enc_kanan;
extern volatile long enc_kiri;

// RPM Variables
extern float rpm_depan_kanan;
extern float rpm_depan_kiri; 
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

// PID Data Structure
struct PIDData {
  double error = 0.0;
  double previousError = 0.0;
  double integral = 0.0;
  double derivative = 0.0;
};

// PID Configuration
#define numOutputs 2  // Number of PID controllers (for 2 motors) // Preferences object

// Error Codes
#define ERROR_PID_CALCULATION 1001

// RPM Configuration
#define minrpm 0
#define maxrpm 90    // RPM maksimal adalah 90

// PWM Configuration for PID - 12-bit PWM (0-4095)
#define pwm_zero 0
#define pwm_min -4095  // PWM minimum (reverse direction)
#define pwm_max 4095   // PWM maximum (forward direction)

// External declarations only
extern int leftSpeed;
extern int rightSpeed;

// PID Data array definition - extern declaration only
extern PIDData pidData[numOutputs];
extern PIDConfig pidConfig;  // Global PID configuration
Preferences preferences;    // Preferences object for storing PID parameters

// Motor PWM variables - extern declarations
extern int pwmKanan;
extern int pwmKiri;

// RPM variables - extern declarations
extern float rpm_depan_kanan;
extern float rpm_depan_kiri;
extern int perRotasi;  // pulses per rotation
extern unsigned long milisrpm;
extern const unsigned long intervalrpm;

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
void logError(int errorCode, const char* message);
void rpmMotor(int rpm1, int rpm2);  // PID RPM function
void loadPIDParameters();           // Load PID from preferences
void savePIDParameters();           // Save PID to preferences
void resetPIDParameters();          // Reset PID to defaults
void handleSerialCommand(String command);  // Handle serial commands
void printHelp();                   // Print help information
void stopAllMotors();              // Stop all motors
void setupMotorPins();             // Setup motor pins
void setupEncoders();              // Setup encoder interrupts
void pembacaan_RPM();              // Read RPM from encoders
void serialEvent();                // Handle serial input
void processCommand(String command); // Process complete commands
bool parseCommand(String command, int &leftSpeed, int &rightSpeed); // Parse legacy L<val>R<val> commands

#endif // CONFIG_H