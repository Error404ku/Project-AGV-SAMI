#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

// ===================================================================
//                    FREERTOS CONFIGURATION
// ===================================================================

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/timers.h>
#include <freertos/event_groups.h>

// ===================================================================
//                    TASK PRIORITIES - OPTIMIZED
// ===================================================================
#define PRIORITY_CRITICAL     4    // Reduced from 5 - Safety & Emergency
#define PRIORITY_HIGH         3    // Reduced from 4 - Sensor readings & PID
#define PRIORITY_MEDIUM       2    // Reduced from 3 - Motor control & Logic
#define PRIORITY_LOW          1    // Reduced from 2 - WiFi, Display, Menu
#define PRIORITY_IDLE         1    // Same - Background tasks

// ===================================================================
//                    TASK STACK SIZES - OPTIMIZED
// ===================================================================
#define STACK_SIZE_SMALL      1536   // Reduced from 2048 - Simple tasks
#define STACK_SIZE_MEDIUM     2048   // Reduced from 4096 - Standard tasks  
#define STACK_SIZE_LARGE      3072   // Reduced from 8192 - Complex tasks with JSON

// ===================================================================
//                    QUEUE SIZES - OPTIMIZED
// ===================================================================
#define SENSOR_QUEUE_SIZE     5     // Reduced from 10
#define COMMAND_QUEUE_SIZE    3     // Reduced from 5
#define DISPLAY_QUEUE_SIZE    2     // Reduced from 3

// ===================================================================
//                    TIMER CONFIGURATION - SAFE MODE
// ===================================================================
#define FREERTOS_TIMER_TASK_PRIORITY    1    // Low priority
#define FREERTOS_TIMER_TASK_STACK_SIZE  1536 // Reduced stack
#define FREERTOS_TIMER_QUEUE_LENGTH     3    // Reduced from 10
#define FREERTOS_COMMAND_QUEUE_LENGTH   3    // Reduced queue
#define WATCHDOG_TIMER_PERIOD_MS        5000 // 5 seconds
#define SENSOR_HEALTH_TIMER_PERIOD_MS   2000 // 2 seconds

// ===================================================================
//                    FREERTOS HANDLES
// ===================================================================

// Task Handles
extern TaskHandle_t taskHandleSafety;
extern TaskHandle_t taskHandleSensorMagnet;
extern TaskHandle_t taskHandleSensorUltrasonic;
extern TaskHandle_t taskHandleSensorRFID;
extern TaskHandle_t taskHandlePIDController;
extern TaskHandle_t taskHandleMotorControl;
extern TaskHandle_t taskHandleAGVLogic;
extern TaskHandle_t taskHandleWiFi;
extern TaskHandle_t taskHandleDisplay;
extern TaskHandle_t taskHandleButton;
extern TaskHandle_t taskHandleMenu;

// Queue Handles
extern QueueHandle_t queueSensorData;
extern QueueHandle_t queueCommands;
extern QueueHandle_t queueDisplayUpdate;
extern QueueHandle_t queueButtonPress;

// Semaphore Handles
extern SemaphoreHandle_t mutexSensorData;
extern SemaphoreHandle_t mutexMotorControl;
extern SemaphoreHandle_t mutexDisplay;
extern SemaphoreHandle_t mutexPreferences;

// Event Group Handles
extern EventGroupHandle_t eventGroupSystem;

// Timer Handles
extern TimerHandle_t timerWatchdog;
extern TimerHandle_t timerSensorHealth;

// ===================================================================
//                    EVENT BITS
// ===================================================================
#define EVENT_SYSTEM_READY     BIT0
#define EVENT_SENSORS_OK       BIT1
#define EVENT_EMERGENCY_STOP   BIT2
#define EVENT_WIFI_CONNECTED   BIT3
#define EVENT_AGV_MODE         BIT4

// ===================================================================
//                    DATA STRUCTURES
// ===================================================================

// Sensor data structure for queue
typedef struct {
    uint32_t timestamp;
    uint8_t sensorType;    // 0=magnet, 1=ultrasonic, 2=rfid
    int16_t errorValue;    // For magnet sensor
    uint16_t distance;     // For ultrasonic sensor
    String rfidId;         // For RFID sensor
    bool isValid;
} SensorData_t;

// Command structure for queue
typedef struct {
    uint8_t commandType;   // 0=move, 1=stop, 2=mode_change
    int16_t param1;
    int16_t param2;
    AgvState newState;
} Command_t;

// Display update structure
typedef struct {
    uint8_t line;
    String text;
    bool clearFirst;
} DisplayUpdate_t;

// Button press structure
typedef struct {
    uint8_t buttonId;
    uint32_t pressTime;
    bool isLongPress;
} ButtonPress_t;

// ===================================================================
//                    FUNCTION PROTOTYPES
// ===================================================================

// Task functions
void taskSafetyMonitor(void *parameters);
void taskSensorMagnet(void *parameters);
void taskSensorUltrasonic(void *parameters);
void taskSensorRFID(void *parameters);
void taskPIDController(void *parameters);
void taskMotorControl(void *parameters);
void taskAGVLogic(void *parameters);
void taskWiFiManager(void *parameters);
void taskDisplayManager(void *parameters);
void taskButtonHandler(void *parameters);
void taskMenuManager(void *parameters);

// Timer callbacks
void timerCallbackWatchdog(TimerHandle_t xTimer);
void timerCallbackSensorHealth(TimerHandle_t xTimer);

// Initialization functions
bool initializeFreeRTOS();
void createAllTasks();
void createQueuesAndSemaphores();

#endif
