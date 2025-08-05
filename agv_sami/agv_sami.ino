#include "config.h"
#include "performance_linefollower.h"
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

  // Initialize ultrasonic sensor if not already done
  static bool ultrasonicSensorInitialized = false;
  if (!ultrasonicSensorInitialized) {
    setupUltrasonikWithParams(SLAVEID_ULTRASONIK_DEPAN);
    ultrasonicSensorInitialized = true;
  }

}

void loop() {
  server.handleClient();
  loopWifi();  // Handle WiFi connection monitoring
  
  // Rate-limited sensor readings to reduce delays
  if (shouldReadRfid()) {
    loopRfid();  // Handle RFID scanning
  }

  if (isAgvMode) {
    if (shouldReadUltrasonic()) {
      loopUltrasonik();
    }
    if (shouldReadMagnet()) {
      loopMagneticSensor();
    }
    lamp_flip_flop();
    if (currentStateAgv != AGV_STATE_NULL){
      // --- Pembacaan sensor sesuai mode ---
      if (moveStateAgv == AGV_STATE_MOVE_FORWARD) {
        setMagnetSlaveId(SLAVEID_MAGNET_DEPAN);
        setUltrasonicSlaveId(SLAVEID_ULTRASONIK_DEPAN);
      } else if (moveStateAgv == AGV_STATE_MOVE_BACKWARD) {
        setMagnetSlaveId(SLAVEID_MAGNET_BELAKANG);
        setUltrasonicSlaveId(SLAVEID_ULTRASONIK_BELAKANG);
      }
      agvMode(currentStateAgv);
    }
    else if (currentStateAgv == AGV_STATE_NULL){
      if (hookPosition != DOWN_POS){
        hookPosition = hook(DOWN_HOOK);
      }else{
        lcd.clear(); // Membersihkan tampilan sebelum menampilkan mode AGV
        displayPrint();
        agvMode(AGV_STATE_TERMINAL_PICKUP);
      }
    }
    // Check for B button to exit AGV mode
    if (STOP()) {
      agvMode(AGV_STATE_STOP);
      isAgvMode = false;
      newRfidScanned = false; // Reset flag RFID saat keluar dari AGV mode
      agvStopCalled = false; // Reset agvStopCalled when exiting AGV mode
      buttonStep = 0;  // Reset button step
    }
  } else {
    // Menu Mode
    // inTerminal();
    agvMode(AGV_STATE_STOP);
    handleMenu();
  }

}
