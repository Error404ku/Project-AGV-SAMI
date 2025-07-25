#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"
#include <Wire.h>
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <ModbusMaster.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <Wiegand.h>

// ==================== HARDWARE PIN DEFINITIONS ====================

// LCD I2C Configuration
#define LCD_COLUMNS 20
#define LCD_ROWS 4
#define LCD_ADDRESS 0x27

// Motor L298N Pins
#define MOTOR_IN1 32
#define MOTOR_IN2 33
#define MOTOR_IN3 25
#define MOTOR_IN4 26
#define MOTOR_ENA 27
#define MOTOR_ENB 14

// RS485 Communication Pins
#define MAX485_DE 4
#define MAX485_RE 4
#define RX_RS485 16
#define TX_RS485 17

// Hook Motor Pins
#define HOOK_RELAY_PIN 21
#define LIMIT_SWITCH_UP_PIN 20
#define LIMIT_SWITCH_DOWN_PIN 19

// Button Pins
#define BUTTON_UP_PIN 10
#define BUTTON_DOWN_PIN 40
#define BUTTON_RIGHT_PIN 39
#define BUTTON_LEFT_PIN 42
#define BUTTON_START_PIN 9
#define BUTTON_STOP_PIN 41

// RFID Wiegand Pins
#define PIN_D0 12
#define PIN_D1 13

// Music/LED Pins
#define MUSIC_PIN_0 2
#define MUSIC_PIN_1 18
#define MUSIC_PIN_2 5
#define MUSIC_PIN_3 23

// ==================== COMMUNICATION CONSTANTS ====================

// RS485 Configuration
#define BAUDRATE_RS485 9600
#define PACKET_LENGTH 13
#define MAX_DEVICES 4

// Device Addresses
#define ADDR_MAGNET_FRONT 1
#define ADDR_ULTRASONIC_FRONT 2
#define ADDR_ULTRASONIC_BACK 3
#define ADDR_MAGNET_BACK 4

// WiFi Configuration
#define WIFI_SSID "AGV_SAMI"
#define WIFI_PASSWORD "12345678"
#define WIFI_STATIC_IP IPAddress(192, 168, 4, 1)
#define WIFI_GATEWAY IPAddress(192, 168, 4, 1)
#define WIFI_SUBNET IPAddress(255, 255, 255, 0)

// ==================== TIMING CONSTANTS ====================

// Communication Timing
#define DEVICE_SWITCH_INTERVAL 100  // ms
#define COMM_TIMEOUT_MS 5000        // ms
#define SENSOR_READ_INTERVAL 50     // ms

// Button Timing
#define BUTTON_DEBOUNCE_DELAY 300   // ms
#define BUTTON_HOLD_DURATION 3000   // ms

// Menu Timing
#define MENU_REFRESH_INTERVAL 100   // ms
#define RFID_SCAN_TIMEOUT 10000     // ms

// Safety Timing
#define OBSTACLE_CHECK_INTERVAL 100 // ms
#define HOOK_DELAY_AT_LIMIT 2000    // ms

// ==================== SYSTEM LIMITS ====================

// Sensor Limits
#define MAX_MAGNET_SENSORS 16
#define MAX_ULTRASONIC_SENSORS 5
#define MIN_SAFE_DISTANCE 30        // cm

// Motor Limits
#define MAX_PWM_VALUE 4095
#define MIN_PWM_VALUE 0
#define DEFAULT_BASE_SPEED 1000

// Menu Limits
#define MAX_MENU_DISPLAY 3
#define MAX_MANUAL_TARGETS 10
#define MAX_RFID_STATIONS 20

// PID Limits
#define PID_MAX_INTEGRAL 1000.0
#define PID_MIN_INTEGRAL -1000.0
#define PID_MAX_OUTPUT 4095.0
#define PID_MIN_OUTPUT -4095.0

// ==================== DEFAULT VALUES ====================

// PID Default Values
#define DEFAULT_KP_LINEFOLLOWER 70.0
#define DEFAULT_KI_LINEFOLLOWER 0.0
#define DEFAULT_KD_LINEFOLLOWER 0.0

#define DEFAULT_KP_GENERAL 1.0
#define DEFAULT_KI_GENERAL 0.0
#define DEFAULT_KD_GENERAL 0.0

// PWM Configuration
#define PWM_FREQUENCY 1000
#define PWM_RESOLUTION 12
#define PWM_CHANNEL_ENA 0
#define PWM_CHANNEL_ENB 1

// ==================== PREFERENCES NAMESPACES ====================

#define PREF_NAMESPACE_SETTINGS "agv-settings"
#define PREF_NAMESPACE_STATIONS "stations"
#define PREF_NAMESPACE_RFID "rfid-stations"
#define PREF_NAMESPACE_ERRORS "error-log"

// ==================== GLOBAL OBJECTS ====================

// Hardware Objects
extern LiquidCrystal_I2C lcd;
extern WebServer server;
extern Preferences preferences;
extern ModbusMaster nodeMagnetFront;
extern ModbusMaster nodeMagnetBack;
extern Wiegand wiegand;

// ==================== GLOBAL VARIABLES ====================

// System State
extern SystemState systemState;
extern SystemConfig systemConfig;
extern SensorData sensorData;

// Communication Variables
extern bool inPacket;
extern int byteCounter;
extern byte dataPacket[PACKET_LENGTH];
extern int currentDeviceAddress;
extern DeviceStatus deviceStatus[MAX_DEVICES];

// Station Management
extern StationsList stationsList;
extern RfidStation rfidStations[MAX_RFID_STATIONS];
extern int rfidStationCount;

// Menu Variables
extern int selectedItem;
extern int maxItems;
extern int menuStartIndex;
extern bool menuNeedsRefresh;

// RFID Variables
extern String lastScannedRfid;
extern bool newRfidScanned;

// Music Variables
extern bool statusMusic;

// Movement Variables
extern bool modeMaju;
extern bool modeMundur;
extern bool modeBerhenti;
extern bool force;

// Sensor State Variables
extern bool tengahAktif;
extern bool kananHilang;
extern bool kiriHilang;
extern bool sensorkebacasemua;

// Station Variables
extern int station;
extern bool sudahDeteksiStasiun;
extern int indexTarget;

// Timing Variables
extern unsigned long previousMillis;
extern unsigned long lastDeviceSwitch;
extern unsigned long lastButtonPress;
extern unsigned long lastObstacleCheck;

// ==================== FUNCTION DECLARATIONS ====================

// Initialization Functions
void setupAll();
void setupHardware();
void setupCommunication();
void setupSensors();
void setupInterface();

// Main Loop Functions
void loopSensors();
void loopCommunication();
void loopInterface();
void loopControl();
void loopSafety();

// Utility Functions
void logError(ErrorCode code, String message);
void saveSystemConfig();
void loadSystemConfig();
void resetToDefaults();

// Debug Functions
#ifdef DEBUG_MODE
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x, ...) Serial.printf(x, __VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x, ...)
#endif

#endif // CONFIG_H