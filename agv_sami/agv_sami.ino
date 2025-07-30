#include "config.h"
extern bool modeMaju;
extern bool modeMundur;

void setup() {
  Serial.begin(115200);
  setupAll();

  // Initialize performance optimization system
  initPerformanceOptimization();

  // lcd.clear();
  changeStateMode("maju");
  Serial.println("SETUP SELESAI - Performance Optimization Active");
}

void loop() {
  // Start performance monitoring
  startPerformanceMonitoring();
  // Update performance optimization timers
  updatePerformanceOptimization();
  // Skip main operations if system is in error state
  if (systemInErrorState) {
    endPerformanceMonitoring();
    return;
  }
  server.handleClient();
  loopRfid();  // Handle RFID scanning - now controlled internally by conditions
  // loopUltrasonik(); // Akan dipanggil manual sesuai mode

  if (isAgvMode) {
    // AGV Mode - Run normal AGV operation    
    displayPrint();
    bacaSensor();
    loopUltrasonik();
    lamp_flip_flop();
    // --- Pembacaan sensor sesuai mode ---
    if (modeMaju) {
      setMagnetSlaveId(SLAVEID_MAGNET_DEPAN);
      setUltrasonicSlaveId(SLAVEID_ULTRASONIK_DEPAN);
    } else if (modeMundur) {
      setMagnetSlaveId(SLAVEID_MAGNET_BELAKANG);
      setUltrasonicSlaveId(SLAVEID_ULTRASONIK_BELAKANG);
    }

    displaySensorData();
    logicAgv();

    // Check for B button to exit AGV mode
    if (STOP()) {
      isAgvMode = false;
      modeBerhenti = true;
      buttonStep = 0;  // Reset button step
    }
  } else {
    // Menu Mode
    inTerminal();
    handleMenu();
  }

  // End performance monitoring
  endPerformanceMonitoring();
  // LCD doesn't need display() call - content shows immediately
}
