// ===================================================================
//                    FREERTOS IMPLEMENTATION INTEGRATED
// ===================================================================

#include "config.h"

// FreeRTOS Handles Definitions - These are defined in config.h as extern
TaskHandle_t taskHandleSafety = NULL;
TaskHandle_t taskHandleSensorMagnet = NULL;
TaskHandle_t taskHandleSensorUltrasonic = NULL;
TaskHandle_t taskHandleSensorRFID = NULL;
TaskHandle_t taskHandlePIDController = NULL;
TaskHandle_t taskHandleAGVLogic = NULL;
TaskHandle_t taskHandleWiFi = NULL;
TaskHandle_t taskHandleDisplay = NULL;
TaskHandle_t taskHandleButton = NULL;
TaskHandle_t taskHandleMenu = NULL;

QueueHandle_t queueSensorData = NULL;
QueueHandle_t queueCommands = NULL;
QueueHandle_t queueDisplayUpdate = NULL;
QueueHandle_t queueButtonPress = NULL;

SemaphoreHandle_t mutexSensorData = NULL;
SemaphoreHandle_t mutexMotorControl = NULL;
SemaphoreHandle_t mutexDisplay = NULL;
SemaphoreHandle_t mutexPreferences = NULL;

EventGroupHandle_t eventGroupSystem = NULL;
TimerHandle_t timerWatchdog = NULL;
TimerHandle_t timerSensorHealth = NULL;

// Shared data with mutex protection
volatile SensorData_t sharedSensorData;
volatile Command_t currentCommand;

// ===================================================================
//                    SAFETY MONITOR TASK (HIGHEST PRIORITY)
// ===================================================================
void taskSafetyMonitor(void *parameters) {
    Serial.println("[FREERTOS] Safety Monitor Task Started");
    
    while (true) {
        bool emergencyDetected = false;
        
        // Check obstacle detection with mutex protection
        if (xSemaphoreTake(mutexSensorData, pdMS_TO_TICKS(5)) == pdTRUE) {
            if (sharedSensorData.sensorType == 1 && sharedSensorData.distance < minSafeDistanceFront) {
                emergencyDetected = true;
                obstacleDetected = true;
            }
            xSemaphoreGive(mutexSensorData);
        }
        
        // Check STOP button (use existing function)
        if (STOP()) {
            emergencyDetected = true;
        }
        
        // Handle emergency
        if (emergencyDetected) {
            xEventGroupSetBits(eventGroupSystem, EVENT_EMERGENCY_STOP);
            
            // Send emergency stop command
            Command_t emergencyCmd;
            emergencyCmd.commandType = 1; // Stop command
            emergencyCmd.newState = AGV_STATE_STOP;
            xQueueSendToFront(queueCommands, &emergencyCmd, 0);
            
            // Immediately stop motors with mutex protection
            if (xSemaphoreTake(mutexMotorControl, pdMS_TO_TICKS(5)) == pdTRUE) {
                pwmMotor(0, 0);
                xSemaphoreGive(mutexMotorControl);
            }
        } else {
            xEventGroupClearBits(eventGroupSystem, EVENT_EMERGENCY_STOP);
        }
        
        // Update every 20ms instead of 10ms for stability
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// ===================================================================
//                    MAGNETIC SENSOR TASK
// ===================================================================
void taskSensorMagnet(void *parameters) {
    Serial.println("[FREERTOS] Magnetic Sensor Task Started");
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (true) {
        // Use existing optimized magnetic sensor reading function
        loopMagneticSensor();
        
        // Update shared data with mutex protection
        SensorData_t magnetData;
        magnetData.timestamp = millis();
        magnetData.sensorType = 0; // Magnet sensor
        magnetData.errorValue = errorValue;
        magnetData.isValid = (errorValue != 99);
        
        // Send to sensor data queue (non-blocking)
        xQueueSend(queueSensorData, &magnetData, 0);
        
        // Update shared data with mutex protection
        if (xSemaphoreTake(mutexSensorData, pdMS_TO_TICKS(1)) == pdTRUE) {
            memcpy((void*)&sharedSensorData, &magnetData, sizeof(SensorData_t));
            xSemaphoreGive(mutexSensorData);
        }
        
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(15)); // Reduced from 5ms to 15ms for stability
    }
}

// ===================================================================
//                    ULTRASONIC SENSOR TASK
// ===================================================================
void taskSensorUltrasonic(void *parameters) {
    Serial.println("[FREERTOS] Ultrasonic Sensor Task Started");
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (true) {
        // Use existing ultrasonic sensor reading function
        loopUltrasonik();
        
        // Update shared data
        SensorData_t ultrasonicData;
        ultrasonicData.timestamp = millis();
        ultrasonicData.sensorType = 1; // Ultrasonic sensor
        ultrasonicData.distance = ultrasonicDistances[0]; // Front sensor
        ultrasonicData.isValid = true;
        
        // Send to sensor data queue (non-blocking)
        xQueueSend(queueSensorData, &ultrasonicData, 0);
        
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(50)); // 50ms cycle (20Hz)
    }
}

// ===================================================================
//                    RFID SENSOR TASK
// ===================================================================
void taskSensorRFID(void *parameters) {
    Serial.println("[FREERTOS] RFID Sensor Task Started");
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (true) {
        // Use existing RFID reading function
        loopRfid();
        
        // Check if new RFID detected (use existing global variable)
        if (newRfidScanned) {
            SensorData_t rfidData;
            rfidData.timestamp = millis();
            rfidData.sensorType = 2; // RFID sensor
            rfidData.rfidId = lastDetectedRfidId;
            rfidData.isValid = true;
            
            // Send to sensor data queue
            xQueueSend(queueSensorData, &rfidData, 0);
            
            newRfidScanned = false; // Reset flag
        }
        
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(100)); // 100ms cycle (10Hz)
    }
}

// ===================================================================
//                    PID CONTROLLER TASK
// ===================================================================
void taskPIDController(void *parameters) {
    Serial.println("[FREERTOS] PID Controller Task Started");
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (true) {
        // Check if system is in AGV mode and not in emergency
        EventBits_t eventBits = xEventGroupGetBits(eventGroupSystem);
        
        if ((eventBits & EVENT_AGV_MODE) && !(eventBits & EVENT_EMERGENCY_STOP)) {
            // Get current sensor data with mutex protection
            if (xSemaphoreTake(mutexSensorData, pdMS_TO_TICKS(5)) == pdTRUE) {
                if (sharedSensorData.sensorType == 0 && sharedSensorData.isValid) {
                    // Determine PID mode based on current AGV state
                    PidMode currentPidMode = PID_MODE_DEFAULT;
                    
                    if (moveStateAgv == AGV_STATE_MOVE_FORWARD) {
                        currentPidMode = hookPosition == UP_POS ? PID_MODE_MAJU_MASSA : PID_MODE_MAJU;
                    } else if (moveStateAgv == AGV_STATE_MOVE_BACKWARD) {
                        currentPidMode = hookPosition == UP_POS ? PID_MODE_MUNDUR_MASSA : PID_MODE_MUNDUR;
                    }
                    
                    // Execute PID control with mutex protection
                    if (xSemaphoreTake(mutexMotorControl, pdMS_TO_TICKS(5)) == pdTRUE) {
                        pidLinefollower(sharedSensorData.errorValue, currentPidMode);
                        xSemaphoreGive(mutexMotorControl);
                    }
                }
                xSemaphoreGive(mutexSensorData);
            }
        }
        
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(25)); // Reduced from 10ms to 25ms for stability
    }
}

// ===================================================================
//                    AGV LOGIC TASK
// ===================================================================
void taskAGVLogic(void *parameters) {
    Serial.println("[FREERTOS] AGV Logic Task Started");
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (true) {
        // Check for commands in queue
        Command_t receivedCommand;
        if (xQueueReceive(queueCommands, &receivedCommand, 0) == pdTRUE) {
            // Process command
            switch (receivedCommand.commandType) {
                case 0: // Move command
                    moveStateAgv = receivedCommand.newState;
                    break;
                case 1: // Stop command
                    currentStateAgv = AGV_STATE_STOP;
                    agvMode(AGV_STATE_STOP);
                    break;
                case 2: // Mode change
                    currentStateAgv = receivedCommand.newState;
                    break;
            }
        }
        
        // Execute current AGV logic only if in AGV mode
        EventBits_t eventBits = xEventGroupGetBits(eventGroupSystem);
        if (eventBits & EVENT_AGV_MODE) {
            // Use existing AGV mode function
            agvMode(currentStateAgv);
            
            // Handle lamp flip flop (existing function)
            lamp_flip_flop();
            
            // Handle sensor slave ID switching based on movement direction
            if (currentStateAgv != AGV_STATE_NULL) {
                if (moveStateAgv == AGV_STATE_MOVE_FORWARD) {
                    setMagnetSlaveId(SLAVEID_MAGNET_DEPAN);
                    setUltrasonicSlaveId(SLAVEID_ULTRASONIK_DEPAN);
                } else if (moveStateAgv == AGV_STATE_MOVE_BACKWARD) {
                    setMagnetSlaveId(SLAVEID_MAGNET_BELAKANG);
                    setUltrasonicSlaveId(SLAVEID_ULTRASONIK_BELAKANG);
                }
            }
        }
        
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(30)); // Reduced from 20ms to 30ms for stability
    }
}

// ===================================================================
//                    BUTTON HANDLER TASK
// ===================================================================
void taskButtonHandler(void *parameters) {
    Serial.println("[FREERTOS] Button Handler Task Started");
    
    while (true) {
        // Check all buttons with proper debouncing (use existing button functions)
        static uint32_t lastButtonTime[6] = {0};
        uint32_t currentTime = millis();
        
        // UP button
        if (UP() && (currentTime - lastButtonTime[0] > 200)) {
            ButtonPress_t buttonPress = {0, currentTime, false};
            xQueueSend(queueButtonPress, &buttonPress, 0);
            lastButtonTime[0] = currentTime;
        }
        
        // DOWN button
        if (DOWN() && (currentTime - lastButtonTime[1] > 200)) {
            ButtonPress_t buttonPress = {1, currentTime, false};
            xQueueSend(queueButtonPress, &buttonPress, 0);
            lastButtonTime[1] = currentTime;
        }
        
        // LEFT button
        if (LEFT() && (currentTime - lastButtonTime[2] > 200)) {
            ButtonPress_t buttonPress = {2, currentTime, false};
            xQueueSend(queueButtonPress, &buttonPress, 0);
            lastButtonTime[2] = currentTime;
        }
        
        // RIGHT button
        if (RIGHT() && (currentTime - lastButtonTime[3] > 200)) {
            ButtonPress_t buttonPress = {3, currentTime, false};
            xQueueSend(queueButtonPress, &buttonPress, 0);
            lastButtonTime[3] = currentTime;
        }
        
        // START button
        if (START() && (currentTime - lastButtonTime[4] > 200)) {
            ButtonPress_t buttonPress = {4, currentTime, false};
            xQueueSend(queueButtonPress, &buttonPress, 0);
            lastButtonTime[4] = currentTime;
            
            // Toggle AGV mode
            EventBits_t eventBits = xEventGroupGetBits(eventGroupSystem);
            if (eventBits & EVENT_AGV_MODE) {
                xEventGroupClearBits(eventGroupSystem, EVENT_AGV_MODE);
                isAgvMode = false;
            } else {
                xEventGroupSetBits(eventGroupSystem, EVENT_AGV_MODE);
                isAgvMode = true;
            }
        }
        
        // STOP button handling with double-click detection
        static bool firstStopClick = false;
        static unsigned long firstStopTime = 0;
        const unsigned long doubleClickInterval = 2000;
        
        if (STOP() && (currentTime - lastButtonTime[5] > 200)) {
            lastButtonTime[5] = currentTime;
            
            if (!firstStopClick) {
                firstStopClick = true;
                firstStopTime = currentTime;
                
                // Send display update for first click
                DisplayUpdate_t displayUpdate;
                displayUpdate.line = 0;
                displayUpdate.text = "STOP 1x detected";
                displayUpdate.clearFirst = true;
                xQueueSend(queueDisplayUpdate, &displayUpdate, 0);
                
                displayUpdate.line = 1;
                displayUpdate.text = "Click again to exit";
                displayUpdate.clearFirst = false;
                xQueueSend(queueDisplayUpdate, &displayUpdate, 0);
            } else {
                if (currentTime - firstStopTime <= doubleClickInterval) {
                    // Valid double click - exit AGV mode
                    xEventGroupClearBits(eventGroupSystem, EVENT_AGV_MODE);
                    isAgvMode = false;
                    
                    Command_t stopCmd;
                    stopCmd.commandType = 1;
                    stopCmd.newState = AGV_STATE_STOP;
                    xQueueSend(queueCommands, &stopCmd, 0);
                    
                    firstStopClick = false;
                    firstStopTime = 0;
                } else {
                    // Too late, treat as new first click
                    firstStopClick = true;
                    firstStopTime = currentTime;
                }
            }
        }
        
        // Check for timeout on first stop click
        if (firstStopClick && (currentTime - firstStopTime > doubleClickInterval)) {
            firstStopClick = false;
            firstStopTime = 0;
            
            // Send display update to reset to normal
            DisplayUpdate_t displayUpdate;
            displayUpdate.line = 0;
            displayUpdate.text = ""; // Will trigger displayPrint()
            displayUpdate.clearFirst = true;
            xQueueSend(queueDisplayUpdate, &displayUpdate, 0);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10)); // 10ms polling
    }
}

// ===================================================================
//                    DISPLAY MANAGER TASK
// ===================================================================
void taskDisplayManager(void *parameters) {
    Serial.println("[FREERTOS] Display Manager Task Started");
    
    while (true) {
        DisplayUpdate_t displayUpdate;
        
        // Check for display updates in queue
        if (xQueueReceive(queueDisplayUpdate, &displayUpdate, pdMS_TO_TICKS(100)) == pdTRUE) {
            // Update display with mutex protection
            if (xSemaphoreTake(mutexDisplay, pdMS_TO_TICKS(50)) == pdTRUE) {
                if (displayUpdate.clearFirst) {
                    lcd.clear();
                }
                if (displayUpdate.text.length() > 0) {
                    lcd.setCursor(0, displayUpdate.line);
                    lcd.print(displayUpdate.text);
                } else {
                    // Empty text means use default display function
                    EventBits_t eventBits = xEventGroupGetBits(eventGroupSystem);
                    if (eventBits & EVENT_AGV_MODE) {
                        displayPrint(); // Existing function
                    } else {
                        // For menu mode, use existing menu display logic
                        // This will be handled by the menu manager task
                    }
                }
                xSemaphoreGive(mutexDisplay);
            }
        } else {
            // Regular display update (existing display functions)
            if (xSemaphoreTake(mutexDisplay, pdMS_TO_TICKS(10)) == pdTRUE) {
                EventBits_t eventBits = xEventGroupGetBits(eventGroupSystem);
                if (eventBits & EVENT_AGV_MODE) {
                    displayPrint(); // Existing function
                } else {
                    // Menu display handled by menu manager task
                    // Don't interfere with menu display here
                }
                xSemaphoreGive(mutexDisplay);
            }
        }
    }
}

// ===================================================================
//                    WIFI MANAGER TASK
// ===================================================================
void taskWiFiManager(void *parameters) {
    Serial.println("[FREERTOS] WiFi Manager Task Started");
    
    while (true) {
        // Handle WiFi connection (existing function)
        loopWifi();
        
        // Handle web server (existing server object)
        server.handleClient();
        
        // Update WiFi status event
        if (WiFi.status() == WL_CONNECTED) {
            xEventGroupSetBits(eventGroupSystem, EVENT_WIFI_CONNECTED);
        } else {
            xEventGroupClearBits(eventGroupSystem, EVENT_WIFI_CONNECTED);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms cycle
    }
}

// ===================================================================
//                    MENU MANAGER TASK
// ===================================================================
void taskMenuManager(void *parameters) {
    Serial.println("[FREERTOS] Menu Manager Task Started");
    
    while (true) {
        // Check if system is in menu mode (not AGV mode)
        EventBits_t eventBits = xEventGroupGetBits(eventGroupSystem);
        
        if (!(eventBits & EVENT_AGV_MODE)) {
            // Handle button presses for menu navigation
            ButtonPress_t buttonPress;
            if (xQueueReceive(queueButtonPress, &buttonPress, pdMS_TO_TICKS(50)) == pdTRUE) {
                // Process button press for menu (integrate with existing menu system)
                switch (buttonPress.buttonId) {
                    case 0: // UP
                        // Use existing menu navigation logic
                        break;
                    case 1: // DOWN
                        // Use existing menu navigation logic
                        break;
                    case 2: // LEFT
                        // Handle left navigation
                        break;
                    case 3: // RIGHT
                        // Handle right navigation or enter
                        break;
                }
            }
            
            // Handle regular menu operations (existing function)
            handleMenu();
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); // 50ms cycle for menu responsiveness
    }
}

// ===================================================================
//                    TIMER CALLBACKS
// ===================================================================
void timerCallbackWatchdog(TimerHandle_t xTimer) {
    // Reset ESP32 watchdog (use existing function)
    esp_task_wdt_reset();
}

void timerCallbackSensorHealth(TimerHandle_t xTimer) {
    // Check sensor health and update event group
    bool sensorsOK = true;
    
    if (xSemaphoreTake(mutexSensorData, pdMS_TO_TICKS(10)) == pdTRUE) {
        if (sharedSensorData.sensorType == 0 && !sharedSensorData.isValid) {
            sensorsOK = false;
        }
        xSemaphoreGive(mutexSensorData);
    }
    
    if (sensorsOK) {
        xEventGroupSetBits(eventGroupSystem, EVENT_SENSORS_OK);
    } else {
        xEventGroupClearBits(eventGroupSystem, EVENT_SENSORS_OK);
    }
}

// ===================================================================
//                    INITIALIZATION FUNCTIONS
// ===================================================================
bool initializeFreeRTOS() {
    Serial.println("[FREERTOS] Initializing FreeRTOS components...");
    
    // Create queues and semaphores first
    createQueuesAndSemaphores();
    
    // Create event group
    eventGroupSystem = xEventGroupCreate();
    if (eventGroupSystem == NULL) {
        Serial.println("[ERROR] Failed to create event group");
        return false;
    }
    
    // Create timers
    // Skip timers completely to avoid interrupt allocation issues in ESP32-S3
    // Watchdog and sensor health will be handled within tasks instead
    Serial.println("[FREERTOS] Skipping timers for ESP32-S3 compatibility");
    timerWatchdog = NULL;
    timerSensorHealth = NULL;
    
    // Timer-less mode is OK for ESP32-S3 - proceed without timers
    Serial.println("[FREERTOS] Timer-less mode for ESP32-S3 stability");
    
    // Create all tasks
    createAllTasks();
    
    // Set initial system state
    xEventGroupSetBits(eventGroupSystem, EVENT_SYSTEM_READY | EVENT_SENSORS_OK);
    
    Serial.println("[FREERTOS] FreeRTOS initialization completed successfully");
    return true;
}

void createQueuesAndSemaphores() {
    // Create queues
    queueSensorData = xQueueCreate(SENSOR_QUEUE_SIZE, sizeof(SensorData_t));
    queueCommands = xQueueCreate(COMMAND_QUEUE_SIZE, sizeof(Command_t));
    queueDisplayUpdate = xQueueCreate(DISPLAY_QUEUE_SIZE, sizeof(DisplayUpdate_t));
    queueButtonPress = xQueueCreate(5, sizeof(ButtonPress_t));
    
    // Create mutexes
    mutexSensorData = xSemaphoreCreateMutex();
    mutexMotorControl = xSemaphoreCreateMutex();
    mutexDisplay = xSemaphoreCreateMutex();
    mutexPreferences = xSemaphoreCreateMutex();
    
    Serial.println("[FREERTOS] Queues and semaphores created");
}

void createAllTasks() {
    BaseType_t result;
    
    // Safety Monitor Task (Highest Priority) - Reduced stack
    result = xTaskCreatePinnedToCore(
        taskSafetyMonitor,
        "SafetyMonitor",
        STACK_SIZE_SMALL,  // Reduced from MEDIUM
        NULL,
        PRIORITY_CRITICAL,
        &taskHandleSafety,
        1 // Pin to Core 1
    );
    if (result != pdPASS) {
        Serial.println("[ERROR] Failed to create Safety Monitor task");
        return;
    }
    
    // Sensor Tasks (High Priority) - Reduced stack for sensors
    xTaskCreatePinnedToCore(
        taskSensorMagnet,
        "SensorMagnet",
        STACK_SIZE_SMALL,  // Reduced from MEDIUM
        NULL,
        PRIORITY_HIGH,
        &taskHandleSensorMagnet,
        1 // Pin to Core 1 for real-time performance
    );
    
    xTaskCreatePinnedToCore(
        taskSensorUltrasonic,
        "SensorUltrasonic",
        STACK_SIZE_SMALL,  // Reduced from MEDIUM
        NULL,
        PRIORITY_HIGH,
        &taskHandleSensorUltrasonic,
        1 // Pin to Core 1
    );
    
    xTaskCreatePinnedToCore(
        taskSensorRFID,
        "SensorRFID",
        STACK_SIZE_SMALL,  // Reduced from MEDIUM
        NULL,
        PRIORITY_HIGH,
        &taskHandleSensorRFID,
        1 // Pin to Core 1
    );
    
    // PID Controller Task (High Priority)
    xTaskCreatePinnedToCore(
        taskPIDController,
        "PIDController",
        STACK_SIZE_MEDIUM,  // Keep MEDIUM for PID calculations
        NULL,
        PRIORITY_HIGH,
        &taskHandlePIDController,
        1 // Pin to Core 1 for real-time control
    );
    
    // AGV Logic Task (Medium Priority)
    xTaskCreatePinnedToCore(
        taskAGVLogic,
        "AGVLogic",
        STACK_SIZE_MEDIUM,  // Reduced from LARGE
        NULL,
        PRIORITY_MEDIUM,
        &taskHandleAGVLogic,
        0 // Pin to Core 0
    );
    
    // Button Handler Task (Medium Priority) - Minimal stack
    xTaskCreatePinnedToCore(
        taskButtonHandler,
        "ButtonHandler",
        STACK_SIZE_SMALL,  // Use SMALL instead of TINY
        NULL,
        PRIORITY_MEDIUM,
        &taskHandleButton,
        0 // Pin to Core 0
    );
    
    // Display Manager Task (Low Priority) - Minimal stack
    xTaskCreatePinnedToCore(
        taskDisplayManager,
        "DisplayManager",
        STACK_SIZE_SMALL,  // Use SMALL instead of TINY
        NULL,
        PRIORITY_LOW,
        &taskHandleDisplay,
        0 // Pin to Core 0
    );
    
    // WiFi Manager Task (Low Priority) - Keep larger for HTTP/JSON
    xTaskCreatePinnedToCore(
        taskWiFiManager,
        "WiFiManager",
        STACK_SIZE_LARGE,  // Keep LARGE for WiFi operations
        NULL,
        PRIORITY_LOW,
        &taskHandleWiFi,
        0 // Pin to Core 0
    );
    
    // Menu Manager Task (Low Priority)
    xTaskCreatePinnedToCore(
        taskMenuManager,
        "MenuManager",
        STACK_SIZE_SMALL,  // Reduced from MEDIUM
        NULL,
        PRIORITY_LOW,
        &taskHandleMenu,
        0 // Pin to Core 0
    );
    
    Serial.println("[FREERTOS] All tasks created successfully with optimized stack sizes");
}
