#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <Preferences.h>
#include <Adafruit_SSD1306.h>

// Menu states
#define MENU_MAIN 0
#define MENU_MOTOR_TEST 2  // Motor Test changed to submenu
#define MENU_PID_SETTINGS 3
#define MENU_TARGET_SETTINGS 4
#define MENU_AGV_MODE 1
#define MENU_RESET 5
#define MENU_RFID_SETTINGS 6
#define MENU_WIFI_SETTINGS 14
#define MENU_MOTOR_SETTINGS 18
#define MENU_MOTOR_INVERT 19
#define MENU_MUSIC_SETTINGS 20
#define MENU_MUSIC_TEST 21
#define MENU_MUSIC_ON 40
#define MENU_MUSIC_OBSTACLE 41
#define MENU_MUSIC_STATION 42
#define MENU_MUSIC_OUTOFLINE 43
#define MENU_MUSIC_WARNING 44
#define MENU_HOOK_TEST 22
#define MENU_MAGNET_CHECK 23
#define MENU_ULTRASONIC_CHECK 24
#define MENU_RESET_AGV_STATE 25
#define MENU_ULTRASONIC_SETTINGS 30
#define MENU_ULTRASONIC_FRONT 31
#define MENU_ULTRASONIC_BACK 32
#define MENU_ULTRASONIC_FRONT_TENGAH 51
#define MENU_ULTRASONIC_FRONT_SERONG 52
#define MENU_ULTRASONIC_BACK_TENGAH 53
#define MENU_ULTRASONIC_BACK_SERONG 54
#define MENU_PID_FORWARD 28
#define MENU_PID_BACKWARD 29
#define MENU_PID_FORWARD_WITHMASSA 34
#define MENU_PID_FORWARD_DEFAULT 35
#define MENU_PID_BACKWARD_WITHMASSA 36
#define MENU_PID_BACKWARD_DEFAULT 37

// Motor Settings Sub Menu
#define MENU_SPEED_SETTING 46
#define MENU_PID_RPM_SETTING 47

// Motor Test Sub Menu (MENU_MOTOR_TEST is now 2, defined above)
#define MENU_MOTOR_TEST_PWM 49
#define MENU_MOTOR_TEST_RPM 50

// External variables from other files
extern int targetStation[2];
// extern int targetStationFromKomputer[]; // Removed - not used
// extern bool modeMaju; // Removed - not used
extern double kp, ki, kd;
extern int baseSpeed;
extern bool tombolBoot;
// Menu variables
extern int selectedItem;
extern int maxItems;
extern bool isAgvMode;
// ===================================================================
// MENU VARIABLES SUDAH DIPINDAHKAN KE config.h
// ===================================================================

// Function declarations
void setupMenu();
void handleMenu();
void displayMainMenu();
void displayMotorTest();  // Changed from displayMotorTest to submenu
void displayPidSettings();
void displayTargetSettings();
void displayWifiSettings();
void handleMotorTest();  // Changed from handleMotorTest to submenu
void handlePidSettings();
void handlePidSubmenu();

void handleTargetSettings();
void handleWifiSettings();

void displayPidSubmenu();

void displayPidForwardSubmenu();
void displayPidBackwardSubmenu();
void handlePidForwardSubmenu();
void handlePidBackwardSubmenu();
void displayPidForwardWithMassaSettings();
void displayPidForwardDefaultSettings();
void displayPidBackwardWithMassaSettings();
void displayPidBackwardDefaultSettings();
void handlePidForwardWithMassaSettings();
void handlePidForwardDefaultSettings();
void handlePidBackwardWithMassaSettings();
void handlePidBackwardDefaultSettings();
void displayUltrasonicSettings();
void handleUltrasonicSettings();
void displayUltrasonicFrontSettings();
void displayUltrasonicBackSettings();
void handleUltrasonicFrontSettings();
void handleUltrasonicBackSettings();
void displayUltrasonicFrontTengahSettings();
void displayUltrasonicFrontSerongSettings();
void displayUltrasonicBackTengahSettings();
void displayUltrasonicBackSerongSettings();
void handleUltrasonicFrontTengahSettings();
void handleUltrasonicFrontSerongSettings();
void handleUltrasonicBackTengahSettings();
void handleUltrasonicBackSerongSettings();
void displayMusicSettings();
void handleMusicSettings();
void displayMusicSubmenu(const char* title, int* currentPin);
void handleMusicSubmenu(int* targetPin);

// Motor Settings Sub Menu functions (modified from old Motor Settings)
void displayMotorSettings();  // Updated to show sub menu
void handleMotorSettings();   // Updated to handle sub menu navigation
void displaySpeedSetting();
void handleSpeedSetting();
void displayPidRpmSetting();
void handlePidRpmSetting();

// Motor Test Sub Menu functions (displayMotorTest and handleMotorTest declared above)
void displayMotorTestPWM();
void handleMotorTestPWM();
void displayMotorTestRPM();
void handleMotorTestRPM();

// RFID functions that were missing declarations
String getRfidForStation(int stationId);

// Motor functions that were missing declarations  
void rpmMotor(int rpmKiri, int rpmKanan);
void motorStop();

#endif