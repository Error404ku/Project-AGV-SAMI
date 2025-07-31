#ifndef CONFIG_H
#define CONFIG_H
// Include Library
#include <Wire.h>
#include <Arduino.h>
#include <math.h>
#include <Wire.h>
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
#include "menu.h"
#include <Wiegand.h>

enum AgvState {
  AGV_STATE_MOVE_FORWARD,
  AGV_STATE_MOVE_BACKWARD,
  AGV_STATE_STOP,
  AGV_STATE_TERMINAL_PICKUP,
  AGV_STATE_TERMINAL_DROP,
  AGV_STATE_WAREHOUSE,
  AGV_STATE_STATION,
  AGV_STATE_NULL
};

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
int BAUDRATE = 9600;

// --- SENSOR MAGNET VARIABLES ---
int currentMagnetSlaveId = SLAVEID_MAGNET_DEPAN;

// --- SENSOR ULTRASONIK VARIABLES ---
bool obstacleDetected = false;
unsigned long lastObstacleCheck = 0;
const unsigned long obstacleCheckInterval = 100;  // Check every 100ms
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

// --- PID CONTROLLER VARIABLES ---
float pidError = 0;
float lastError = 0;
float integral = 0;
float derivative = 0;
bool sudahStopPelanPelan = false;

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
AgvState moveStateAgv = AGV_STATE_STOP;

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
Timer magnetSwitchTimer = {0, 100, false, false};
Timer buttonDebounceTimer = {0, 300, false, false};
Timer menuDelayTimer = {0, 1500, false, false};
Timer errorRecoveryTimer = {0, 5000, false, false};
Timer performanceTimer = {0, 5000, false, false};

// Sensor distances array
uint16_t ultrasonicDistances[5] = { 0 };  // Store distances from 5 probes
uint16_t minSafeDistance = 30;            // cm - minimum safe distance

// --- RFID TERMINAL VARIABLES ---
String terminalDropRfidId = "";
String terminalPickUpRfidId = "";
String ujungRfidId = "";
bool exceptErrorPosition = false;

// --- WAREHOUSE & UJUNG RFID VARIABLES ---
String warehouseRfidId = "";

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
const int maxMusicItems = 4;
const unsigned long WIFI_CONNECT_TIMEOUT = 3000; // Optimized to 3 seconds
const int WIFI_MAX_SCROLL = 3; // Maximum scroll positions (0-3 existing)

// Menu refresh control
bool menuNeedsRefresh = true;
int lastSelectedItem = -1;
int lastMenuStartIndex = -1;

// Temporary variables for settings
double tempKp = 70.0;          // Will be initialized from kpLinefollower
double tempKi = 0.0;           // Will be initialized from kiLinefollower
double tempKd = 0.0;           // Will be initialized from kdLinefollower
int tempBaseSpeed = 2000;      // Will be initialized from baseSpeed

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
float pidIncrement = 0.1f;

// RFID menu variables
int selectedRfidItem = 0;
int selectedStationId = 1;
bool isWaitingForRfid = false;
unsigned long rfidScanTimeout = 0;

// Target settings variables
unsigned long xButtonHoldStart = 0;
bool isClearingStations = false;

// Motor invert settings (temporary)
bool tempInvertY = false;      // Will be initialized from invertMotorY
bool tempInvertX = false;      // Will be initialized from invertMotorX
bool tempInvertKanan = false;  // Will be initialized from invertMotorKanan
bool tempInvertKiri = false;   // Will be initialized from invertMotorKiri
bool tempInvertHook = false;   // Will be initialized from invertHook

// Music mapping settings (temporary)
int tempMusicStationPin = 0;   // Will be initialized from musicStationPin
int tempMusicErrorPin = 1;     // Will be initialized from musicErrorPin
int tempMusicDetectPin = 2;    // Will be initialized from musicDetectPin
int tempMusicKomputerPin = 3;  // Will be initialized from musicKomputerPin

// Motor invert menu variables
int selectedInvertItem = 0;  // 0=Y-axis, 1=X-axis, 2=Motor Kanan, 3=Motor Kiri, 4=Hook

// Music settings variables
int selectedMusicItem = 0;  // 0=Station, 1=Error, 2=Detect, 3=Komputer

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

void setupSensorMagnet(int slaveId);
void setupUltrasonikWithParams(int slaveId);
void setupRS485(int baudrate);

// ### DEFINE ###
// # TOMBOL
// #define tombol 6  // Pin analog lama (tidak digunakan lagi)
// BOOT_PIN definition moved to top of file

// Individual button pins (manual assignment) - definitions moved to top of file

// Available pins for button calibration (not used with manual assignment)
const int availablePins[] = { 39, 40, 41, 42, 2, 1 };
const int availablePinsCount = 6;

// Current button pin assignments (manual fixed values)
extern int currentPinUp;
extern int currentPinLeft;
extern int currentPinRight;
extern int currentPinDown;
extern int currentPinStart;
extern int currentPinStop;

#define sdaPin 3
#define sclPin 8
// LCD I2C
#define LCD_COLUMNS 16    // Jumlah kolom LCD
#define LCD_ROWS 4        // Jumlah baris LCD
#define LCD_ADDRESS 0x27  // Alamat I2C LCD (biasanya 0x27 atau 0x3F)
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
// #Interrupt
#define lampPin 47

// #Inisialisasi Pin Motor L298N
#define IN1 48  // Motor kanan direction 1
#define IN2 45  // Motor kanan direction 2
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
#define MAX485_DE 36
#define MAX485_RE 36
// RS485 Serial Pins (shared for all sensors)
#define RS485_RX 18
#define RS485_TX 17

// Mapping Slave ID ke Sensor - definitions moved to top of file

// Hapus/abaikan pin RX/TX sensor lain (semua pakai RS485_RX dan RS485_TX)
// #define RX_MAGNET_FRONT 11//3
// #define TX_MAGNET_FRONT 10//8
// #define RX_ULTRASONIK_FRONT 18
// #define TX_ULTRASONIK_FRONT 17
// #define RX_MAGNET_BACK 3//11
// #define TX_MAGNET_BACK 8//10
// #define RX_ULTRASONIK_BACK 9
// #define TX_ULTRASONIK_BACK 46

// #Inisialisasi Pin Hook Motor
#define MOTOR_DI1_PIN 20
#define MOTOR_DI2_PIN 19
#define MOTOR_PWM_PIN 21
#define HOOK_PWM_CHANNEL 2


// Alamat slave sensor yang diharapkan
const byte SENSOR_ADDRESS = 0x01;
const int PACKET_LENGTH = 15;
byte dataPacket[PACKET_LENGTH];
int byteCounter = 0;
bool inPacket = false;

int jumlahMagnet[16];

ModbusMaster node;

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

// # Pin Channel PWM
const int channelKanan = 0;
const int channelKiri = 1;

// # PWM Resolution
const int pwmResolution = 12;

// # PWM Frequency
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
unsigned long milisRpm = 0;
int intervalRpm = 200;

int perRotasi = 230;
// rpm
int pwmKanan, pwmKiri;

// K 0.5 1.5 0.0
double kp = 0.2, ki = 0.4, kd = 0.0;
float kpLinefollower = 70.0;  // Sesuaikan dengan kebutuhan
float kiLinefollower = 0.0;
float kdLinefollower = 0.0;
// int pwm_min = -1023;
// int pwm_zero = 0;
// int pwm_max = 1023;

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

// RFID
#define PIN_D0 12
#define PIN_D1 13

// Wiegand object declaration
Wiegand wiegand;

// RFID Station Management
const int MAX_RFID_STATIONS = 100;
struct RfidStation {
  int stationId;
  String rfidId;
  bool isActive;
};
RfidStation rfidStations[MAX_RFID_STATIONS];
int rfidStationCount = 0;

// RFID Ujung and Warehouse Management
const int MAX_RFID_UJUNG = 10;
const int MAX_RFID_WAREHOUSE = 10;

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

RfidUjung rfidUjungList[MAX_RFID_UJUNG];
RfidWarehouse rfidWarehouseList[MAX_RFID_WAREHOUSE];
int rfidUjungCount = 0;
int rfidWarehouseCount = 0;

// Auto input station data
struct AutoInputStation {
  int stationId;
  String rfidId;
  bool isActive;
};
const int MAX_AUTO_STATIONS = 20;
AutoInputStation autoStations[MAX_AUTO_STATIONS];
int autoStationCount = 0;

// RFID scanning variables
bool isScanning = false;
int currentScanStation = 0;
// String lastScannedRfid = ""; // Replaced with optimized char array
extern char lastScannedRfidOptimized[32];  // Optimized RFID storage
bool newRfidScanned = false;

// Obstacle detection variables
extern bool obstacleDetected;
extern uint16_t ultrasonicDistances[5];

// Pin Relay music 7, 15, 16, 14
#define pinMusic1 7
#define pinMusic2 15
#define pinMusic3 16
#define pinMusic4 14

bool statusMusic = false;

// pin hook 20 dan 19, menggunakan relay
#define pinHook1 20
#define pinHook2 19
#define pinMotorHook 21

// Motor inversion settings
bool invertMotorY = false;      // Invers maju-mundur (forward/backward)
bool invertMotorX = false;      // Invers kiri-kanan (left/right)
bool invertMotorKanan = false;  // Invers motor kanan individual
bool invertMotorKiri = false;   // Invers motor kiri individual
bool invertHook = false;        // Invers hook naik-turun

// Music pin mapping settings (0=pinMusic1, 1=pinMusic2, 2=pinMusic3, 3=pinMusic4)
int musicStationPin = 0;   // Default: pinMusic1 untuk station
int musicErrorPin = 1;     // Default: pinMusic2 untuk error
int musicDetectPin = 2;    // Default: pinMusic3 untuk detect
int musicKomputerPin = 3;  // Default: pinMusic4 untuk komputer

// Enum for music modes
enum MusicMode {
  MUSIC_MODE_STATION,
  MUSIC_MODE_ERROR,
  MUSIC_MODE_DETECT,
  MUSIC_MODE_KOMPUTER
};


enum moveStateAGV {
  LAST_STATE_MOVE_FORWARD,
  LAST_STATE_MOVE_BACKWARD
};
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

// ===== ULTRASONIC SENSOR FUNCTIONS =====
void setUltrasonicSlaveId(int slaveId);
void initUltrasonicSensor(int slaveId);
void switchUltrasonicSensor(bool useFrontSensor);
int getCurrentUltrasonicSlaveId();
void loopUltrasonik();
void checkObstacles();
void preTransmissionUltrasonic();
void postTransmissionUltrasonic();

// ===== MAGNET SENSOR FUNCTIONS =====
void bacaSensor();
void bacaSensor(int slaveId);
void switchMagnetSensor(bool useFrontSensor);
int getCurrentMagnetSlaveId();
void setMagnetSlaveId(int slaveId);

// ===== WIFI CONFIGURATION FUNCTIONS =====
bool saveWifiConfig(const String& ssid, const String& password, const String& staticIP, const String& gateway, const String& subnet, const String& dns);
bool loadWifiConfig();
void updateIPAddressesFromStrings();
void handleWifiConfig();
void handleSaveWifi();
void handleRoot();
void handleUpdateTargetStations();
void handleShowTargetStations();
void handleShowStationAddresses();
void handleShowUjungStations();
void handleShowWarehouseRfid();
void handleShowTerminalRfid();
bool saveTargetStationsListToPreferences();
bool loadTargetStationsListFromPreferences();
void clearTargetStationsData();
void sortTargetStationsList();
bool removeTargetStationById(int stationId);

// WiFi connection management functions
void setupWifi();           // Dipanggil di setup()
void startWifiConnection(); // Dipanggil saat tombol START ditekan
void loopWifi();            // Dipanggil di loop() jika diperlukan

// RFID Ujung functions
void displayRfidUjung();
void handleRfidUjung();
void saveRfidUjungToPreferences();
void loadRfidUjungFromPreferences();

// RFID Warehouse functions
void displayRfidWarehouse();
void handleRfidWarehouse();
void saveRfidWarehouseToPreferences();
void loadRfidWarehouseFromPreferences();

// Warehouse & Ujung RFID sync functions
void loadWarehouseUjungRfid();
void saveWarehouseRfid(String rfidId);
void saveUjungRfid(String rfidId);

// Terminal Drop & Pickup RFID functions
void loadTerminalRfid();
void saveTerminalDropRfid(String rfidId);
void saveTerminalPickUpRfid(String rfidId);

// AGV Movement functions
void agvMode(AgvState state);
void agvMoveForward();
void agvMoveBackward();
void agvWarehouse();
void agvStation();
void agvTerminalPickup();
void agvTerminalDrop();
void agvStop();
void moveStateAGV(AgvState lastState);
String agvStateToString(AgvState state);
AgvState stringToAgvState(String stateString);
void saveCurrentStateAGVToPreferences(AgvState currentState);
// AgvState loadCurrentStateAGVFromPreferences();
void savemoveStateAGVToPreferences(AgvState lastState);
// AgvState loadmoveStateAGVFromPreferences();
void loadAllAGVStatesFromPreferences();

// Hook control function
void hook(String command);

// PID and motor control functions
enum PidMode {
  PID_MODE_MAJU,
  PID_MODE_MUNDUR,
  PID_MODE_FORCEMUNDUR,
  PID_MODE_FORCEMAJU,
  PID_MODE_STOPPELANPELAN,
  PID_MODE_BERHENTI,
  PID_MODE_DEFAULT
};
void pidLinefollower(int error, PidMode mode);
void pwmMotor(int leftSpeed, int rightSpeed);

// Button functions
bool START();
bool STOP();
bool UP();
bool DOWN();
bool LEFT();
bool RIGHT();

// AGV mode control
extern bool isAgvMode;
bool agvStopCalled = false;

// Except error position flag - untuk mengabaikan error setelah warehouse terdeteksi
extern bool exceptErrorPosition;
void resetExceptErrorFlag();
void saveExceptErrorFlag();
void loadExceptErrorFlag();

// Terminal and display functions
// void inTerminal();
void displayPrint();
void displaySensorData();
void lamp_flip_flop();
void handleMenu();
void loopRfid();
void setupAll();

// Auto Input Station functions
void displayAutoInputStation();
void handleAutoInputStation();
void saveAutoStationsToPreferences();
void loadAutoStationsFromPreferences();
bool isStationExists(String rfidData);

// Menu initialization function
void initMenuTempVariables();

// ===== PERFORMANCE OPTIMIZATION FUNCTIONS =====
// Timer system
struct Timer;
extern Timer stopPelanPelanTimer;
extern Timer ultrasonicSwitchTimer;
extern Timer magnetSwitchTimer;
extern Timer buttonDebounceTimer;
extern Timer menuDelayTimer;
extern Timer errorRecoveryTimer;
extern Timer performanceTimer;

void startTimer(Timer* timer, unsigned long interval);
void stopTimer(Timer* timer);
bool checkTimer(Timer* timer);
bool isTimerActive(Timer* timer);
bool wasTimerTriggered(Timer* timer);

// Performance monitoring
void startPerformanceMonitoring();
void endPerformanceMonitoring();
void printPerformanceStats();
void resetPerformanceStats();
void initPerformanceOptimization();
void updatePerformanceOptimization();

// Optimized state management
void setStatusJalan(const char* status);
void setCurrentMode(const char* mode);
const char* getStatusJalan();
const char* getCurrentMode();

// Error recovery system
extern bool systemInErrorState;
bool attemptErrorRecovery(int errorCode);
bool recoverSensorCommunication();
bool recoverMotorControl();
bool recoverRfidCommunication();
bool recoverWifiConnection();
void checkErrorRecovery();
void initErrorRecovery();


// Global AGV state tracking variables
extern AgvState currentStateAgv;
extern AgvState moveStateAgv;
// String variables ujungRfidId, terminalDropRfidId, terminalPickUpRfidId already defined above



int getStationFromLastRfid();
void clearAllRfidStations();
bool deleteRfidStation(int stationId);
bool addRfidStation(int stationId, String rfidId);
#endif