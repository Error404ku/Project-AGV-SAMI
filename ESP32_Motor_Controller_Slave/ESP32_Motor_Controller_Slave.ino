#include "config.h"

// Variable definitions (not extern - these are the actual variables)
PIDData pidData[numOutputs];
PIDConfig pidConfig;  // Global PID configuration   // Preferences object for storing PID parameters

// Motor PWM variables
int pwmKanan = 0;
int pwmKiri = 0;

// RPM variables
float rpm_depan_kanan = 0;
float rpm_depan_kiri = 0;
int perRotasi = 892;  // pulses per rotation
unsigned long milisrpm = 0;
const unsigned long intervalrpm = 1000;

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
  
  // Setup motor pins
  setupMotorPins();
  
  // Setup encoder interrupts
  setupEncoders();
  
  // Send current PID parameters to AGV_SAMI for synchronization
  sendPIDToMaster();
  
  Serial.println("ESP32 Motor Controller Ready");
  Serial.println("Listening for commands from Master ESP32 via Serial...");
  Serial.println("Available commands: KP<val>, KI<val>, KD<val>, RPM<r1>,<r2>, STATUS, STOP, L<val>R<val>");
}

void loop() {
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
  
  // Read RPM from encoders
  pembacaan_RPM();
}