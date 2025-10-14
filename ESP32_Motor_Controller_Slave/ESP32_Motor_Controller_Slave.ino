#include "config.h"

// Variable definitions (not extern - these are the actual variables)
PIDData pidData[numOutputs];
PIDConfig pidConfig;  // Global PID configuration (backward compatibility)

// Individual Motor PID Configuration
PIDConfig pidConfigRight;  // PID configuration for right motor
PIDConfig pidConfigLeft;   // PID configuration for left motor

// Motor PWM variables
int pwmKanan = 0;
int pwmKiri = 0;

// RPM variables
int rpm_depan_kanan = 0;
int rpm_depan_kiri = 0;
int perRotasi = 1656;  // pulses per rotation
unsigned long milisrpm = 0;
// Interval RPM untuk pembacaan yang lebih stabil (was 100ms, now 500ms)
const unsigned long intervalrpm = 500;

// Serial variables  
String inputString = "";
bool stringComplete = false;

// Encoder Variables
volatile long enc_kanan = 0;
volatile long enc_kiri = 0;

// Legacy PWM command variables
int leftSpeed = 0;
int rightSpeed = 0;

// Error logging function
void logError(int errorCode, const char* message) {
  Serial.print("[ERROR ");
  Serial.print(errorCode);
  Serial.print("] ");
  Serial.println(message);
}

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  // Initialize Serial1 for communication with ESP32 Master (AGV_SAMI)  
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  loadPIDParameters();
  sendPIDToMaster();
  
  // Setup motor pins
  setupMotorPins();
  
  // Setup encoder interrupts
  setupEncoders();
  
  // Send current PID parameters to AGV_SAMI for synchronization
  sendPIDToMaster();
  
  Serial.println("ESP32 Motor Controller Ready");
  Serial.println("Listening for commands from Master ESP32 via Serial...");
  Serial.println("Available commands: PIDRIGHT<kp>,<ki>,<kd>, PIDLEFT<kp>,<ki>,<kd>, RPM<r1>,<r2>, STATUS, STOP, L<val>R<val>");
  Serial.println();
  Serial.println("=== AUTO-TUNER PID TERSEDIA ===");
  Serial.println("Ketik 'tune' untuk memulai auto-tuning PID");
  Serial.println("Ketik 'help' untuk melihat semua command auto-tuner");
  Serial.printf("PID Right Motor: Kp=%.4f, Ki=%.4f, Kd=%.4f\n", pidConfigRight.kp, pidConfigRight.ki, pidConfigRight.kd);
  Serial.printf("PID Left Motor: Kp=%.4f, Ki=%.4f, Kd=%.4f\n", pidConfigLeft.kp, pidConfigLeft.ki, pidConfigLeft.kd);
  Serial.printf("PID General (Auto-tuner): Kp=%.4f, Ki=%.4f, Kd=%.4f\n", pidConfig.kp, pidConfig.ki, pidConfig.kd);
}

void loop() {
  // Handle auto-tuning state machine (jika sedang aktif)
  handleAutoTuning();
  
  // Check for incoming serial data from Master ESP32
  if (Serial1.available()) {  // Ubah kembali ke Serial untuk komunikasi dengan Master
    serialEvent();
  }
  
  // Process complete command
  if (stringComplete) {
    processCommand(inputString);
    inputString = "";
    stringComplete = false;
  }
  // rpmMotor(-20,-20);
  // Read RPM from encoders
  // setMotorSpeed(2, -1000);   // Motor 2 = Kiri
  // rpmMotor(20, 0); 
  pembacaan_RPM();
}