// Include Library
#include <Wire.h>
#include <Arduino.h>
#include <math.h>
#include <Adafruit_SSD1306.h>  // Memanggil Library OLED SSD1306
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

// --- Deklarasi Global ---
WebServer server(80);
Preferences preferences; // Untuk deviceMap
Preferences stationsPreferences; // Objek Preferences untuk station yang ditemukan
std::unordered_map<int, std::vector<String>> deviceMap;
std::vector<int> stationsList; // Array di RAM untuk menyimpan station yang ditemukan

// ### DEFINE ###

// # TOMBOL
#define tombol 6
#define BOOT_PIN 0
// #Oled I2C
#define SCREEN_WIDTH 128  // Lebar Oled dalam Pixel
#define SCREEN_HEIGHT 64  // Tinggi Oled dalam Pixel
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// #Interrupt
#define interruptPin 47

// #Inisialisasi Pin Motor
#define mdKananA 1
#define mdKananB 2
#define mdKiriA 40
#define mdKiriB 41

// #Inisialisasi Pin Encoder
#define encKananA 42
#define encKananB 37
#define encKiriA 39
#define encKiriB 48

// #Inisialisasi Sensor Magnet
#define MAX485_DE 35
#define MAX485_RE 35
#define RXD2 18
#define TXD2 17
#define BAUDRATE 9600

int jumlahMagnet[16];

ModbusMaster node;

// ## VARIABLE ##
// # variable Web Server
const char* ssid = "Sabila";
const char* password = "11111111";
IPAddress staticIP(192, 168, 106, 150);
IPAddress gateway(192, 168, 106, 115);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 106, 115); // Gunakan gateway sebagai DNS

const char* PREFERENCES_NAMESPACE = "device_data";
const char* STATIONS_NAMESPACE = "stations"; // Namespace untuk menyimpan station yang ditemukan

// # Variable Nilai Encoder

int encKananAVal = 0;
int encKananBVal = 0;
int encKiriAVal = 0;
int encKiriBVal = 0;

// # Variable RPM
int rpmKanan = 0;
int rpmKiri = 0;
int rpm1, rpm2;

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
int pwm_min = -1023;
int pwm_zero = 0;
int pwm_max = 1023;

int maxrpm = 900;
int minrpm = -900;
int errorValue = 0;
// #Timing Millis()
unsigned long previousMillis = 0;
const unsigned long interval = 100;

// #variable pembacaanpos
// Deteksi kasus khusus

int totalSensorAktif = 0;
int buttonStep = 0;  // Track button state for sequential actions

int baseSpeed = 1000;

void setup() {
  Serial.begin(115200);
  setupAll();
  setupMenu();  // Initialize menu system
  setupTombol();
  display.clearDisplay();
  Serial.println("SETUP SELESAI");
}

void loop() {
  server.handleClient();

  if (isAgvMode) {
    // AGV Mode - Run normal AGV operation
    pembacaanRpm();
    displayPrint();
    bacaSensorGaris();
    logicAgv();

    // Check for B button to exit AGV mode
    if (B()) {
      isAgvMode = false;
      modeBerhenti = true;
      buttonStep = 0;  // Reset button step
    }
  } else {
    // Menu Mode
    inTerminal();
    handleMenu();
  }
  display.display();
}
