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
  loopRfid(); // Handle RFID scanning - now controlled internally by conditions
  // loopUltrasonik(); // Akan dipanggil manual sesuai mode

  if (isAgvMode) {
    // AGV Mode - Run normal AGV operation
    // pembacaanRpm();
    // bacaSensorGaris(); // Akan dipanggil manual sesuai mode
    
    // --- Pembacaan sensor sesuai mode ---
    if (modeMaju) {
      bacaSensor(SLAVEID_MAGNET_DEPAN);
      switchUltrasonicSensor(true);
      loopUltrasonik();
    } else if (!modeMaju) {
      bacaSensor(SLAVEID_MAGNET_BELAKANG);
      switchUltrasonicSensor(false);
      loopUltrasonik();
    }
    
    displayPrint();
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
