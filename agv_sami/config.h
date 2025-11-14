#ifndef CONFIG_H
#define CONFIG_H

// ===================================================================
//                           DEBUG FLAGS
// ===================================================================
#define DEBUG_PID 0
#define DEBUG_SENSOR 0
#define DEBUG_MOTOR 0
#define DEBUG_SETUP 0
#define DEBUG_WIFI 0
#define DEBUG_PERFORMANCE 0

#if DEBUG_SETUP
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x, ...) Serial.printf(x, __VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x, ...)
#endif

// ===================================================================
//                        LIBRARY INCLUDES
// ===================================================================
#include <Wire.h>
#include <Arduino.h>
#include <math.h>
#include <LiquidCrystal_I2C.h>
#include <ModbusMaster.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <unordered_map>
#include <vector>
#include <SPIFFS.h>
#include <algorithm>
#include <Wiegand.h>
#include <Adafruit_SSD1306.h>

// ===================================================================
//                          ENUMERATIONS
// ===================================================================
enum AgvState {
  AGV_STATE_MOVE_FORWARD,
  AGV_STATE_STOP,
  AGV_STATE_TERMINAL_PICKUP,
  AGV_STATE_TERMINAL_DROP,
  AGV_STATE_WAREHOUSE,
  AGV_STATE_STATION,
  AGV_STATE_SWITCH_FORWARD,
  AGV_STATE_NULL
};

enum PidMode {
  PID_MODE_MAJU,
  PID_MODE_MAJU_MASSA,
  PID_MODE_BERHENTI,
  PID_MODE_DEFAULT
};

enum MusicMode {
  MUSIC_MODE_ON,
  MUSIC_MODE_OBSTACLE,
  MUSIC_MODE_STATION,
  MUSIC_MODE_OUTOFLINE,
  MUSIC_MODE_WARNING
};

enum HookPosition {
  UP_POS,
  DOWN_POS,
  STOP_POS
};

enum HookPositionMode {
  UP_HOOK,
  DOWN_HOOK,
  STOP_HOOK
};

// ===================================================================
//                        MENU STATE DEFINITIONS
// ===================================================================
// Main Menu States
#define MENU_MAIN 0
#define MENU_AGV_MODE 1
#define MENU_MOTOR_TEST 2
#define MENU_PID_SETTINGS 3
#define MENU_TARGET_SETTINGS 4
#define MENU_RESET 5
#define MENU_RFID_SETTINGS 6
#define MENU_WIFI_SETTINGS 14
#define MENU_MOTOR_SETTINGS 18
#define MENU_MOTOR_INVERT 19
#define MENU_MUSIC_SETTINGS 20
#define MENU_MUSIC_TEST 21
#define MENU_HOOK_TEST 22
#define MENU_MAGNET_CHECK 23
#define MENU_ULTRASONIC_CHECK 24
#define MENU_RESET_AGV_STATE 25
#define MENU_TERMINAL_DROP 26
#define MENU_TERMINAL_PICKUP 27

// PID Submenu
#define MENU_PID_FORWARD 28
#define MENU_PID_BACKWARD 29
#define MENU_PID_FORWARD_WITHMASSA 34
#define MENU_PID_FORWARD_DEFAULT 35
#define MENU_PID_BACKWARD_WITHMASSA 36
#define MENU_PID_BACKWARD_DEFAULT 37

// Ultrasonic Submenu
#define MENU_ULTRASONIC_SETTINGS 30
#define MENU_ULTRASONIC_FRONT 31
#define MENU_ULTRASONIC_BACK 32
#define MENU_ULTRASONIC_FRONT_TENGAH 51
#define MENU_ULTRASONIC_FRONT_SERONG 52
#define MENU_ULTRASONIC_BACK_TENGAH 53
#define MENU_ULTRASONIC_BACK_SERONG 54

// Music Submenu
#define MENU_MUSIC_ON 40
#define MENU_MUSIC_OBSTACLE 41
#define MENU_MUSIC_STATION 42
#define MENU_MUSIC_OUTOFLINE 43
#define MENU_MUSIC_WARNING 44

// Motor Settings Submenu
#define MENU_SPEED_SETTING 46
#define MENU_PID_RPM_SETTING 47
#define MENU_PID_RPM_RIGHT 48
#define MENU_MOTOR_TEST_PWM 49
#define MENU_MOTOR_TEST_RPM 50
#define MENU_PID_RPM_LEFT 59

// RPM Tuning Menu
#define MENU_RPM_TUNING 55
#define MENU_RPM_TUNE_START 56
#define MENU_RPM_TUNE_CANCEL 57
#define MENU_RPM_TUNE_STATUS 58
#define MENU_RPM_TUNE_RIGHT 62
#define MENU_RPM_TUNE_LEFT 63

// RFID Submenu
#define MENU_RFID_UJUNG 15
#define MENU_RFID_WAREHOUSE 16
#define MENU_AUTO_INPUT_STATION 17

// ===================================================================
//                          ERROR CODES
// ===================================================================
#define ERROR_SENSOR_COMMUNICATION 1
#define ERROR_MOTOR_CONTROL 2
#define ERROR_RFID_COMMUNICATION 3
#define ERROR_WIFI_CONNECTION 4
#define ERROR_LCD_COMMUNICATION 5
#define ERROR_SYSTEM_INITIALIZATION 6
#define ERROR_ENCODER_FAILURE 7
#define ERROR_PID_CALCULATION 8
#define ERROR_MEMORY_ALLOCATION 9
#define ERROR_INVALID_CONFIGURATION 10
#define ERROR_ULTRASONIC_COMMUNICATION 11

// ===================================================================
//                          PIN DEFINITIONS
// ===================================================================
// Button Pins
#define BOOT_PIN 0
#define PIN_UP 10
#define PIN_LEFT 42
#define PIN_RIGHT 39
#define PIN_DOWN 40
#define PIN_START 9
#define PIN_STOP 41

// I2C Pins
#define sdaPin 3
#define sclPin 8

// LCD Configuration
#define LCD_COLUMNS 20
#define LCD_ROWS 4
#define LCD_ADDRESS 0x27

// Motor Control Pins (L298N)
#define IN1 48
#define IN2 38
#define IN3 4
#define IN4 5
#define ENA 35
#define ENB 6

// RS485 Pins for Magnet Sensors (Serial1)
#define MAX485_DE 36
#define MAX485_RE 36
#define RS485_RX 18
#define RS485_TX 17

// RS485 Pins for Ultrasonic Sensors (Serial2)
#define RS485_RX2 11
#define RS485_TX2 46

// RFID Pins (Wiegand)
#define PIN_D0 12
#define PIN_D1 13

// Music Pins (Relay)
#define pinMusic1 7
#define pinMusic2 15
#define pinMusic3 16
#define pinMusic4 14
#define pinMusic5 37
#define pinMusic6 2

// Hook Pins (Relay)
#define pinHook1 20
#define pinHook2 45
#define pinMotorHook 21

// Lamp Pin (Interrupt)
#define lampPin 47

// ===================================================================
//                          SLAVE IDs
// ===================================================================
#define SLAVEID_MAGNET_DEPAN 1
#define SLAVEID_ULTRASONIK_DEPAN 2
#define SLAVEID_ULTRASONIK_BELAKANG 3
#define SLAVEID_MAGNET_BELAKANG 4

// ===================================================================
//                        TIMING CONSTANTS
// ===================================================================
// Button & Input Timing
const unsigned long BUTTON_HOLD_INTERVAL = 100;
const unsigned long BUTTON_DEBOUNCE_DELAY = 200;
const unsigned long debounceDelay = 300;
const unsigned long X_HOLD_DURATION = 3000;
const unsigned long buttonDelay = 200;

// Display & UI Timing
const unsigned long MESSAGE_DISPLAY_DURATION = 2000;
const unsigned long SHORT_DISPLAY_DURATION = 1000;
const unsigned long DISPLAY_UPDATE_INTERVAL = 200;

// Sensor Timing
const unsigned long SENSOR_READ_INTERVAL = 100;
const unsigned long obstacleCheckInterval = 100;

// System Timing
const unsigned long NVS_WRITE_DELAY = 50;
const unsigned long OPERATION_DELAY = 100;
const unsigned long SWITCH_DEBOUNCE = 200;

// Communication Timing
const unsigned long RPM_REQUEST_INTERVAL = 100;
const unsigned long TUNING_STATUS_INTERVAL = 2000;
const unsigned long PID_REQUEST_TIMEOUT = 15000;

// RFID Timing
const unsigned long RFID_SCAN_TIMEOUT = 10000;
const unsigned long UJUNG_IGNORE_DURATION = 5000;

// WiFi Timing
const unsigned long WIFI_CONNECT_TIMEOUT = 3000;

// Performance Monitoring Timing
const unsigned long performanceUpdateInterval = 5000;
const unsigned long ACCELERATION_INTERVAL = 500;

// ===================================================================
//                          SYSTEM CONSTANTS
// ===================================================================
// PWM Configuration
const int pwmResolution = 12;
const int pwmFrequency = 5000;
const int maxPwm = 4096;
const int minPwm = -4096;

// Menu Configuration
const int MAX_MANUAL_TARGETS = 10;
const float MAX_INCREMENT = 10.0f;
const int maxInvertItems = 5;
const int maxMusicItems = 5;
const int WIFI_MAX_SCROLL = 3;

// RFID Configuration
const int MAX_RFID_STATIONS = 50;
const int MAX_RFID_UJUNG = 2;
const int MAX_RFID_WAREHOUSE = 1;

// PID Configuration
const int numOutputs = 10;

// ===================================================================
//                          DATA STRUCTURES
// ===================================================================
struct Timer {
  unsigned long previousMillis;
  unsigned long interval;
  bool active;
  bool triggered;
};

struct PIDData {
  double error;
  double integral;
  double derivative;
  double previousError;
  double lastDerivative;
  double lastOutput;
  unsigned long lastComputeTime;
};

struct RfidStation {
  int stationId;
  String rfidId;
  bool isActive;
};

struct RfidUjung {
  int ujungId;
  String rfidId;
  bool isActive;
};

struct RfidWarehouse {
  int warehouseId;
  String rfidId;
  bool isActive;
};

// ===================================================================
//                        GLOBAL OBJECTS
// ===================================================================
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
WebServer server(80);
Preferences preferences;
Preferences stationsPreferences;
Wiegand wiegand;
ModbusMaster magnetNode;
ModbusMaster ultrasonicNode;

// ===================================================================
//                      GLOBAL VARIABLES
// ===================================================================
// Communication
int BAUDRATE = 115200;
String motorControllerBuffer = "";
bool motorControllerStringComplete = false;

// AGV State
AgvState currentStateAgv = AGV_STATE_NULL;
AgvState currentRFID = AGV_STATE_NULL;
bool isAgvMode = false;
bool agvStopCalled = false;
bool stopCalledPickup = false;
bool exceptErrorPosition = false;

// Sensor Variables
int currentMagnetSlaveId = SLAVEID_MAGNET_DEPAN;
int currentUltrasonicSlaveId = SLAVEID_ULTRASONIK_DEPAN;
int lastErrorValue = 99;
int errorValue = 0;
int jumlahMagnet[16];
int totalSensorAktif = 0;
unsigned long lastDetectionTime = 0;
unsigned long lastObstacleCheck = 0;
bool obstacleDetected = false;

// Ultrasonic Distances
uint16_t ultrasonicDistances[5] = { 0 };
uint16_t minSafeDistanceFront = 30;
uint16_t minSafeDistanceBack = 20;
uint16_t minSafeDistanceFrontSerong = 25;
uint16_t minSafeDistanceBackSerong = 15;

// Button Variables
bool tombolBoot = false;
unsigned long bootHoldStart = 0;
int lastPressed;
int upPin = PIN_UP;
int downPin = PIN_DOWN;
int rightPin = PIN_RIGHT;
int leftPin = PIN_LEFT;
int startPin = PIN_START;
int stopPin = PIN_STOP;
unsigned long lastUpPress = 0;
unsigned long lastDownPress = 0;
unsigned long lastLeftPress = 0;
unsigned long lastRightPress = 0;
unsigned long lastStartPress = 0;
unsigned long lastStopPress = 0;
int buttonStep = 0;

// PID Variables - Combined Motor Control
double kp = 0.2;
double ki = 0.4;
double kd = 0.0;
double motorPidKp = 1.0;
double motorPidKi = 0.15;
double motorPidKd = 0.0;

// PID Variables - Line Follower Modes
double kpForwardWithMassa = 70.0;
double kiForwardWithMassa = 0.0;
double kdForwardWithMassa = 0.0;
double kpForwardDefault = 70.0;
double kiForwardDefault = 0.0;
double kdForwardDefault = 0.0;
double kpBackwardWithMassa = 70.0;
double kiBackwardWithMassa = 0.0;
double kdBackwardWithMassa = 0.0;
double kpBackwardDefault = 70.0;
double kiBackwardDefault = 0.0;
double kdBackwardDefault = 0.0;

// PID Variables - Right Motor
double motorPidKpRight = 1.0;
double motorPidKiRight = 0.15;
double motorPidKdRight = 0.0;

// PID Variables - Left Motor
double motorPidKpLeft = 1.0;
double motorPidKiLeft = 0.15;
double motorPidKdLeft = 0.0;

// PID Data Arrays
PIDData pidData[numOutputs];

// Motor Control Variables
int baseSpeed = 2000;
int pidSpeed = 0;
int pwmKanan = 0;
int pwmKiri = 0;
int maxMotorRpm = 90;
int testMotorSpeed = 1000;
float currentRpmKanan = 0.0;
float currentRpmKiri = 0.0;
unsigned long lastRpmRequestTime = 0;
bool invertMotorY = false;
bool invertMotorX = false;
bool invertMotorKanan = false;
bool invertMotorKiri = false;
int motorTestState = 0;

// Hook Variables
bool isHookUp = false;
bool invertHook = false;
HookPosition hookPosition = STOP_POS;
HookPositionMode hookPositionMode = STOP_HOOK;
int hookTestState = 0;

// Soft Start Variables
static unsigned long softStartTime = 0;
static bool softStartActive = false;
static PidMode lastMode = PID_MODE_DEFAULT;
static bool firstChange = true;
static bool forceLeft = false;
static bool inLine = true;

// Timer Instances
Timer stopPelanPelanTimer = {0, 500, false, false};
Timer ultrasonicSwitchTimer = {0, 100, false, false};
Timer magnetSwitchTimer = {0, 50, false, false};
Timer buttonDebounceTimer = {0, 300, false, false};
Timer menuDelayTimer = {0, 1500, false, false};
Timer errorRecoveryTimer = {0, 5000, false, false};
Timer performanceTimer = {0, 5000, false, false};

// Performance Monitoring Variables
unsigned long loopStartTime = 0;
unsigned long loopExecutionTime = 0;
unsigned long maxLoopTime = 0;
unsigned long minLoopTime = 999999;
unsigned long totalLoops = 0;
unsigned long lastPerformanceUpdate = 0;
bool systemInErrorState = false;
int errorRecoveryAttempts = 0;
size_t freeHeapSize = 0;
size_t minFreeHeap = 0;
bool resetDisplayRequested = false;

// Menu System Variables
int selectedItem = 0;
int maxItems;
int menuStartIndex = 0;
int maxMenuDisplay = 3;
int currentMenu = 0;
bool menuNeedsRefresh = true;
int lastSelectedItem = -1;
int lastMenuStartIndex = -1;
int selectedParam = 0;
int selectedTarget = 0;
int selectedRfidItem = 0;
int selectedStationId = 1;
int selectedTuningItem = 0;
int selectedTuningSubItem = 0;
int selectedInvertItem = 0;
int selectedMusicItem = 0;
int selectedMusicPin = 0;
int wifiScrollIndex = 0;
unsigned long lastButtonPress = 0;
unsigned long pidButtonHoldStart = 0;
float pidIncrement = 0.01f;
unsigned long xButtonHoldStart = 0;
bool isClearingStations = false;
unsigned long previousMillis = 0;
const unsigned long interval = 100;

// RFID Variables
String terminalDropRfidId = "";
String terminalPickUpRfidId = "";
String warehouseRfidId = "";
String ujungRfidId = "";
char lastScannedRfidOptimized[32] = "";
bool newRfidScanned = false;
RfidStation rfidStations[MAX_RFID_STATIONS];
RfidUjung rfidUjungList[MAX_RFID_UJUNG];
RfidWarehouse rfidWarehouseList[MAX_RFID_WAREHOUSE];
int rfidStationCount = 0;
int rfidUjungCount = 0;
int rfidWarehouseCount = 0;
bool updatestations = false;
bool isUjungSlowMode = false;
unsigned long lastUjungDetectionTime = 0;
bool isScanning = false;
int currentScanStation = 0;
bool isWaitingForRfid = false;
unsigned long rfidScanTimeout = 0;

// Target Settings Variables
bool useAutoTarget = false;
int manualTargetCount = 2;
std::vector<int> targetStationsList;

// RPM Tuning Variables
String tuningStatus = "IDLE";
int tuningProgress = 0;
unsigned long lastTuningStatusRequest = 0;

// WiFi Variables
char ssid[32] = "My Phone";
char password[64] = "kalolaparmakan";
char staticIPStr[16] = "192.168.121.14";
char gatewayStr[16] = "192.168.121.99";
char subnetStr[16] = "255.255.255.0";
char dnsStr[16] = "192.168.121.99";
IPAddress staticIP(192, 168, 121, 14);
IPAddress gateway(192, 168, 121, 99);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 121, 99);
bool isConnectingWifi = false;
bool wifiConnectionResult = false;
unsigned long wifiConnectStartTime = 0;
const char* PREFERENCES_NAMESPACE = "device_data";
const char* STATIONS_NAMESPACE = "stations";

// Music Variables
bool statusMusic = false;
int musicOnPin = 0;
int musicObstaclePin = 1;
int musicStationPin = 2;
int musicOutOfLinePin = 3;
int musicWarningPin = 4;
MusicMode currentMusicMode = MUSIC_MODE_ON;

// Startup Synchronization Flags
bool pidDataReceived = false;
bool pidDataReceivedRight = false;
bool pidDataReceivedLeft = false;
bool systemReadyToRun = false;
unsigned long pidRequestStartTime = 0;

// ===================================================================
//                   EXTERNAL VARIABLE DECLARATIONS
// ===================================================================
extern int targetStation[2];
extern double kp, ki, kd;
extern int baseSpeed;
extern bool tombolBoot;
extern int selectedItem;
extern int maxItems;
extern bool isAgvMode;

// ===================================================================
//                      FUNCTION DECLARATIONS
// ===================================================================
// Setup and Main Menu
void setupMenu();
void handleMenu();
void displayMainMenu();

// Motor Test Functions
void displayMotorTest();
void handleMotorTest();
void displayMotorTestPWM();
void handleMotorTestPWM();
void displayMotorTestRPM();
void handleMotorTestRPM();

// PID Settings Functions
void displayPidSettings();
void handlePidSettings();
void displayPidSubmenu();
void handlePidSubmenu();
void displayPidForwardSubmenu();
void handlePidForwardSubmenu();
void displayPidBackwardSubmenu();
void handlePidBackwardSubmenu();
void displayPidForwardWithMassaSettings();
void handlePidForwardWithMassaSettings();
void displayPidForwardDefaultSettings();
void handlePidForwardDefaultSettings();
void displayPidBackwardWithMassaSettings();
void handlePidBackwardWithMassaSettings();
void displayPidBackwardDefaultSettings();
void handlePidBackwardDefaultSettings();

// Target Settings Functions
void displayTargetSettings();
void handleTargetSettings();

// WiFi Settings Functions
void displayWifiSettings();
void handleWifiSettings();

// Ultrasonic Settings Functions
void displayUltrasonicSettings();
void handleUltrasonicSettings();
void displayUltrasonicFrontSettings();
void handleUltrasonicFrontSettings();
void displayUltrasonicBackSettings();
void handleUltrasonicBackSettings();
void displayUltrasonicFrontTengahSettings();
void handleUltrasonicFrontTengahSettings();
void displayUltrasonicFrontSerongSettings();
void handleUltrasonicFrontSerongSettings();
void displayUltrasonicBackTengahSettings();
void handleUltrasonicBackTengahSettings();
void displayUltrasonicBackSerongSettings();
void handleUltrasonicBackSerongSettings();

// Music Settings Functions
void displayMusicSettings();
void handleMusicSettings();
void displayMusicSubmenu(const char* title, int* currentPin);
void handleMusicSubmenu(int* targetPin);

// Motor Settings Functions
void displayMotorSettings();
void handleMotorSettings();
void displaySpeedSetting();
void handleSpeedSetting();
void displayPidRpmSetting();
void handlePidRpmSetting();
void displayPidRpmRight();
void handlePidRpmRight();
void displayPidRpmLeft();
void handlePidRpmLeft();

// RPM Tuning Functions
void displayRpmTuningMenu();
void handleRpmTuningMenu();
void displayTuningStatus();
void displayTuningStartMenu(String motorName);
void handleTuningStartMenu(String command);
void sendTuningCommand(String command);
void parseTuningResponse(String response);

// RFID Functions
String getRfidForStation(int stationId);

// Music Functions
void music(MusicMode mode);
void stopMusic();
void silentMusic();

// Hook Functions
HookPosition hook(HookPositionMode mode);

// Motor Control Functions
void rpmMotor(int rpmKiri, int rpmKanan);
void motorStop();

#endif // CONFIG_H
