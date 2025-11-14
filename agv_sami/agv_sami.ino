#include "config.h"
#include "performance_linefollower.h"
// extern bool modeMaju; // Removed - not used
// extern bool modeMundur; // Removed - not used

void setup() {
  Serial.begin(115200);
  setupPreferences();
  setuplamp();
  setupMusic();
  setupDisplay();
  setupMenu();
  Serial.println("Menu System Ready");  // Initialize menu system

  // Setup RS485 communication for both Serial1 and Serial2
  setupRS485(BAUDRATE);        // Serial1 untuk sensor magnet
  Serial.println("RS485 Serial1 Ready");
  setupRS485_Serial2(BAUDRATE); // Serial2 untuk sensor ultrasonik
  Serial.println("RS485 Serial2 Ready");
  
  setupSensorMagnet(SLAVEID_MAGNET_DEPAN);
  Serial.println("Magnet Sensor Ready");  
  setupSensorUltrasonic(SLAVEID_ULTRASONIK_DEPAN);
  Serial.println("Ultrasonic Sensor Ready");
  
  setupHook();  // setupBuzzer();
  Serial.println("Hook Ready");
  setupWifi();  // Setup WiFi configuration
  Serial.println("WiFi Ready");
  
  startWifiConnection();  // Auto-start WiFi connection
  Serial.println("WiFi Connection Started");
  setupWebServer();  // Setup Web Server - CRITICAL for HTTP access
  Serial.println("Web Server Ready");
  setupTombol();
  Serial.println("Tombol Ready");
  setupRfid();
  Serial.println("RFID Ready");
  resetSensorTimers(); 
  Serial.println("Sensor Timers Reset");
  setupMotor();  // Setup motor serial communication
  Serial.println("Motor Ready");
  
  // Show message that AGV System is now ready
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AGV System");
  lcd.setCursor(0, 1);
  lcd.print("READY TO RUN");
  delay(1000);
}

void loop() {
  
  // Static variables for double click STOP functionality - moved to loop scope
  static bool firstStopClick = false;
  static unsigned long firstStopTime = 0;
  static bool showingStopMessage = false;
  static unsigned long stopMessageStartTime = 0;
  static bool lastStopState = false;
  static bool exitRequested = false; // Flag to track exit request
  const unsigned long doubleClickInterval = 2000; // 2 seconds
  const unsigned long messageDisplayTime = 2000; // 2 seconds to show message
  const unsigned long exitMessageTime = 1500; // 1.5 seconds for exit message
  
  // ===== SYSTEM READY CHECK - ONLY AT STARTUP =====
  // Only block loop at startup, NOT during AGV operation
  static bool initialStartupComplete = false;
  if (!initialStartupComplete && !systemReadyToRun) {
    // Keep trying to get PID data if not received yet
    handleMotorControllerSerial();
    if (!checkSystemReadyStatus()) {
      delay(100);
      return; // Don't proceed with normal loop until initial startup ready
    }
    initialStartupComplete = true; // Mark that initial startup is complete
  }
  
  // ===== OPTIMIZED LOOP ORDER - PRIORITY-BASED =====
  // PRIORITY 1: Critical Path - Line Following & Motor Control
  loopMagneticSensor();           // Magnet sensor (CRITICAL for line following)
  handleMotorControllerSerial();  // Motor control communication
  
  // PRIORITY 2: Navigation & Safety
  loopRfid();        // RFID position tracking (CRITICAL - no rate limit)
  loopUltrasonik();  // Obstacle detection (already has internal 100ms rate limit)
  
  // PRIORITY 3: Network Tasks - Rate Limited to 20ms (50Hz)
  static unsigned long lastNetwork = 0;
  if (millis() - lastNetwork > 20) {
    server.handleClient();
    loopWifi();
    lastNetwork = millis();
  }

  if (isAgvMode) {
    
    // Reset watchdog sebelum operasi sensor
    // esp_task_wdt_reset();
    
    // Sensor readings moved to main loop for better priority management
    // loopUltrasonik() and loopMagneticSensor() now called at top of loop
    
    lamp_flip_flop();
    
    // Reset watchdog sebelum operasi motor
    // esp_task_wdt_reset();
    
    if (currentStateAgv != AGV_STATE_NULL){
      // --- Pembacaan sensor sesuai mode ---
      // Selalu gunakan sensor depan (hanya maju)
      setMagnetSlaveId(SLAVEID_MAGNET_DEPAN);
      setUltrasonicSlaveId(SLAVEID_ULTRASONIK_DEPAN);
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
      unsigned long elapsed = millis() - stopMessageStartTime;
      unsigned long displayDuration = exitRequested ? exitMessageTime : messageDisplayTime;
      
      if (elapsed >= displayDuration) {
        showingStopMessage = false;
        lcd.clear();
        
        // If exit was requested, perform exit now
        if (exitRequested) {
          agvMode(AGV_STATE_STOP);
          isAgvMode = false;
          resetDisplayFlags();
          newRfidScanned = false;
          agvStopCalled = false;
          buttonStep = 0;
          
          // Reset all double click variables
          firstStopClick = false;
          firstStopTime = 0;
          lastStopState = false;
          exitRequested = false;
          
          return; // Exit AGV mode immediately
        }
        
        resetDisplayRequested = true;
      }
      // Continue normal loop execution while showing message
      // If exit requested, ignore STOP button completely
      if (exitRequested) {
        lastStopState = currentStopState; // Update state to prevent false triggers
        return; // Skip all button processing
      }
    }
    
    // Detect button press (rising edge) - only if not exiting
    if (currentStopState && !lastStopState && !exitRequested) {
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
          // Valid double click - show exit message first
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Exiting AGV Mode");
          lcd.setCursor(0, 1);
          lcd.print("Please wait...");
          
          // Set flag to show message for 1500ms before exiting
          showingStopMessage = true;
          firstStopClick = false; // Prevent re-triggering
          stopMessageStartTime = currentTime;
          exitRequested = true; // Mark for exit after message display
          
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
    
}
