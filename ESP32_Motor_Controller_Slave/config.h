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
float rpm_depan_kanan = 0;
float rpm_depan_kiri = 0;
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

int leftSpeed = 0;
int rightSpeed = 0;

// Function declarations
void logError(int errorCode, const char* message);

#endif // CONFIG_H 