#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>
#include <vector>

// ==================== ENUMS ====================

// Menu states
enum MenuState {
  MENU_MAIN = 0,
  MENU_AGV_MODE = 1,
  MENU_MOTOR_TEST = 2,
  MENU_PID_SETTINGS = 3,
  MENU_TARGET_SETTINGS = 4,
  MENU_RFID_SETTINGS = 5,
  MENU_MOTOR_SETTINGS = 6,
  MENU_MOTOR_INVERT = 7,
  MENU_MUSIC_SETTINGS = 8,
  MENU_MUSIC_TEST = 9,
  MENU_HOOK_TEST = 10,
  MENU_RESET = 11,
  MENU_MAGNET_CHECK = 12,
  MENU_ULTRASONIC_CHECK = 13
};

// Hook states
enum HookState {
  HOOK_IDLE,
  HOOK_MOVING_UP,
  HOOK_AT_TOP,
  HOOK_MOVING_DOWN,
  HOOK_AT_BOTTOM
};

// AGV modes
enum AgvMode {
  MODE_TERMINAL,
  MODE_WAREHOUSE,
  MODE_STATION
};

// Movement states
enum MovementState {
  MOVEMENT_STOP,
  MOVEMENT_FORWARD,
  MOVEMENT_BACKWARD,
  MOVEMENT_LEFT,
  MOVEMENT_RIGHT
};

// Error codes
enum ErrorCode {
  ERROR_SENSOR_COMMUNICATION = 1,
  ERROR_MOTOR_CONTROL = 2,
  ERROR_RFID_COMMUNICATION = 3,
  ERROR_WIFI_CONNECTION = 4,
  ERROR_LCD_COMMUNICATION = 5,
  ERROR_SYSTEM_INITIALIZATION = 6,
  ERROR_ENCODER_FAILURE = 7,
  ERROR_PID_CALCULATION = 8,
  ERROR_MEMORY_ALLOCATION = 9,
  ERROR_INVALID_CONFIGURATION = 10
};

// ==================== STRUCTURES ====================

// PID data structure
struct PIDData {
  double error;
  double integral;
  double derivative;
  double previousError;
  
  PIDData() : error(0), integral(0), derivative(0), previousError(0) {}
};

// PID parameters structure
struct PIDParams {
  double kp;
  double ki;
  double kd;
  double minIntegral;
  double maxIntegral;
  
  PIDParams(double p = 0, double i = 0, double d = 0, double minInt = -1000, double maxInt = 1000) 
    : kp(p), ki(i), kd(d), minIntegral(minInt), maxIntegral(maxInt) {}
};

// Motor control structure
struct MotorControl {
  int leftSpeed;
  int rightSpeed;
  bool leftDirection;  // true = forward, false = backward
  bool rightDirection;
  
  MotorControl() : leftSpeed(0), rightSpeed(0), leftDirection(true), rightDirection(true) {}
};

// Sensor data structure
struct SensorData {
  int magnetFront[16];
  int magnetBack[16];
  uint16_t ultrasonicFront[5];
  uint16_t ultrasonicBack[5];
  int totalActiveSensors;
  int errorValue;
  bool obstacleDetected;
  
  SensorData() : totalActiveSensors(0), errorValue(0), obstacleDetected(false) {
    for(int i = 0; i < 16; i++) {
      magnetFront[i] = 0;
      magnetBack[i] = 0;
    }
    for(int i = 0; i < 5; i++) {
      ultrasonicFront[i] = 0;
      ultrasonicBack[i] = 0;
    }
  }
};

// RFID station structure
struct RfidStation {
  int stationId;
  String rfidId;
  bool isActive;
  
  RfidStation() : stationId(0), rfidId(""), isActive(false) {}
  RfidStation(int id, String rfid) : stationId(id), rfidId(rfid), isActive(true) {}
};

// Device status structure
struct DeviceStatus {
  bool isOnline;
  unsigned long lastSuccessfulComm;
  int errorCount;
  
  DeviceStatus() : isOnline(false), lastSuccessfulComm(0), errorCount(0) {}
};

// System configuration structure
struct SystemConfig {
  // PID parameters
  PIDParams pidLinefollower;
  PIDParams pidGeneral;
  
  // Motor settings
  int baseSpeed;
  int maxPwm;
  bool invertMotorY;
  bool invertMotorX;
  bool invertMotorKanan;
  bool invertMotorKiri;
  bool invertHook;
  
  // Music pin mapping
  int musicStationPin;
  int musicErrorPin;
  int musicDetectPin;
  int musicKomputerPin;
  
  // Safety settings
  uint16_t minSafeDistance;
  unsigned long commTimeoutMs;
  
  SystemConfig() {
    pidLinefollower = PIDParams(70.0, 0.0, 0.0);
    pidGeneral = PIDParams(1.0, 0.0, 0.0);
    baseSpeed = 1000;
    maxPwm = 4095;
    invertMotorY = false;
    invertMotorX = false;
    invertMotorKanan = false;
    invertMotorKiri = false;
    invertHook = false;
    musicStationPin = 0;
    musicErrorPin = 1;
    musicDetectPin = 2;
    musicKomputerPin = 3;
    minSafeDistance = 30;
    commTimeoutMs = 5000;
  }
};

// Global system state
struct SystemState {
  MenuState currentMenu;
  AgvMode currentAgvMode;
  MovementState currentMovement;
  HookState currentHookState;
  bool isAgvMode;
  bool isInitialized;
  
  SystemState() {
    currentMenu = MENU_MAIN;
    currentAgvMode = MODE_TERMINAL;
    currentMovement = MOVEMENT_STOP;
    currentHookState = HOOK_IDLE;
    isAgvMode = false;
    isInitialized = false;
  }
};

// Station list type
typedef std::vector<int> StationsList;

#endif // TYPES_H