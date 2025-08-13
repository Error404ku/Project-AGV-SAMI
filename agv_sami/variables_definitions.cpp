#include "config.h"

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

size_t freeHeapSize = 0;
size_t minFreeHeap = 0;

// --- AGV STATE VARIABLES ---
int currentStateAgv = STATE_READY;   // AGV STATE_READY
int moveStateAgv = MOVE_STOP;       // MOVE_STOP
String currentRFID = "";

// --- TIMER VARIABLES ---
SimpleTimer stopPelanPelanTimer;
SimpleTimer ultrasonicSwitchTimer;
SimpleTimer magnetSwitchTimer;
SimpleTimer buttonDebounceTimer;
SimpleTimer menuDelayTimer;
SimpleTimer errorRecoveryTimer;
SimpleTimer performanceTimer;

// --- ULTRASONIC ARRAY VARIABLES ---
int ultrasonicDistances[2] = {0, 0};  // [depan, belakang]
int minSafeDistanceFront = 20; // cm untuk depan
int minSafeDistanceBack = 15;  // cm untuk belakang

// --- RFID VARIABLES ---
String terminalDropRfidId = "";
String terminalPickUpRfidId = "";
bool exceptErrorPosition = false;

String warehouseRfidId = "";
String ujungRfidId = "";
String rfidMajuId = "";

// --- MENU VARIABLES ---
int selectedItem = 0;
int maxItems;
int menuStartIndex = 0;        // For scrolling menu
int maxMenuDisplay = 3;        // Maximum items displayed at once
bool isAgvMode = false;
int currentMenu = 0;           // MENU_MAIN

// Menu refresh and scrolling
bool menuNeedsRefresh = false;
int lastSelectedItem = -1;
int lastMenuStartIndex = -1;

// --- PID TUNING VARIABLES ---
// Test PID Forward Dengan Massa
float tempKpForwardWithMassa = KP_FORWARD_WITH_MASSA;
float tempKiForwardWithMassa = KI_FORWARD_WITH_MASSA;
float tempKdForwardWithMassa = KD_FORWARD_WITH_MASSA;

// Test PID Forward Default
float tempKpForwardDefault = KP_FORWARD_DEFAULT;
float tempKiForwardDefault = KI_FORWARD_DEFAULT;
float tempKdForwardDefault = KD_FORWARD_DEFAULT;

// Test PID Backward Dengan Massa
float tempKpBackwardWithMassa = KP_BACKWARD_WITH_MASSA;
float tempKiBackwardWithMassa = KI_BACKWARD_WITH_MASSA;
float tempKdBackwardWithMassa = KD_BACKWARD_WITH_MASSA;

// Test PID Backward Default
float tempKpBackwardDefault = KP_BACKWARD_DEFAULT;
float tempKiBackwardDefault = KI_BACKWARD_DEFAULT;
float tempKdBackwardDefault = KD_BACKWARD_DEFAULT;

// Current PID values (working copies)
float tempKp = KP;
float tempKi = KI;
float tempKd = KD;
float tempBaseSpeed = BASE_SPEED;

// --- MANUAL TARGET VARIABLES ---
bool useAutoTarget = false;
int manualTargetCount = 0;
int selectedParam = 0; // 0=Kp, 1=Ki, 2=Kd, 3=Base Speed
int selectedTarget = 0; // For selecting target stations

unsigned long lastButtonPress = 0;

// --- PID TEST VARIABLES ---
unsigned long pidButtonHoldStart = 0;
float pidIncrement = 0.1; // Increment for PID parameter adjustment

// --- RFID STATION VARIABLES ---
int selectedRfidItem = 0;
String selectedStationId = "";
bool isWaitingForRfid = false;
unsigned long rfidScanTimeout = 30000; // 30 second timeout

bool isClearingStations = false;
unsigned long xButtonHoldStart = 0;

// --- PARAMETER ADJUSTMENT VARIABLES ---
bool tempInvertY = INVERT_Y;
bool tempInvertX = INVERT_X;
bool tempInvertKanan = INVERT_KANAN;
bool tempInvertKiri = INVERT_KIRI;
bool tempInvertHook = INVERT_HOOK;

int tempMusicOnPin = MUSIC_ON_PIN;
int tempMusicObstaclePin = MUSIC_OBSTACLE_PIN;
int tempMusicStationPin = MUSIC_STATION_PIN;
int tempMusicOutOfLinePin = MUSIC_OUT_OF_LINE_PIN;
int tempMusicWarningPin = MUSIC_WARNING_PIN;

int tempMinSafeDistanceFront = MIN_SAFE_DISTANCE_FRONT;
int tempMinSafeDistanceBack = MIN_SAFE_DISTANCE_BACK;

int selectedInvertItem = 0;
int selectedMusicItem = 0;
int selectedMusicPin = 0;

// --- MOTOR TEST STATE VARIABLES ---
int motorTestState = 0;  // 0=stopped, 1=forward, 2=backward, 3=left, 4=right

// --- HOOK TEST STATE VARIABLES ---
int hookTestState = 0;  // 0=stopped, 1=up, 2=down

// --- WIFI VARIABLES ---
bool isConnectingWifi = false;
int wifiConnectionResult = 0; // 0=none, 1=success, 2=failed, 3=timeout
unsigned long wifiConnectStartTime = 0;
int wifiScrollIndex = 0;

// --- LCD DISPLAY ---
LiquidCrystal_I2C lcd(0x27, 20, 4);  // I2C address 0x27, 20 column and 4 rows

// --- COMMUNICATION VARIABLES ---
byte dataPacket[100];
byte byteCounter = 0;
byte inPacket = 0;
int jumlahMagnet = 0;

// --- I2C SENSOR NODES ---
ModbusMaster magnetNode;
ModbusMaster ultrasonicNode;

// --- WIFI & NETWORK CONFIGURATION ---
char ssid[64] = "";
char password[64] = "";
String staticIPStr = "";
String gatewayStr = "";
String subnetStr = "";
String dnsStr = "";
IPAddress staticIP;
IPAddress gateway;
IPAddress subnet;
IPAddress dns;

// --- PREFERENCES NAMESPACES ---
const char* PREFERENCES_NAMESPACE = "agvPrefs";
const char* STATIONS_NAMESPACE = "stations";

// --- PID VALUES ---
PIDData pidData;
unsigned long milisRpm = 0;
unsigned long intervalRpm = 100;
float perRotasi = 374;

int pwmKanan, pwmKiri;

// Current PID values
double kp = KP, ki = KI, kd = KD;

// PID Controller for Linefollower
double kpLinefollower = KP_LINEFOLLOWER;
double kiLinefollower = KI_LINEFOLLOWER;
double kdLinefollower = KD_LINEFOLLOWER;

// Error calculation
int errorValue = 0;
unsigned long previousMillis = 0;

// --- WIEGAND RFID ---
Wiegand wiegand;

// --- STATION MANAGEMENT ---
RfidStation rfidStations[MAX_STATIONS];
int rfidStationCount = 0;

// --- HOOK POSITION ---
int hookPosition = 0;
int hookPositionMode = 0; // 0 = DOWN, 1 = UP

// --- STOP CALLED ---
bool agvStopCalled = false;

// State variables for PID test toggle
bool pidTestMajuMassaRunning = false;
bool pidTestMundurMassaRunning = false;
bool pidTestMajuDefaultRunning = false;
bool pidTestMundurDefaultRunning = false;
