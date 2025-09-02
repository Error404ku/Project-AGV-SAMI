#include "config.h"
#include "performance_linefollower.h"
// extern bool modeMaju; // Removed - not used
// extern bool modeMundur; // Removed - not used

void setup() {
  setupAll();

  // Initialize performance optimization system
  initPerformanceOptimization();

  // Load all AGV states efficiently in one call
  loadAllAGVStatesFromPreferences();

  // Initialize ultrasonic sensor if not already done
  static bool ultrasonicSensorInitialized = false;
  if (!ultrasonicSensorInitialized) {
    setupUltrasonikWithParams(SLAVEID_ULTRASONIK_DEPAN);
    ultrasonicSensorInitialized = true;
  }

  // Wait for PID data from motor controller slave before proceeding
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for PID");
  lcd.setCursor(0, 1);
  lcd.print("Data from Slave");
  
  // Loop until PID data is received or timeout
  while (!checkSystemReadyStatus()) {
    // Handle incoming serial data while waiting
    handleMotorControllerSerial();
    delay(50); // Small delay to prevent watchdog issues
    // esp_task_wdt_reset(); // Reset watchdog
  }
  
  // System is now ready
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AGV System");
  lcd.setCursor(0, 1);
  lcd.print("READY TO RUN");
  delay(1000);
}

void loop() {
  esp_task_wdt_reset();
  
  // Static variables for double click STOP functionality - moved to loop scope
  static bool firstStopClick = false;
  static unsigned long firstStopTime = 0;
  static bool showingStopMessage = false;
  static unsigned long stopMessageStartTime = 0;
  static bool lastStopState = false;
  const unsigned long doubleClickInterval = 2000; // 2 seconds
  const unsigned long messageDisplayTime = 2000; // 2 seconds to show message
  
  // Only proceed with normal operations if system is ready
  if (!systemReadyToRun) {
    // Keep trying to get PID data if not received yet
    handleMotorControllerSerial();
    if (!checkSystemReadyStatus()) {
      delay(100);
      return; // Don't proceed with normal loop until ready
    }
  }
  
  // Handle incoming serial data from motor controller
  handleMotorControllerSerial();
  
  // Monitor system health (setiap 5 detik) - debug disabled
  static unsigned long lastSystemCheck = 0;
  if (millis() - lastSystemCheck > 5000) {
    size_t freeHeap = ESP.getFreeHeap();
    if (freeHeap < 15000) { // Less than 15KB free
      // Low memory warning disabled for production
      // Stop semua motor untuk menghemat resources
      rpmMotor(0, 0);  // Use RPM stop command
      hook(STOP_HOOK);
      if (freeHeap < 8000) {
        // Critical memory restart disabled for production
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
    // esp_task_wdt_reset();
    
    // Original sensor reading
    if (shouldReadUltrasonic()) {
      loopUltrasonik();
    }
    
    loopMagneticSensor();
    
    lamp_flip_flop();
    
    // Reset watchdog sebelum operasi motor
    // esp_task_wdt_reset();
    
    if (currentStateAgv != AGV_STATE_NULL){
      // --- Pembacaan sensor sesuai mode ---
      if (moveStateAgv == AGV_STATE_MOVE_FORWARD) {
        setMagnetSlaveId(SLAVEID_MAGNET_DEPAN);
        setUltrasonicSlaveId(SLAVEID_ULTRASONIK_DEPAN);
      } else if (moveStateAgv == AGV_STATE_MOVE_BACKWARD) {
        setMagnetSlaveId(SLAVEID_MAGNET_BELAKANG);
        setUltrasonicSlaveId(SLAVEID_ULTRASONIK_BELAKANG);
      }
      // Don't call agvMode() when showing stop message to prevent display override
      if (!showingStopMessage) {
        agvMode(currentStateAgv);
      }
    }
    else if (currentStateAgv == AGV_STATE_NULL){
      // esp_task_wdt_reset();
      if (hookPosition != DOWN_POS){
        hookPosition = hook(DOWN_HOOK);
      }else{
        // Don't call agvMode() when showing stop message to prevent display override
        if (!showingStopMessage) {
          agvMode(AGV_STATE_TERMINAL_PICKUP);
        }
      }
    }
    // Check for double click STOP button to exit AGV mode
    // Read current button state (without debounce for faster response)
    bool currentStopState = (digitalRead(stopPin) == HIGH);
    
    // Handle stop message display timing - show for full 2 seconds
    if (showingStopMessage) {
      if (millis() - stopMessageStartTime >= messageDisplayTime) {
        showingStopMessage = false;
        lcd.clear();
        resetDisplayRequested = true;
      }
      // Continue normal loop execution while showing message
    }
    
    // Detect button press (rising edge)
    if (currentStopState && !lastStopState) {
      unsigned long currentTime = millis();
      
      if (!firstStopClick) {
        // First click detected
        firstStopClick = true;
        firstStopTime = currentTime;
        
        // Show message on LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("STOP 1x detected");
        lcd.setCursor(0, 1);
        lcd.print("Click again to exit");
        
        // Start message display timer
        showingStopMessage = true;
        stopMessageStartTime = currentTime;
        
      } else {
        // Check if second click is within interval
        if (currentTime - firstStopTime <= doubleClickInterval) {
          // Valid double click - exit AGV mode
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Exiting AGV Mode");
          lcd.setCursor(0, 1);
          lcd.print("Please wait...");
          delay(1500); // Show exit message
          
          agvMode(AGV_STATE_STOP);
          isAgvMode = false;
          resetDisplayFlags();
          newRfidScanned = false;
          agvStopCalled = false;
          buttonStep = 0;
          
          // Reset all double click variables
          firstStopClick = false;
          firstStopTime = 0;
          showingStopMessage = false;
          lastStopState = false;
          
          return; // Exit AGV mode immediately
        } else {
          // Second click too late, treat as new first click
          firstStopClick = true;
          firstStopTime = currentTime;
          
          // Show message on LCD
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("STOP 1x detected");
          lcd.setCursor(0, 1);
          lcd.print("Click again to exit");
          
          // Start message display timer
          showingStopMessage = true;
          stopMessageStartTime = currentTime;
        }
      }
    }
    
    // Update button state for next iteration
    lastStopState = currentStopState;
    
    // Check if first click has timed out
    if (firstStopClick && (millis() - firstStopTime > doubleClickInterval) && !showingStopMessage) {
      firstStopClick = false;
      firstStopTime = 0;
      lcd.clear();
      resetDisplayRequested = true;
    }
    
    // Don't call displayPrint() here - let each mode handle its own display
    // displayPrint() was overriding mode-specific displays like modeDisplayWarehouse()
  } else {
    // esp_task_wdt_reset();
    agvMode(AGV_STATE_STOP);
    handleMenu();
    softStartTime = millis();
    softStartActive = true;
    pidSpeed = baseSpeed / 4;
  }
  
  // Reset watchdog timer to prevent reboot
  // esp_task_wdt_reset();
  
}
