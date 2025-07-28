// Debug RS485 Communication System
// File ini berisi fungsi-fungsi debug untuk mengatasi masalah RS485

// Flag untuk mengaktifkan/menonaktifkan debug
bool enableRS485Debug = true;
bool enableDetailedDebug = false;
bool enablePacketDebug = false;
bool enableTimingDebug = false;

// Statistik komunikasi
struct CommStats {
  unsigned long totalAttempts;
  unsigned long successfulComm;
  unsigned long failedComm;
  unsigned long crcErrors;
  unsigned long timeouts;
  unsigned long lastResetTime;
};

CommStats magnetFrontStats = {0};
CommStats magnetBackStats = {0};
CommStats ultrasonicFrontStats = {0};
CommStats ultrasonicBackStats = {0};

// Timing variables untuk debug
unsigned long debugLastPrint = 0;
const unsigned long debugPrintInterval = 2000; // Print debug setiap 2 detik

// Last successful communication times for each device
unsigned long lastSuccessfulComm[4] = {0}; // [0]=MagnetFront, [1]=UltrasonicFront, [2]=UltrasonicBack, [3]=MagnetBack

// Forward declarations
void printDeviceStats(CommStats& stats);

void setupRS485Debug() {
  if (enableRS485Debug) {
    Serial.println("\n=== RS485 DEBUG SYSTEM INITIALIZED ===");
    Serial.println("Debug flags:");
    Serial.print("- Basic Debug: "); Serial.println(enableRS485Debug ? "ON" : "OFF");
    Serial.print("- Detailed Debug: "); Serial.println(enableDetailedDebug ? "ON" : "OFF");
    Serial.print("- Packet Debug: "); Serial.println(enablePacketDebug ? "ON" : "OFF");
    Serial.print("- Timing Debug: "); Serial.println(enableTimingDebug ? "ON" : "OFF");
    Serial.println("==========================================\n");
    
    resetCommStats();
  }
}

void resetCommStats() {
  magnetFrontStats = {0};
  magnetBackStats = {0};
  ultrasonicFrontStats = {0};
  ultrasonicBackStats = {0};
  
  magnetFrontStats.lastResetTime = millis();
  magnetBackStats.lastResetTime = millis();
  ultrasonicFrontStats.lastResetTime = millis();
  ultrasonicBackStats.lastResetTime = millis();
  
  if (enableRS485Debug) {
    Serial.println("Communication statistics reset.");
  }
}

void debugRS485Loop() {
  if (!enableRS485Debug) return;
  
  unsigned long currentMillis = millis();
  
  // Print debug info setiap interval tertentu
  if (currentMillis - debugLastPrint >= debugPrintInterval) {
    debugLastPrint = currentMillis;
    printCommStatistics();
    printDeviceStatus();
    printSensorData();
  }
}

void printCommStatistics() {
  if (!enableRS485Debug) return;
  
  Serial.println("\n=== COMMUNICATION STATISTICS ===");
  
  // Magnet Front Stats
  Serial.println("Magnet Front:");
  printDeviceStats(magnetFrontStats);
  
  // Magnet Back Stats
  Serial.println("Magnet Back:");
  printDeviceStats(magnetBackStats);
  
  // Ultrasonic Front Stats
  Serial.println("Ultrasonic Front:");
  printDeviceStats(ultrasonicFrontStats);
  
  // Ultrasonic Back Stats
  Serial.println("Ultrasonic Back:");
  printDeviceStats(ultrasonicBackStats);
  
  Serial.println("================================\n");
}

void printDeviceStats(CommStats& stats) {
  if (stats.totalAttempts > 0) {
    float successRate = (float)stats.successfulComm / stats.totalAttempts * 100.0;
    Serial.print("  Success Rate: ");
    Serial.print(successRate, 1);
    Serial.print("% (");
    Serial.print(stats.successfulComm);
    Serial.print("/");
    Serial.print(stats.totalAttempts);
    Serial.println(")");
    
    if (stats.failedComm > 0) {
      Serial.print("  Failed: "); Serial.println(stats.failedComm);
    }
    if (stats.crcErrors > 0) {
      Serial.print("  CRC Errors: "); Serial.println(stats.crcErrors);
    }
    if (stats.timeouts > 0) {
      Serial.print("  Timeouts: "); Serial.println(stats.timeouts);
    }
  } else {
    Serial.println("  No communication attempts");
  }
}

void printDeviceStatus() {
  if (!enableRS485Debug) return;
  
  Serial.println("=== DEVICE STATUS ===");
  Serial.println(getDeviceStatusString());
  
  // Print last successful communication times
  unsigned long currentMillis = millis();
  Serial.println("Last successful communication:");
  
  for (int i = 0; i < 4; i++) {
    const char* deviceNames[] = {"Magnet Front", "Ultrasonic Front", "Ultrasonic Back", "Magnet Back"};
    Serial.print("  ");
    Serial.print(deviceNames[i]);
    Serial.print(": ");
    
    if (lastSuccessfulComm[i] > 0) {
      unsigned long timeSince = currentMillis - lastSuccessfulComm[i];
      Serial.print(timeSince);
      Serial.println(" ms ago");
    } else {
      Serial.println("Never");
    }
  }
  Serial.println("=====================\n");
}

void printSensorData() {
  if (!enableDetailedDebug) return;
  
  Serial.println("=== SENSOR DATA ===");
  
  // Print magnet data
  Serial.print("Magnet Front: ");
  printMagnetArray(jumlahMagnetFront);
  
  Serial.print("Magnet Back:  ");
  printMagnetArray(jumlahMagnetBack);
  
  // Print ultrasonic data
  Serial.print("Ultrasonic Front: ");
  for (int i = 0; i < 5; i++) {
    Serial.print(ultrasonicDistancesFront[i]);
    if (i < 4) Serial.print(", ");
  }
  Serial.println(" cm");
  
  Serial.print("Ultrasonic Back:  ");
  for (int i = 0; i < 5; i++) {
    Serial.print(ultrasonicDistancesBack[i]);
    if (i < 4) Serial.print(", ");
  }
  Serial.println(" cm");
  
  Serial.print("Error Value: ");
  Serial.println(errorValue);
  
  Serial.println("===================\n");
}

void printMagnetArray(uint8_t* magnetArray) {
  for (int i = 0; i < 16; i++) {
    Serial.print(magnetArray[i]);
  }
  Serial.println();
}

void debugModbusError(uint8_t result, const char* deviceName) {
  if (!enableRS485Debug) return;
  
  Serial.print("[ERROR] ");
  Serial.print(deviceName);
  Serial.print(" - Modbus Error: 0x");
  Serial.print(result, HEX);
  Serial.print(" (");
  
  switch (result) {
    case 0x01:
      Serial.print("Illegal Function");
      break;
    case 0x02:
      Serial.print("Illegal Data Address");
      break;
    case 0x03:
      Serial.print("Illegal Data Value");
      break;
    case 0x04:
      Serial.print("Slave Device Failure");
      break;
    case 0xE0:
      Serial.print("Invalid Slave ID");
      break;
    case 0xE1:
      Serial.print("Invalid Function");
      break;
    case 0xE2:
      Serial.print("Response Timed Out");
      break;
    case 0xE3:
      Serial.print("Invalid CRC");
      break;
    default:
      Serial.print("Unknown Error");
      break;
  }
  Serial.println(")");
}

void debugPacketData(byte* packet, int length, const char* direction) {
  if (!enablePacketDebug) return;
  
  Serial.print("[PACKET] ");
  Serial.print(direction);
  Serial.print(": ");
  
  for (int i = 0; i < length; i++) {
    if (packet[i] < 0x10) Serial.print("0");
    Serial.print(packet[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void debugTiming(const char* operation, unsigned long startTime) {
  if (!enableTimingDebug) return;
  
  unsigned long duration = millis() - startTime;
  Serial.print("[TIMING] ");
  Serial.print(operation);
  Serial.print(": ");
  Serial.print(duration);
  Serial.println(" ms");
}

// Fungsi untuk update statistik komunikasi
void updateMagnetFrontStats(bool success, bool timeout) {
  magnetFrontStats.totalAttempts++;
  if (success) {
    magnetFrontStats.successfulComm++;
    lastSuccessfulComm[0] = millis(); // Update last successful time
  } else {
    magnetFrontStats.failedComm++;
    if (timeout) magnetFrontStats.timeouts++;
  }
}

void updateMagnetBackStats(bool success, bool timeout) {
  magnetBackStats.totalAttempts++;
  if (success) {
    magnetBackStats.successfulComm++;
    lastSuccessfulComm[3] = millis(); // Update last successful time
  } else {
    magnetBackStats.failedComm++;
    if (timeout) magnetBackStats.timeouts++;
  }
}

void updateUltrasonicFrontStats(bool success, bool crcError) {
  ultrasonicFrontStats.totalAttempts++;
  if (success) {
    ultrasonicFrontStats.successfulComm++;
    lastSuccessfulComm[1] = millis(); // Update last successful time
  } else {
    ultrasonicFrontStats.failedComm++;
    if (crcError) ultrasonicFrontStats.crcErrors++;
  }
}

void updateUltrasonicBackStats(bool success, bool crcError) {
  ultrasonicBackStats.totalAttempts++;
  if (success) {
    ultrasonicBackStats.successfulComm++;
    lastSuccessfulComm[2] = millis(); // Update last successful time
  } else {
    ultrasonicBackStats.failedComm++;
    if (crcError) ultrasonicBackStats.crcErrors++;
  }
}

// Fungsi untuk test koneksi individual
void testIndividualDevices() {
  Serial.println("\n=== TESTING INDIVIDUAL DEVICES ===");
  
  // Test Magnet Front
  Serial.println("Testing Magnet Front...");
  currentDeviceAddress = ADDR_MAGNET_FRONT;
  communicateWithMagnetFront();
  delay(100);
  
  // Test Magnet Back
  Serial.println("Testing Magnet Back...");
  currentDeviceAddress = ADDR_MAGNET_BACK;
  communicateWithMagnetBack();
  delay(100);
  
  // Test Ultrasonic (passive listening)
  Serial.println("Listening for Ultrasonic data...");
  unsigned long testStart = millis();
  while (millis() - testStart < 1000) {
    processUltrasonicData();
    delay(10);
  }
  
  Serial.println("Individual device test complete.\n");
}

// Fungsi untuk diagnosa masalah umum
void diagnoseCommonIssues() {
  Serial.println("\n=== DIAGNOSING COMMON ISSUES ===");
  
  // Check baudrate
  Serial.print("Configured Baudrate: ");
  Serial.println(BAUDRATE_RS485);
  
  // Check device addresses
  Serial.println("Device Addresses:");
  Serial.print("  Magnet Front: "); Serial.println(ADDR_MAGNET_FRONT);
  Serial.print("  Ultrasonic Front: "); Serial.println(ADDR_ULTRASONIC_FRONT);
  Serial.print("  Ultrasonic Back: "); Serial.println(ADDR_ULTRASONIC_BACK);
  Serial.print("  Magnet Back: "); Serial.println(ADDR_MAGNET_BACK);
  
  // Check pin configuration
  Serial.println("Pin Configuration:");
  Serial.print("  TX_RS485: "); Serial.println(TX_RS485);
  Serial.print("  RX_RS485: "); Serial.println(RX_RS485);
  Serial.print("  MAX485_DE: "); Serial.println(MAX485_DE);
  Serial.print("  MAX485_RE: "); Serial.println(MAX485_RE);
  
  // Check timing configuration
  Serial.println("Timing Configuration:");
  Serial.println("  Device Switch Intervals:");
  const char* deviceNames[] = {"Magnet Front", "Ultrasonic Front", "Ultrasonic Back", "Magnet Back"};
  for (int i = 0; i < 4; i++) {
    Serial.print("    "); Serial.print(deviceNames[i]); Serial.print(": "); Serial.print(deviceSwitchInterval[i]); Serial.println(" ms");
  }
  Serial.println("  Device Timeouts:");
  for (int i = 0; i < 4; i++) {
    Serial.print("    "); Serial.print(deviceNames[i]); Serial.print(": "); Serial.print(deviceTimeout[i]); Serial.println(" ms");
  }
  
  Serial.println("Diagnosis complete.\n");
}

// Fungsi untuk toggle debug modes
void toggleDebugMode(char mode) {
  switch (mode) {
    case 'b': // basic
      enableRS485Debug = !enableRS485Debug;
      Serial.print("Basic Debug: "); Serial.println(enableRS485Debug ? "ON" : "OFF");
      break;
    case 'd': // detailed
      enableDetailedDebug = !enableDetailedDebug;
      Serial.print("Detailed Debug: "); Serial.println(enableDetailedDebug ? "ON" : "OFF");
      break;
    case 'p': // packet
      enablePacketDebug = !enablePacketDebug;
      Serial.print("Packet Debug: "); Serial.println(enablePacketDebug ? "ON" : "OFF");
      break;
    case 't': // timing
      enableTimingDebug = !enableTimingDebug;
      Serial.print("Timing Debug: "); Serial.println(enableTimingDebug ? "ON" : "OFF");
      break;
    case 'r': // reset stats
      resetCommStats();
      break;
    case 'i': // individual test
      testIndividualDevices();
      break;
    case 'g': // diagnose
      diagnoseCommonIssues();
      break;
    default:
      Serial.println("Debug Commands:");
      Serial.println("  b - Toggle Basic Debug");
      Serial.println("  d - Toggle Detailed Debug");
      Serial.println("  p - Toggle Packet Debug");
      Serial.println("  t - Toggle Timing Debug");
      Serial.println("  r - Reset Statistics");
      Serial.println("  i - Test Individual Devices");
      Serial.println("  g - Diagnose Common Issues");
      break;
  }
}