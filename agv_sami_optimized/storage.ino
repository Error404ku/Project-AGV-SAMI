/*
  STORAGE.INO - Persistent Storage Management

  This file handles saving and loading system configuration
  to and from the non-volatile storage (LittleFS).
*/

#include <LittleFS.h>
#include <ArduinoJson.h>

const char* CONFIG_FILE = "/config.json";

void setupStorage() {
  if (!LittleFS.begin(true)) { // Format if mount failed
    DEBUG_PRINTLN("LittleFS mount failed. Formatting...");
    logError(ERROR_STORAGE, "LittleFS mount failed");
    return;
  }
  DEBUG_PRINTLN("LittleFS mounted successfully.");
}

void saveConfig() {
  File configFile = LittleFS.open(CONFIG_FILE, "w");
  if (!configFile) {
    DEBUG_PRINTLN("Failed to open config file for writing");
    logError(ERROR_STORAGE, "Failed to save config");
    return;
  }

  StaticJsonDocument<1024> doc;

  // System settings
  doc["baseSpeed"] = systemConfig.baseSpeed;
  doc["maxPwm"] = systemConfig.maxPwm;
  doc["invertMotorX"] = systemConfig.invertMotorX;
  doc["invertMotorY"] = systemConfig.invertMotorY;
  doc["invertMotorKanan"] = systemConfig.invertMotorKanan;
  doc["invertMotorKiri"] = systemConfig.invertMotorKiri;

  // PID settings
  doc["pid_kp"] = systemConfig.pidLinefollower.kp;
  doc["pid_ki"] = systemConfig.pidLinefollower.ki;
  doc["pid_kd"] = systemConfig.pidLinefollower.kd;

  if (serializeJson(doc, configFile) == 0) {
    DEBUG_PRINTLN("Failed to write to config file");
    logError(ERROR_STORAGE, "Failed to serialize config");
  } else {
    DEBUG_PRINTLN("Configuration saved.");
  }
  configFile.close();
}

void loadConfig() {
  File configFile = LittleFS.open(CONFIG_FILE, "r");
  if (!configFile) {
    DEBUG_PRINTLN("Config file not found. Loading default values.");
    // Optional: save default config here if you want
    // saveConfig(); 
    return; // Keep default values
  }

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, configFile);
  if (error) {
    DEBUG_PRINTF("Failed to read config file, using default values. Error: %s\n", error.c_str());
    logError(ERROR_STORAGE, "Failed to parse config");
    return;
  }

  // System settings
  systemConfig.baseSpeed = doc["baseSpeed"] | systemConfig.baseSpeed;
  systemConfig.maxPwm = doc["maxPwm"] | systemConfig.maxPwm;
  systemConfig.invertMotorX = doc["invertMotorX"] | systemConfig.invertMotorX;
  systemConfig.invertMotorY = doc["invertMotorY"] | systemConfig.invertMotorY;
  systemConfig.invertMotorKanan = doc["invertMotorKanan"] | systemConfig.invertMotorKanan;
  systemConfig.invertMotorKiri = doc["invertMotorKiri"] | systemConfig.invertMotorKiri;

  // PID settings
  systemConfig.pidLinefollower.kp = doc["pid_kp"] | systemConfig.pidLinefollower.kp;
  systemConfig.pidLinefollower.ki = doc["pid_ki"] | systemConfig.pidLinefollower.ki;
  systemConfig.pidLinefollower.kd = doc["pid_kd"] | systemConfig.pidLinefollower.kd;

  configFile.close();
  DEBUG_PRINTLN("Configuration loaded.");
}