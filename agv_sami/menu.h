#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <Preferences.h>
#include <Adafruit_SSD1306.h>

// Menu states are now defined as enum in menu.ino for better type safety

// External variables from other files
extern int targetStation[2];
extern int targetStationFromKomputer[];
extern double kp, ki, kd;
extern int baseSpeed;
extern bool tombolBoot;
// Menu variables
extern int selectedItem;
extern int maxItems;
extern bool isAgvMode;
int currentMenu = MENU_MAIN;

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
void handleTargetSettings();
void handleWifiSettings();

#endif