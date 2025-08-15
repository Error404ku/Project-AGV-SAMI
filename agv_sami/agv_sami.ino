#include "config.h"
#include "performance_linefollower.h"
// extern bool modeMaju; // Removed - not used
// extern bool modeMundur; // Removed - not used

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
  esp_task_wdt_reset();
  
  // Monitor system health (setiap 5 detik)
  static unsigned long lastSystemCheck = 0;
  if (millis() - lastSystemCheck > 5000) {
    size_t freeHeap = ESP.getFreeHeap();
    if (freeHeap < 15000) { // Less than 15KB free
      Serial.printf("WARNING: Low memory! Free heap: %d bytes\n", freeHeap);
      // Stop semua motor untuk menghemat resources
      pwmMotor(0, 0);
      hook(STOP_HOOK);
      if (freeHeap < 8000) {
        Serial.println("CRITICAL: Memory too low, restarting...");
        ESP.restart();
      }
    }
    lastSystemCheck = millis();
  }
  
  server.handleClient();
  loopWifi();  // Handle WiFi connection monitoring
  
  // Rate-limited sensor readings to reduce delays
  if (shouldReadRfid()) {
    loopRfid();  // Handle RFID scanning
  }

  if (isAgvMode) {
    
    // Reset watchdog sebelum operasi sensor
    esp_task_wdt_reset();
    
    // Original sensor reading
    if (shouldReadUltrasonic()) {
      loopUltrasonik();
    }
    if (shouldReadMagnet()) {
      loopMagneticSensor();
    }
    
    lamp_flip_flop();
    
    // Reset watchdog sebelum operasi motor
    esp_task_wdt_reset();
    
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
      esp_task_wdt_reset();
      if (hookPosition != DOWN_POS){
        hookPosition = hook(DOWN_HOOK);
      }else{
        agvMode(AGV_STATE_TERMINAL_PICKUP);
      }
    }
    // Check for double click STOP button to exit AGV mode
    static bool firstStopClick = false;
    static unsigned long firstStopTime = 0;
    const unsigned long doubleClickInterval = 2000; // 2 seconds
    
    if (STOP()) {
      unsigned long currentTime = millis();
      
      if (!firstStopClick) {
        // First click detected
        firstStopClick = true;
        firstStopTime = currentTime;
        Serial.println("[AGV_EXIT] First STOP click detected. Click again within 2 seconds to exit AGV mode.");
        
        // Show message on LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("STOP 1x detected");
        lcd.setCursor(0, 1);
        lcd.print("Click again to exit");
      } else {
        // Check if second click is within interval
        if (currentTime - firstStopTime <= doubleClickInterval) {
          // Valid double click - exit AGV mode
          Serial.println("[AGV_EXIT] Double click confirmed. Exiting AGV mode.");
          agvMode(AGV_STATE_STOP);
          isAgvMode = false;
          resetDisplayFlags(); // Reset semua flag display
          newRfidScanned = false; // Reset flag RFID saat keluar dari AGV mode
          agvStopCalled = false; // Reset agvStopCalled when exiting AGV mode
          buttonStep = 0;  // Reset button step
          
          // Reset double click variables
          firstStopClick = false;
          firstStopTime = 0;
        } else {
          // Second click too late, treat as new first click
          firstStopClick = true;
          firstStopTime = currentTime;
          Serial.println("[AGV_EXIT] Second click too late. Starting new double click sequence.");
          
          // Show message on LCD
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("STOP 1x detected");
          lcd.setCursor(0, 1);
          lcd.print("Click again to exit");
        }
      }
    } else {
      // Check if first click has timed out
      if (firstStopClick && (millis() - firstStopTime > doubleClickInterval)) {
        firstStopClick = false;
        firstStopTime = 0;
        Serial.println("[AGV_EXIT] Double click timeout. Reset to normal AGV display.");
        
        // Reset display to normal AGV mode
        lcd.clear();
        displayPrint();
      }
    }
  } else {
    esp_task_wdt_reset();
    agvMode(AGV_STATE_STOP);
    handleMenu();
  }
  
  // Reset watchdog timer to prevent reboot
  esp_task_wdt_reset();
  
}
