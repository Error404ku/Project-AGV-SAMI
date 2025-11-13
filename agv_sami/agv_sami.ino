#include "config.h"
#include "performance_linefollower.h"
// extern bool modeMaju; // Removed - not used
// extern bool modeMundur; // Removed - not used

void setup() {
  setupPreferences();
  setuplamp();
  setupMusic();
  setupDisplay();
  setupMenu();  // Initialize menu system

  // Setup RS485 communication for both Serial1 and Serial2
  setupRS485(BAUDRATE);        // Serial1 untuk sensor magnet
  setupRS485_Serial2(BAUDRATE); // Serial2 untuk sensor ultrasonik
  
  setupSensorMagnet(SLAVEID_MAGNET_DEPAN);  
  setupSensorUltrasonic(SLAVEID_ULTRASONIK_DEPAN);
  
  setupHook();  // setupBuzzer();
  setupWifi();  // Setup WiFi configuration
  
  startWifiConnection();  // Auto-start WiFi connection
  setupWebServer();  // Setup Web Server - CRITICAL for HTTP access
  setupTombol();
  setupRfid();
  resetSensorTimers(); 
  setupMotor();  // Setup motor serial communication
  
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
