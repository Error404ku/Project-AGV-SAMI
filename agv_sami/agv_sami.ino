#include "config.h"
extern bool modeMaju;
extern bool modeMundur;

void setup() {
  Serial.begin(115200);
  setupAll();

  // Initialize performance optimization system
  initPerformanceOptimization();

  // lcd.clear();
  // changeStateMode(STATE_MODE_MAJU);
  AgvState currentStateAGV = loadCurrentStateAGVFromPreferences();
  AgvState lastStateAGV = loadLastStateAGVFromPreferences();
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
    if (!currentStateAGV == AGV_STATE_NULL){
      // --- Pembacaan sensor sesuai mode ---
      if (lastStateAgv == STATE_MODE_MAJU) {
        setMagnetSlaveId(SLAVEID_MAGNET_DEPAN);
        setUltrasonicSlaveId(SLAVEID_ULTRASONIK_DEPAN);
      } else if (lastStateAgv == STATE_MODE_MUNDUR) {
        setMagnetSlaveId(SLAVEID_MAGNET_BELAKANG);
        setUltrasonicSlaveId(SLAVEID_ULTRASONIK_BELAKANG);
      }
      agvMode(currentStateAGV);
    }

    agvMode(AGV_STATE_TERMINAL_PICKUP);
    displaySensorData();

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
