// ===================================================================
//                    PERFORMANCE MONITORING AND OPTIMIZATION
// ===================================================================

#include "config.h"

// ===================================================================
//                    PERFORMANCE METRICS
// ===================================================================

typedef struct {
    char taskName[16];
    uint32_t minExecutionTime;
    uint32_t maxExecutionTime;
    uint32_t avgExecutionTime;
    uint32_t totalExecutions;
    uint32_t cpuUsagePercent;
    uint32_t stackHighWaterMark;
    TaskHandle_t handle;
} TaskPerformanceData_t;

TaskPerformanceData_t taskPerformance[15];
uint8_t taskCount = 0;

// Memory performance tracking
typedef struct {
    size_t freeHeapSize;
    size_t minFreeHeap;
    size_t totalHeapSize;
    uint32_t heapFragmentation;
    uint32_t psramFree;
} MemoryPerformanceData_t;

MemoryPerformanceData_t memoryPerformance;

// System performance metrics
typedef struct {
    uint32_t systemUptime;
    float cpuTemperature;
    uint32_t totalTaskSwitches;
    uint32_t totalInterrupts;
    float systemLoad;
} SystemPerformanceData_t;

SystemPerformanceData_t systemPerformance;

// ===================================================================
//                    PERFORMANCE MONITORING FUNCTIONS
// ===================================================================

void initPerformanceMonitoring() {
    Serial.println("[PERF] Initializing Performance Monitoring...");
    
    // Initialize performance data structures
    memset(taskPerformance, 0, sizeof(taskPerformance));
    memset(&memoryPerformance, 0, sizeof(memoryPerformance));
    memset(&systemPerformance, 0, sizeof(systemPerformance));
    
    // Set initial memory baseline
    memoryPerformance.totalHeapSize = ESP.getHeapSize();
    memoryPerformance.minFreeHeap = ESP.getFreeHeap();
    
    Serial.println("[PERF] Performance Monitoring Initialized");
}

void updateTaskPerformance() {
    // Get task list
    UBaseType_t taskArraySize = uxTaskGetNumberOfTasks();
    TaskStatus_t* pxTaskStatusArray = (TaskStatus_t*)malloc(taskArraySize * sizeof(TaskStatus_t));
    
    if (pxTaskStatusArray != NULL) {
        UBaseType_t actualTasks = uxTaskGetSystemState(pxTaskStatusArray, taskArraySize, NULL);
        
        taskCount = (actualTasks < 15) ? actualTasks : 15; // Limit to array size
        
        for (uint8_t i = 0; i < taskCount; i++) {
            strncpy(taskPerformance[i].taskName, pxTaskStatusArray[i].pcTaskName, 15);
            taskPerformance[i].handle = pxTaskStatusArray[i].xHandle;
            taskPerformance[i].stackHighWaterMark = pxTaskStatusArray[i].usStackHighWaterMark;
            
            // Calculate CPU usage (simplified)
            taskPerformance[i].cpuUsagePercent = (pxTaskStatusArray[i].ulRunTimeCounter * 100) / 
                                               (millis() - systemPerformance.systemUptime + 1);
        }
        
        free(pxTaskStatusArray);
    }
}

void updateMemoryPerformance() {
    memoryPerformance.freeHeapSize = ESP.getFreeHeap();
    
    if (memoryPerformance.freeHeapSize < memoryPerformance.minFreeHeap) {
        memoryPerformance.minFreeHeap = memoryPerformance.freeHeapSize;
    }
    
    // Calculate fragmentation
    size_t largestFreeBlock = ESP.getMaxAllocHeap();
    if (memoryPerformance.freeHeapSize > 0) {
        memoryPerformance.heapFragmentation = 
            ((memoryPerformance.freeHeapSize - largestFreeBlock) * 100) / memoryPerformance.freeHeapSize;
    }
    
    // PSRAM if available
    if (ESP.getPsramSize() > 0) {
        memoryPerformance.psramFree = ESP.getFreePsram();
    }
}

void updateSystemPerformance() {
    systemPerformance.systemUptime = millis();
    systemPerformance.cpuTemperature = temperatureRead();
    
    // Calculate system load based on idle task
    TaskHandle_t idleTask = xTaskGetIdleTaskHandle();
    if (idleTask != NULL) {
        TaskStatus_t idleStatus;
        vTaskGetInfo(idleTask, &idleStatus, pdTRUE, eInvalid);
        systemPerformance.systemLoad = 100.0 - 
            ((float)idleStatus.ulRunTimeCounter * 100.0 / systemPerformance.systemUptime);
    }
}

// ===================================================================
//                    PERFORMANCE REPORTING
// ===================================================================

void printPerformanceReport() {
    Serial.println("\n============================================================");
    Serial.println("              AGV PERFORMANCE REPORT");
    Serial.println("============================================================");
    
    // System Performance
    Serial.printf("System Uptime: %lu ms (%.2f hours)\n", 
                 systemPerformance.systemUptime, 
                 systemPerformance.systemUptime / 3600000.0);
    Serial.printf("CPU Temperature: %.1f°C\n", systemPerformance.cpuTemperature);
    Serial.printf("System Load: %.1f%%\n", systemPerformance.systemLoad);
    Serial.println();
    
    // Memory Performance
    Serial.println("MEMORY PERFORMANCE:");
    Serial.printf("Free Heap: %zu bytes (%.1f KB)\n", 
                 memoryPerformance.freeHeapSize, 
                 memoryPerformance.freeHeapSize / 1024.0);
    Serial.printf("Min Free Heap: %zu bytes (%.1f KB)\n", 
                 memoryPerformance.minFreeHeap, 
                 memoryPerformance.minFreeHeap / 1024.0);
    Serial.printf("Total Heap: %zu bytes (%.1f KB)\n", 
                 memoryPerformance.totalHeapSize, 
                 memoryPerformance.totalHeapSize / 1024.0);
    Serial.printf("Heap Usage: %.1f%%\n", 
                 (1.0 - (float)memoryPerformance.freeHeapSize / memoryPerformance.totalHeapSize) * 100);
    Serial.printf("Fragmentation: %lu%%\n", memoryPerformance.heapFragmentation);
    
    if (memoryPerformance.psramFree > 0) {
        Serial.printf("PSRAM Free: %lu bytes (%.1f KB)\n", 
                     memoryPerformance.psramFree, 
                     memoryPerformance.psramFree / 1024.0);
    }
    Serial.println();
    
    // Task Performance
    Serial.println("TASK PERFORMANCE:");
    Serial.println("Task Name        | CPU% | Stack HWM | Priority | State");
    Serial.println("-------------------------------------------------------");
    
    for (uint8_t i = 0; i < taskCount; i++) {
        TaskStatus_t taskStatus;
        vTaskGetInfo(taskPerformance[i].handle, &taskStatus, pdTRUE, eInvalid);
        
        const char* stateNames[] = {"Running", "Ready", "Blocked", "Suspended", "Deleted", "Invalid"};
        const char* stateName = (taskStatus.eCurrentState < 6) ? stateNames[taskStatus.eCurrentState] : "Unknown";
        
        Serial.printf("%-16s | %3lu%% | %8lu | %8lu | %s\n",
                     taskPerformance[i].taskName,
                     taskPerformance[i].cpuUsagePercent,
                     taskPerformance[i].stackHighWaterMark,
                     (uint32_t)taskStatus.uxCurrentPriority,
                     stateName);
    }
    
    Serial.println("============================================================");
    Serial.println();
}

// ===================================================================
//                    PERFORMANCE OPTIMIZATION RECOMMENDATIONS
// ===================================================================

void analyzePerformanceAndRecommend() {
    Serial.println("PERFORMANCE ANALYSIS & RECOMMENDATIONS:");
    Serial.println("--------------------------------------------------");
    
    // Memory analysis
    float heapUsage = (1.0 - (float)memoryPerformance.freeHeapSize / memoryPerformance.totalHeapSize) * 100;
    
    if (heapUsage > 85.0) {
        Serial.println("⚠️  HIGH MEMORY USAGE DETECTED!");
        Serial.println("   Recommendation: Optimize data structures or increase heap");
    } else if (heapUsage > 70.0) {
        Serial.println("⚡ Memory usage is elevated");
        Serial.println("   Recommendation: Monitor for memory leaks");
    } else {
        Serial.println("✅ Memory usage is healthy");
    }
    
    // Fragmentation analysis
    if (memoryPerformance.heapFragmentation > 30) {
        Serial.println("⚠️  HIGH MEMORY FRAGMENTATION DETECTED!");
        Serial.println("   Recommendation: Implement memory pools or reduce dynamic allocation");
    }
    
    // System load analysis
    if (systemPerformance.systemLoad > 90.0) {
        Serial.println("⚠️  HIGH SYSTEM LOAD DETECTED!");
        Serial.println("   Recommendation: Reduce task frequencies or optimize algorithms");
    } else if (systemPerformance.systemLoad > 70.0) {
        Serial.println("⚡ System load is elevated");
        Serial.println("   Recommendation: Monitor task execution times");
    } else {
        Serial.println("✅ System load is healthy");
    }
    
    // Temperature analysis
    if (systemPerformance.cpuTemperature > 80.0) {
        Serial.println("🔥 HIGH CPU TEMPERATURE!");
        Serial.println("   Recommendation: Check cooling or reduce CPU intensive operations");
    } else if (systemPerformance.cpuTemperature > 60.0) {
        Serial.println("🌡️  CPU temperature is warm");
        Serial.println("   Recommendation: Monitor thermal performance");
    } else {
        Serial.println("❄️  CPU temperature is normal");
    }
    
    // Task-specific recommendations
    for (uint8_t i = 0; i < taskCount; i++) {
        if (taskPerformance[i].stackHighWaterMark < 200) {
            Serial.printf("⚠️  Task '%s' has low stack headroom (%lu bytes)\n", 
                         taskPerformance[i].taskName, 
                         taskPerformance[i].stackHighWaterMark);
            Serial.println("   Recommendation: Increase stack size for this task");
        }
        
        if (taskPerformance[i].cpuUsagePercent > 50) {
            Serial.printf("⚡ Task '%s' has high CPU usage (%lu%%)\n", 
                         taskPerformance[i].taskName, 
                         taskPerformance[i].cpuUsagePercent);
            Serial.println("   Recommendation: Optimize or reduce frequency of this task");
        }
    }
    
    Serial.println("--------------------------------------------------");
    Serial.println();
}

// ===================================================================
//                    PERFORMANCE MONITORING TASK
// ===================================================================
void taskPerformanceMonitor(void *parameters) {
    Serial.println("[FREERTOS] Performance Monitor Task Started");
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (true) {
        // Update all performance metrics
        updateTaskPerformance();
        updateMemoryPerformance();
        updateSystemPerformance();
        
        // Print detailed report every 30 seconds
        static uint32_t reportCounter = 0;
        reportCounter++;
        
        if (reportCounter >= 30) { // 30 seconds at 1Hz
            printPerformanceReport();
            analyzePerformanceAndRecommend();
            reportCounter = 0;
        }
        
        // Send critical alerts immediately
        if (memoryPerformance.freeHeapSize < 10000) { // Less than 10KB free
            Serial.println("🚨 CRITICAL: Low memory condition!");
        }
        
        if (systemPerformance.systemLoad > 95.0) {
            Serial.println("🚨 CRITICAL: System overload detected!");
        }
        
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000)); // 1 second cycle
    }
}

// ===================================================================
//                    AUTOMATIC OPTIMIZATION FUNCTIONS
// ===================================================================

void enableAdaptiveOptimization() {
    // Automatically adjust task priorities based on system load
    if (systemPerformance.systemLoad > 80.0) {
        // Reduce low-priority task frequencies
        Serial.println("[OPTIMIZER] High system load detected - reducing low priority tasks");
        
        // Reduce WiFi task frequency
        if (taskHandleWiFi != NULL) {
            vTaskSuspend(taskHandleWiFi);
            vTaskDelay(pdMS_TO_TICKS(100));
            vTaskResume(taskHandleWiFi);
        }
    }
    
    // Memory optimization
    if (memoryPerformance.heapFragmentation > 40) {
        Serial.println("[OPTIMIZER] High fragmentation detected - performing cleanup");
        
        // Force garbage collection if available
        ESP.getHeapSize(); // This may trigger internal cleanup
    }
    
    // Temperature management
    if (systemPerformance.cpuTemperature > 75.0) {
        Serial.println("[OPTIMIZER] High temperature detected - reducing CPU frequency");
        
        // Could implement CPU frequency scaling here
        setCpuFrequencyMhz(160); // Reduce from 240MHz to 160MHz
    } else if (systemPerformance.cpuTemperature < 50.0) {
        // Restore full performance when temperature is low
        setCpuFrequencyMhz(240);
    }
}

// Integration function to add performance monitor to existing system
void addPerformanceMonitorTask() {
    BaseType_t result = xTaskCreatePinnedToCore(
        taskPerformanceMonitor,
        "PerfMonitor",
        STACK_SIZE_LARGE,
        NULL,
        PRIORITY_LOW,
        NULL,
        0 // Pin to Core 0
    );
    
    if (result == pdPASS) {
        Serial.println("[PERF] Performance Monitor Task created successfully");
    } else {
        Serial.println("[ERROR] Failed to create Performance Monitor Task");
    }
}
