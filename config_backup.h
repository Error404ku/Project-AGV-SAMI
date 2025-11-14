#ifndef CONFIG_H
#define CONFIG_H
// Debug flags - DISABLED untuk production
#define DEBUG_PID 0        // Disable PID debugging
#define DEBUG_SENSOR 0     // Disable sensor debugging
#define DEBUG_MOTOR 0      // Disable motor debugging
#define DEBUG_SETUP 0      // Disable setup debugging
#define DEBUG_WIFI 0       // Disable WiFi debugging
#define DEBUG_PERFORMANCE 0 // Disable performance debugging

// Debug macros - hanya aktif jika debug flag enabled
#if DEBUG_SETUP
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x, ...) Serial.printf(x, __VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x, ...)
#endif

// Include Library
#include <Wire.h>
#include <Arduino.h>
#include <math.h>
#include <LiquidCrystal_I2C.h>  // ESP32 compatible LCD I2C Library
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

// ===================================================================
//                        FREERTOS SHARED DATA STRUCTURES
// ===================================================================

// PID Mode enumeration
enum PidMode {
  PID_MODE_MAJU,
  PID_MODE_MAJU_MASSA,
  PID_MODE_BERHENTI,
  PID_MODE_DEFAULT
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
//                        PIN DEFINITIONS
// ===================================================================

#define BOOT_PIN 0

// Button pins
#define PIN_UP 10     // UP button
#define PIN_LEFT 42   // LEFT button
#define PIN_RIGHT 39  // RIGHT button
#define PIN_DOWN 40   // DOWN button
#define PIN_START 9   // START button
#define PIN_STOP 41   // STOP button

// Slave IDs
#define SLAVEID_MAGNET_DEPAN 1
#define SLAVEID_ULTRASONIK_DEPAN 2
#define SLAVEID_ULTRASONIK_BELAKANG 3
#define SLAVEID_MAGNET_BELAKANG 4

// ===================================================================
//                        DEKLARASI GLOBAL VARIABLES
// ===================================================================

// --- HTTP & WEB SERVER ---
WebServer server(80);

// --- PREFERENCES & STORAGE ---
Preferences preferences;
Preferences stationsPreferences;  // Objek Preferences untuk station yang ditemukan
std::vector<int> targetStationsList;    // Array di RAM untuk menyimpan station yang ditemukan

// --- COMMUNICATION ---
int BAUDRATE = 115200;

// --- SENSOR MAGNET VARIABLES ---
int currentMagnetSlaveId = SLAVEID_MAGNET_DEPAN;
int lastErrorValue = 99;
unsigned long lastDetectionTime = 0;

// --- LOGIC AGV VARIABLES ---
bool stopCalledPickup = false;

// --- SENSOR ULTRASONIK VARIABLES ---
bool obstacleDetected = false;
unsigned long lastObstacleCheck = 0;
const unsigned long obstacleCheckInterval = 100;  // Check every 100ms
// const unsigned long magnetReadInterval = 50;      // Magnet sensor read interval (50ms for high responsiveness)
int currentUltrasonicSlaveId = SLAVEID_ULTRASONIK_DEPAN;

// --- TOMBOL/BUTTON VARIABLES ---
bool tombolBoot = false;
unsigned long bootHoldStart = 0;
int lastPressed;

// Button pin assignments
int upPin = PIN_UP;
int downPin = PIN_DOWN;
int rightPin = PIN_RIGHT;
int leftPin = PIN_LEFT;
int startPin = PIN_START;
int stopPin = PIN_STOP;

// Button debounce timers
unsigned long lastUpPress = 0;
unsigned long lastDownPress = 0;
unsigned long lastLeftPress = 0;
unsigned long lastRightPress = 0;
unsigned long lastStartPress = 0;
unsigned long lastStopPress = 0;

// Soft start variables for PID
static unsigned long softStartTime = 0;
static bool softStartActive = false;
static PidMode lastMode = PID_MODE_DEFAULT;

// Hook status variable
bool isHookUp = false;

// --- PERFORMANCE OPTIMIZATION VARIABLES ---
unsigned long loopStartTime = 0;
unsigned long loopExecutionTime = 0;
unsigned long maxLoopTime = 0;
unsigned long minLoopTime = 999999;
unsigned long totalLoops = 0;
unsigned long performanceUpdateInterval = 5000; // 5 seconds
unsigned long lastPerformanceUpdate = 0;
bool systemInErrorState = false;
int errorRecoveryAttempts = 0;

// Memory tracking variables
size_t freeHeapSize = 0;
size_t minFreeHeap = 0;

// AGV State variables
AgvState currentStateAgv = AGV_STATE_NULL;
AgvState currentRFID = AGV_STATE_NULL;
static bool firstChange = true;

// Timer system structure
struct Timer {
  unsigned long previousMillis;
  unsigned long interval;
  bool active;
  bool triggered;
};

// Timer instances for different operations
Timer stopPelanPelanTimer = {0, 500, false, false};
Timer ultrasonicSwitchTimer = {0, 100, false, false};
Timer magnetSwitchTimer = {0, 50, false, false};  // Optimized to match magnet read interval
Timer buttonDebounceTimer = {0, 300, false, false};
Timer menuDelayTimer = {0, 1500, false, false};
Timer errorRecoveryTimer = {0, 5000, false, false};
Timer performanceTimer = {0, 5000, false, false};

// Sensor distances array
uint16_t ultrasonicDistances[5] = { 0 };  // Store distances from 5 probes
uint16_t minSafeDistanceFront = 30;       // cm - minimum safe distance for front sensor
uint16_t minSafeDistanceBack = 20;        // cm - minimum safe distance for back sensor
uint16_t minSafeDistanceFrontSerong = 25;  // cm - minimum safe distance for front serong sensor
uint16_t minSafeDistanceBackSerong = 15;   // cm - minimum safe distance for back serong sensor

// --- RFID TERMINAL VARIABLES ---
String terminalDropRfidId = "";
String terminalPickUpRfidId = "";
bool exceptErrorPosition = false;

// --- WAREHOUSE & UJUNG RFID VARIABLES ---
String warehouseRfidId = "";
String ujungRfidId = "";

// --- MENU SYSTEM VARIABLES ---
int selectedItem = 0;
int maxItems;
int menuStartIndex = 0;        // For scrolling menu
int maxMenuDisplay = 3;        // Maximum items displayed at once
bool isAgvMode = false;
int currentMenu = 0;           // MENU_MAIN

// --- KONSTANTA MENU ---
const int MAX_MANUAL_TARGETS = 10;  // Maximum number of manual targets allowed
const unsigned long debounceDelay = 300;  // 200ms debounce
const unsigned long RFID_SCAN_TIMEOUT = 10000;  // 10 seconds timeout
const unsigned long X_HOLD_DURATION = 3000;  // 3 seconds hold
const float MAX_INCREMENT = 10.0f;
const unsigned long ACCELERATION_INTERVAL = 500;  // Time in ms to increase increment
const unsigned long buttonDelay = 200;  // Delay in milliseconds between button presses
const int maxInvertItems = 5;
const int maxMusicItems = 5;
const unsigned long WIFI_CONNECT_TIMEOUT = 3000; // Optimized to 3 seconds
const int WIFI_MAX_SCROLL = 3; // Maximum scroll positions (0-3 existing)

// ===================================================================
// TIMING CONSTANTS - Moved from menu.ino
// ===================================================================
// Button & Input Timing
const unsigned long BUTTON_HOLD_INTERVAL = 100;        // Hold button repeat interval
const unsigned long BUTTON_DEBOUNCE_DELAY = 200;      // Button debounce delay

// Display & UI Timing
const unsigned long MESSAGE_DISPLAY_DURATION = 2000;   // Message display time (2s)
const unsigned long SHORT_DISPLAY_DURATION = 1000;     // Short message time (1s)
const unsigned long DISPLAY_UPDATE_INTERVAL = 200;     // Display refresh interval
const unsigned long SENSOR_READ_INTERVAL = 100;        // Sensor reading interval

// System & Operation Timing
const unsigned long NVS_WRITE_DELAY = 50;              // NVS write delay
const unsigned long OPERATION_DELAY = 100;             // General operation delay
const unsigned long SWITCH_DEBOUNCE = 200;             // Switch debounce time

// Menu refresh control
bool menuNeedsRefresh = true;
int lastSelectedItem = -1;
int lastMenuStartIndex = -1;

// Motor RPM Control Variables
int maxMotorRpm = 90;           // Maximum RPM for motor control (0-90)

// PID Variables for combined motor control (backward compatibility)
double motorPidKp = 1.0;        // PID Kp for motor RPM control
double motorPidKi = 0.15;       // PID Ki for motor RPM control  
double motorPidKd = 0.0;        // PID Kd for motor RPM control

// PID Variables for individual motor control - Right Motor
double motorPidKpRight = 1.0;        // PID Kp for right motor RPM control
double motorPidKiRight = 0.15;       // PID Ki for right motor RPM control  
double motorPidKdRight = 0.0;        // PID Kd for right motor RPM control

// PID Variables for individual motor control - Left Motor
double motorPidKpLeft = 1.0;         // PID Kp for left motor RPM control
double motorPidKiLeft = 0.15;        // PID Ki for left motor RPM control  
double motorPidKdLeft = 0.0;         // PID Kd for left motor RPM control

// Current RPM values received from motor controller
float currentRpmKanan = 0.0;    // Current actual RPM of right motor
float currentRpmKiri = 0.0;     // Current actual RPM of left motor

// RPM Request Timer for Motor Test RPM
unsigned long lastRpmRequestTime = 0;
const unsigned long RPM_REQUEST_INTERVAL = 100;  // Request RPM every 100ms (10x per second)

// Motor Test Settings
int testMotorSpeed = 1000;       // Default PWM speed for motor testing (safe speed)

// Target settings
bool useAutoTarget = false;         // New variable to track target source
int manualTargetCount = 2;          // Default to 2 targets for manual mode

// Menu navigation variables
int selectedParam = 0;   // For PID settings menu
int selectedTarget = 0;  // For Target settings menu

// Button handling
unsigned long lastButtonPress = 0;

// PID adjustment variables
unsigned long pidButtonHoldStart = 0;
float pidIncrement = 0.01f;

// RFID menu variables
int selectedRfidItem = 0;
int selectedStationId = 1;
bool isWaitingForRfid = false;
unsigned long rfidScanTimeout = 0;

// RPM Tuning Variables
int selectedTuningItem = 0;         // Selected item in tuning menu (0=Start, 1=Cancel, 2=Status)
int selectedTuningSubItem = 0;      // Selected item in tuning submenu (0=Start, 1=Cancel, 2=Status)
String tuningStatus = "IDLE";       // Current tuning status from slave
int tuningProgress = 0;             // Progress percentage (0-100)
unsigned long lastTuningStatusRequest = 0;  // For periodic status updates
const unsigned long TUNING_STATUS_INTERVAL = 2000; // Update status every 2 seconds

// Target settings variables
unsigned long xButtonHoldStart = 0;
bool isClearingStations = false;

// Motor invert menu variables
int selectedInvertItem = 0;  // 0=Y-axis, 1=X-axis, 2=Motor Kanan, 3=Motor Kiri, 4=Hook

// Music settings variables
int selectedMusicItem = 0;  // 0=On, 1=Obstacle, 2=Station, 3=OutOfLine, 4=Warning
int selectedMusicPin = 0;   // 0-5 for pin selection in submenu

// Motor test variables
int motorTestState = 0;  // 0=stop, 1=forward, 2=backward, 3=left, 4=right

// Hook test variables
int hookTestState = 0;  // 0=stop, 1=naik, 2=turun

// WiFi connection variables
bool isConnectingWifi = false;
bool wifiConnectionResult = false;
unsigned long wifiConnectStartTime = 0;

// WiFi menu scroll variables
int wifiScrollIndex = 0;

// ### DEFINE ###
// # TOMBOL
// #define tombol 6  // Pin analog lama (tidak digunakan lagi)
// BOOT_PIN definition moved to top of file

// Individual button pins (manual assignment) - definitions moved to top of file

#define sdaPin 3
#define sclPin 8
// LCD I2C
#define LCD_COLUMNS 20    // Jumlah kolom LCD
#define LCD_ROWS 4        // Jumlah baris LCD
#define LCD_ADDRESS 0x27  // Alamat I2C LCD (biasanya 0x27 atau 0x3F)
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
// #Interrupt
#define lampPin 47

// #Inisialisasi Pin Motor L298N
#define IN1 48  // Motor kanan direction 1
#define IN2 38  // Motor kanan direction 2
#define IN3 4   // Motor kiri direction 1
#define IN4 5   // Motor kiri direction 2
#define ENA 35  // Motor kanan enable/PWM
#define ENB 6   // Motor kiri enable/PWM

// // #Inisialisasi Pin Encoder
// #define encKananA 42
// #define encKananB 37
// #define encKiriA 39
// #define encKiriB 48

// #Inisialisasi Sensor Magnet dan ultrasonik
// RS485 control pins for Magnet sensors (Serial1)
#define MAX485_DE 36
#define MAX485_RE 36
// RS485 Serial Pins for Magnet sensors (Serial1)
#define RS485_RX 18
#define RS485_TX 17

// RS485 Serial Pins for Ultrasonic sensors (Serial2)
#define RS485_RX2 11  // Pin 11 untuk RX Serial2
#define RS485_TX2 46  // Pin 46 untuk TX Serial2

int jumlahMagnet[16];

ModbusMaster magnetNode;     // ModbusMaster untuk sensor magnet (Serial1)
ModbusMaster ultrasonicNode; // ModbusMaster untuk sensor ultrasonik (Serial2)

// ## VARIABLE ##
// # variable Web Server
// WiFi configuration variables (can be modified via web interface)
char ssid[32] = "My Phone";
char password[64] = "kalolaparmakan";
char staticIPStr[16] = "192.168.121.14";
char gatewayStr[16] = "192.168.121.99";
char subnetStr[16] = "255.255.255.0";
char dnsStr[16] = "192.168.121.99";

// IP Address objects (will be updated from string values)
IPAddress staticIP(192, 168, 121, 14);
IPAddress gateway(192, 168, 121, 99);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 121, 99);

const char* PREFERENCES_NAMESPACE = "device_data";
const char* STATIONS_NAMESPACE = "stations";  // Namespace untuk menyimpan station yang ditemukan

// // # Variable Nilai Encoder

// int encKananAVal = 0;
// int encKananBVal = 0;
// int encKiriAVal = 0;
// int encKiriBVal = 0;

// // # Variable RPM
// int rpmKanan = 0;
// int rpmKiri = 0;
// int rpm1, rpm2;

// # PWM Configuration - ESP32 v3.x Migrasi
// # PWM Resolution (masih sama di v3.x)
const int pwmResolution = 12;

// # PWM Frequency (masih sama di v3.x)
const int pwmFrequency = 5000;

// # Max PWM
const int maxPwm = 4096;
const int minPwm = -4096;

// # Pid
const int numOutputs = 10;
struct PIDData {
  double error;
  double integral;
  double derivative;
  double previousError;
  double lastDerivative;
  double lastOutput;
  unsigned long lastComputeTime;
};

PIDData pidData[numOutputs];
// rpm
int pwmKanan, pwmKiri;

// K 0.5 1.5 0.0
double kp = 0.2, ki = 0.4, kd = 0.0;


// int maxrpm = 900;
// int minrpm = -900;
int errorValue = 0;
// #Timing Millis()
unsigned long previousMillis = 0;
const unsigned long interval = 100;

// #variable pembacaanpos
// Deteksi kasus khusus

int totalSensorAktif = 0;
int buttonStep = 0;  // Track button state for sequential actions

int baseSpeed = 2000;
int pidSpeed = 0;

// RFID
#define PIN_D0 12
#define PIN_D1 13

// Wiegand object declaration
Wiegand wiegand;

// RFID Station Management
const int MAX_RFID_STATIONS = 50;
struct RfidStation {
  int stationId;
  String rfidId;
  bool isActive;
};
RfidStation rfidStations[MAX_RFID_STATIONS];
int rfidStationCount = 0;

// RFID Ujung and Warehouse Management
const int MAX_RFID_UJUNG = 2;
const int MAX_RFID_WAREHOUSE = 1;

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

bool updatestations = false;

RfidUjung rfidUjungList[MAX_RFID_UJUNG];
RfidWarehouse rfidWarehouseList[MAX_RFID_WAREHOUSE];
int rfidUjungCount = 0;
int rfidWarehouseCount = 0;
bool isUjungSlowMode = false;  // false = cepat, true = lambat
unsigned long lastUjungDetectionTime = 0;  // Timer untuk debounce ujung RFID
const unsigned long UJUNG_IGNORE_DURATION = 5000;  // Durasi debounce 2 detik (2000ms)

// Auto input station removed - using existing RfidStation structure

// RFID scanning variables
bool isScanning = false;
int currentScanStation = 0;

// Pin Relay music 7, 15, 16, 14, 37, 38
#define pinMusic1 7
#define pinMusic2 15
#define pinMusic3 16
#define pinMusic4 14
#define pinMusic5 37
#define pinMusic6 2

bool statusMusic = false;

// pin hook 20 dan 19, menggunakan relay
#define pinHook1 20
#define pinHook2 45
#define pinMotorHook 21

// Motor inversion settings
bool invertMotorY = false;      // Invers maju-mundur (forward/backward)
bool invertMotorX = false;      // Invers kiri-kanan (left/right)
bool invertMotorKanan = false;  // Invers motor kanan individual
bool invertMotorKiri = false;   // Invers motor kiri individual
bool invertHook = false;        // Invers hook naik-turun

// Debug configuration - uncomment to enable debug output
#define DEBUG_MAGNET          // Enable magnet sensor debug output
#define DEBUG_PID             // Enable PID controller debug output
// #define DEBUG_ULTRASONIC      // Enable ultrasonic sensor debug output
// #define DEBUG_OBSTACLES       // Enable obstacle detection debug output

// Music pin mapping settings (0=pinMusic1, 1=pinMusic2, 2=pinMusic3, 3=pinMusic4, 4=pinMusic5, 5=pinMusic6/Silent)
int musicOnPin = 0;        // Default: pinMusic1 untuk on
int musicObstaclePin = 1;  // Default: pinMusic2 untuk obstacle
int musicStationPin = 2;   // Default: pinMusic3 untuk station
int musicOutOfLinePin = 3; // Default: pinMusic4 untuk out of line
int musicWarningPin = 4;   // Default: pinMusic5 untuk warning
// musicCustom1Pin dan musicCustom2Pin dihapus karena tidak digunakan

// Enum for music modes
enum MusicMode {
  MUSIC_MODE_ON,
  MUSIC_MODE_OBSTACLE,
  MUSIC_MODE_STATION,
  MUSIC_MODE_OUTOFLINE,
  MUSIC_MODE_WARNING
};

// Variable to track current music mode
MusicMode currentMusicMode = MUSIC_MODE_ON;

// ===== MUSIC FUNCTIONS =====
void music(MusicMode mode);
void stopMusic();
void silentMusic();  // Fungsi untuk mengaktifkan pin 6 (Silent)

static bool forceLeft = false;
static bool inLine = true;

// Error codes definition
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

// Hook control function

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

HookPosition hookPosition = STOP_POS;
HookPositionMode hookPositionMode = STOP_HOOK;
HookPosition hook(HookPositionMode mode);





// AGV mode control
bool agvStopCalled = false;






String motorControllerBuffer = "";
bool motorControllerStringComplete = false;

// Startup PID synchronization flags
bool pidDataReceived = false;           // Flag untuk menandakan PID data telah diterima dari slave (legacy)
bool pidDataReceivedRight = false;      // Flag untuk PID data motor kanan
bool pidDataReceivedLeft = false;       // Flag untuk PID data motor kiri
bool systemReadyToRun = false;         // Flag untuk menandakan sistem siap masuk loop
unsigned long pidRequestStartTime = 0; // Timestamp untuk timeout PID request
const unsigned long PID_REQUEST_TIMEOUT = 15000; // Timeout 15 detik untuk PID request

// ===================================================================
//                     EXTERNAL VARIABLE DECLARATIONS
// ===================================================================
extern int targetStation[2];
extern double kp, ki, kd;
extern int baseSpeed;
extern bool tombolBoot;
extern int selectedItem;
extern int maxItems;
extern bool isAgvMode;

// ===================================================================
//                        MENU FUNCTION DECLARATIONS
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

// Motor Control Functions
void rpmMotor(int rpmKiri, int rpmKanan);
void motorStop();

#endif // CONFIG_H