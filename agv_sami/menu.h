#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <Preferences.h>
#include <Adafruit_SSD1306.h>

// Menu states
#define MENU_MAIN 0
#define MENU_MOTOR_TEST 1
#define MENU_PID_SETTINGS 2
#define MENU_TARGET_SETTINGS 3
#define MENU_AGV_MODE 4

// External variables from other files
extern int targetStation[2];
extern int targetStationFromKomputer[];
extern bool modeStation;
extern bool modeMaju;
extern bool modeBerhenti;
extern bool force;
extern double kp, ki, kd;
extern int baseSpeed;
extern bool tombolBoot;
// Menu variables
extern int currentMenu;
extern int selectedItem;
extern int maxItems;
extern bool isAgvMode;

// Function declarations
void setupMenu();
void handleMenu();
void displayMainMenu();
void displayMotorTest();
void displayPidSettings();
void displayTargetSettings();
void handleMotorTest();
void handlePidSettings();
void handleTargetSettings();

#endif 