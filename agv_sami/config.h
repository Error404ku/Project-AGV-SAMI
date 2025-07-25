#ifndef CONFIG_H
#define CONFIG_H
// Include Library
#include <Wire.h>
#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // ESP32 LCD I2C Library
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

// --- Deklarasi Global ---
WebServer server(80);
Preferences preferences;
Preferences stationsPreferences; // Objek Preferences untuk station yang ditemukan
std::vector<int> stationsList; // Array di RAM untuk menyimpan station yang ditemukan

// Unified RS485 configuration
int BAUDRATE_RS485 = 9600;

// Device addresses for unified RS485 communication
#define ADDR_MAGNET_FRONT 1
#define ADDR_ULTRASONIC_FRONT 2  
#define ADDR_ULTRASONIC_BACK 3
#define ADDR_MAGNET_BACK 4

// Function declarations for unified RS485 system
void setupUnifiedRS485();
void loopUnifiedRS485();
void setupUltrasonikWithParams(int rx, int tx, int baudrate);
int* getCurrentMagnetData();
int* getMagnetData(bool useFront);
uint16_t* getUltrasonicData(bool useFront);
bool isDeviceOnline(int deviceIndex);
String getDeviceStatusString();
void updateMagnetData(uint16_t bitmask, int* magnetArray);
void checkObstaclesFront();
void checkObstaclesBack();
bool hasObstacle(bool checkFront);
uint16_t calculate_crc(byte* buffer, int len);

// ### DEFINE ###
// # TOMBOL
// #define tombol 6  // Pin analog lama (tidak digunakan lagi)
#define BOOT_PIN 0

// Individual button pins (manual assignment)
#define PIN_UP 39     // UP button
#define PIN_LEFT 41   // LEFT button  
#define PIN_RIGHT 42  // RIGHT button
#define PIN_DOWN 40   // DOWN button
#define PIN_START 2   // START button
#define PIN_STOP 1    // STOP button

// Available pins for button calibration (not used with manual assignment)
const int availablePins[] = {39, 40, 41, 42, 2, 1};
const int availablePinsCount = 6;

// Current button pin assignments (manual fixed values)
extern int currentPinUp;
extern int currentPinLeft;
extern int currentPinRight;
extern int currentPinDown;
extern int currentPinStart;
extern int currentPinStop;

// LCD I2C
#define LCD_COLUMNS 20    // Jumlah kolom LCD
#define LCD_ROWS 4      // Jumlah baris LCD
#define LCD_ADDRESS 0x27 // Alamat I2C LCD (biasanya 0x27 atau 0x3F)
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
// #Interrupt
// #define interruptPin 47

// #Inisialisasi Pin Motor L298N
#define IN1 48    // Motor kanan direction 1
#define IN2 45    // Motor kanan direction 2  
#define IN3 4   // Motor kiri direction 1
#define IN4 5   // Motor kiri direction 2
#define ENA 35   // Motor kanan enable/PWM
#define ENB 6   // Motor kiri enable/PWM

// // #Inisialisasi Pin Encoder
// #define encKananA 42
// #define encKananB 37
// #define encKiriA 39
// #define encKiriB 48

// #Inisialisasi Pin Sensor - Unified RS485
#define MAX485_DE 36
#define MAX485_RE 36
// Unified RS485 pins for all sensors
#define RX_RS485 18
#define TX_RS485 17

// #Inisialisasi Pin Hook Motor dengan SSR Relay
#define HOOK_RELAY_PIN 21        // Pin untuk relay SSR-40 DA
#define LIMIT_SWITCH_UP_PIN 20   // Pin untuk limit switch atas
#define LIMIT_SWITCH_DOWN_PIN 19 // Pin untuk limit switch bawah


// Unified RS485 communication variables
const int PACKET_LENGTH = 15;
byte dataPacket[PACKET_LENGTH];
int byteCounter = 0;
bool inPacket = false;

// Current active device address
byte currentDeviceAddress = ADDR_MAGNET_FRONT;

// Sensor data arrays
int jumlahMagnetFront[16];
int jumlahMagnetBack[16];
uint16_t ultrasonicDistancesFront[5] = {0};
uint16_t ultrasonicDistancesBack[5] = {0};

// Modbus master instances
ModbusMaster nodeMagnetFront;
ModbusMaster nodeMagnetBack;

// ## VARIABLE ##
// # variable Web Server
const char* ssid = "My Phone";
const char* password = "kalolaparmakan";
IPAddress staticIP(192, 168, 121, 14);
IPAddress gateway(192, 168, 121, 99);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 121, 99); // Gunakan gateway sebagai DNS

const char* PREFERENCES_NAMESPACE = "device_data";
const char* STATIONS_NAMESPACE = "stations"; // Namespace untuk menyimpan station yang ditemukan

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

int baseSpeed = 1000;

// RFID 
#define PIN_D0 12
#define PIN_D1 13

// Wiegand object declaration
Wiegand wiegand;

// RFID Station mapping variables
const int MAX_RFID_STATIONS = 10;
struct RfidStation {
  int stationId;
  String rfidId;
  bool isActive;
};
RfidStation rfidStations[MAX_RFID_STATIONS];
int rfidStationCount = 0;

// RFID scanning variables
bool isScanning = false;
int currentScanStation = 0;
String lastScannedRfid = "";
bool newRfidScanned = false;

// Obstacle detection variables
extern bool obstacleDetected;

// Pin Relay music 7, 15, 16, 14
#define pinMusic1 7
#define pinMusic2 15
#define pinMusic3 16
#define pinMusic4 14

bool statusMusic = false;

// pin hook dengan SSR relay dan limit switches
#define pinHookRelay 21          // Pin relay SSR untuk kontrol hook motor
#define pinLimitUp 20            // Pin limit switch atas
#define pinLimitDown 19          // Pin limit switch bawah

// Motor inversion settings
bool invertMotorY = false;  // Invers maju-mundur (forward/backward)
bool invertMotorX = false;  // Invers kiri-kanan (left/right)
bool invertMotorKanan = false;  // Invers motor kanan individual
bool invertMotorKiri = false;   // Invers motor kiri individual
bool invertHook = false;        // Invers hook naik-turun

// Music pin mapping settings (0=pinMusic1, 1=pinMusic2, 2=pinMusic3, 3=pinMusic4)
int musicStationPin = 0;    // Default: pinMusic1 untuk station
int musicErrorPin = 1;      // Default: pinMusic2 untuk error  
int musicDetectPin = 2;     // Default: pinMusic3 untuk detect
int musicKomputerPin = 3;   // Default: pinMusic4 untuk komputer

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

#endif