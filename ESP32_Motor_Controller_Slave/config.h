#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

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
#define PWM_FREQ 5000
#define PWM_RESOLUTION 12  // 12-bit PWM (0-4095)
#define PWM_CHANNEL_1 0
#define PWM_CHANNEL_2 1

#define EncoderKananPin 39 //45
#define EncoderKiriPin 37

String inputString = "";
bool stringComplete = false;

// Encoder Variables
volatile long enc_kanan = 0;
volatile long enc_kiri = 0;

// RPM Variables
int rpm_kanan = 0;
int rpm_kiri = 0;
int perRotasi = 892;  // pulses per rotation

// Timing Variables for RPM calculation
unsigned long milisrpm = 0;
const unsigned long intervalrpm = 1000; 

// PID Data Structure
struct PIDData {
  double error = 0.0;
  double previousError = 0.0;
  double integral = 0.0;
  double derivative = 0.0;
};

// PID Configuration
#define numOutputs 2  // Number of PID controllers (for 2 motors)
extern PIDData pidData[numOutputs];

// Error Codes
#define ERROR_PID_CALCULATION 1001

// RPM Configuration
#define minrpm 0
#define maxrpm 90    // RPM maksimal adalah 90

// PWM Configuration for PID - 12-bit PWM (0-4095)
#define pwm_zero 0
#define pwm_min -4095  // PWM minimum (reverse direction)
#define pwm_max 4095   // PWM maximum (forward direction)

// Motor PWM output variables
extern int pwmKanan;  // PWM Motor Kanan 
extern int pwmKiri;   // PWM Motor Kiri

int leftSpeed = 0;
int rightSpeed = 0;

// Function declarations
void logError(int errorCode, const char* message);
void rpmMotor(int rpm1, int rpm2);  // PID RPM function

#endif // CONFIG_H 