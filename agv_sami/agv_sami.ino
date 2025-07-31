#include "config.h"
extern bool modeMaju;
extern bool modeMundur;

void setup() {
  Serial.begin(115200);
  setupAll();

  // Initialize performance optimization system
  initPerformanceOptimization();

  // Load all AGV states efficiently in one call
  loadAllAGVStatesFromPreferences();
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
  loopWifi();  // Handle WiFi connection monitoring
  loopRfid();  // Handle RFID scanning - now controlled internally by conditions
  // loopUltrasonik(); // Akan dipanggil manual sesuai mode

  if (isAgvMode) {
    // AGV Mode - Run normal AGV operation    
    bacaSensor();
    loopUltrasonik();
    lamp_flip_flop();
    if (!currentStateAgv == AGV_STATE_NULL){
      // --- Pembacaan sensor sesuai mode ---
      if (moveStateAgv == AGV_STATE_MOVE_FORWARD) {
        setMagnetSlaveId(SLAVEID_MAGNET_DEPAN);
        setUltrasonicSlaveId(SLAVEID_ULTRASONIK_DEPAN);
      } else if (moveStateAgv == AGV_STATE_MOVE_FORWARD) {
        setMagnetSlaveId(SLAVEID_MAGNET_BELAKANG);
        setUltrasonicSlaveId(SLAVEID_ULTRASONIK_BELAKANG);
      }
      agvMode(currentStateAgv);
    }
    else if (currentStateAgv == AGV_STATE_NULL){
      displayPrint();
      hook("turun");
      agvMode(AGV_STATE_TERMINAL_PICKUP);
    }
    // displaySensorData();

    // Check for B button to exit AGV mode
    if (STOP()) {
      // agvMode(AGV_STATE_STOP);
      isAgvMode = false;
      buttonStep = 0;  // Reset button step
    }
  } else {
    // Menu Mode
    // inTerminal();
    // agvMode(AGV_STATE_STOP);
    handleMenu();
  }

  // End performance monitoring
  endPerformanceMonitoring();
  // LCD doesn't need display() call - content shows immediately
}
