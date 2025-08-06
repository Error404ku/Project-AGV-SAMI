#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <Preferences.h>
#include <Adafruit_SSD1306.h>

// Menu states
#define MENU_MAIN 0
#define MENU_MOTOR_TEST 2
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
#define MENU_HOOK_TEST 22
#define MENU_MAGNET_CHECK 23
#define MENU_ULTRASONIC_CHECK 24
#define MENU_RESET_AGV_STATE 25
#define MENU_ULTRASONIC_SETTINGS 30
#define MENU_ULTRASONIC_FRONT 31
#define MENU_ULTRASONIC_BACK 32
#define MENU_PID_FORWARD 28
#define MENU_PID_BACKWARD 29

// External variables from other files
extern int targetStation[2];
extern int targetStationFromKomputer[];
extern bool modeMaju;
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
void displayMotorTest();
void displayPidSettings();
void displayTargetSettings();
void displayWifiSettings();
void handleMotorTest();
void handlePidSettings();
void handlePidSubmenu();
void handlePidForwardSettings();
void handlePidBackwardSettings();
void handleTargetSettings();
void handleWifiSettings();

void displayPidSubmenu();
void displayPidForwardSettings();
void displayPidBackwardSettings();
void displayUltrasonicSettings();
void handleUltrasonicSettings();
void displayUltrasonicFrontSettings();
void displayUltrasonicBackSettings();
void handleUltrasonicFrontSettings();
void handleUltrasonicBackSettings();

#endif