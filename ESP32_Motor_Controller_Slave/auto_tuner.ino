// auto_tuner.ino
// Adaptive PID Auto-Tuning untuk RPM Motor
// Menggunakan metode Analisis Offline Cerdas dengan State Machine

// =============================================
// CONSTANTS & CONFIGURATION
// =============================================

// Tuning Parameters - Kp-First RPM Strategy
const int TUNING_TARGET_RPM = 40;              // Target RPM untuk pengujian (SINKRON dengan maxMotorRpm di Master)
const unsigned long TEST_DURATION_MS = 20000;  // 20 detik per siklus untuk deteksi overshooting/undershooting yang lebih akurat
const unsigned long COOLDOWN_DURATION_MS = 1000; // 1 detik cooldown
const int MAX_TUNING_CYCLES = 15;              // 15 cycles total

// Analysis Thresholds - Kp-First RPM Control Strategy
const float RISE_TIME_START_PERCENT = 0.1;    // 10% untuk start rise time (Kp focus)
const float RISE_TIME_END_PERCENT = 0.9;      // 90% untuk end rise time (Kp focus)
const float HIGH_OVERSHOOT_THRESHOLD = 15.0;   // Overshoot tinggi (Kp toleran lebih tinggi)
const float MEDIUM_OVERSHOOT_THRESHOLD = 8.0;  // Overshoot sedang (Kp focus response)
const float LOW_OVERSHOOT_THRESHOLD = 3.0;     // Overshoot rendah
const unsigned long SLOW_RISE_TIME_MS = 5000;  // Rise time lambat (5 detik untuk 20s test)
const unsigned long MEDIUM_RISE_TIME_MS = 2500; // Rise time sedang (2.5 detik untuk 20s test)

// PID Adjustment Factors - Kp-First RPM Strategy (RESPONSE-BASED OPTIMIZATION)
// Kp adalah PRIORITAS UTAMA untuk RPM control (response time dan immediate reaction)
const float KP_COARSE_STEP = 4.0;             // Kp PRIORITAS untuk motor response
const float KP_FINE_STEP = 1.5;               // Kp PRIORITAS untuk fine-tuning
const float KP_ULTRA_FINE_STEP = 0.5;         // Kp PRIORITAS untuk precision

const float KI_COARSE_STEP = 2.0;             // Ki supportive untuk Kp dominance
const float KI_FINE_STEP = 0.8;               // Ki supportive fine-tuning
const float KI_ULTRA_FINE_STEP = 0.2;         // Ki supportive precision

const float KD_COARSE_STEP = 0.1;             // Kd untuk damping overshoot
const float KD_FINE_STEP = 0.04;              // Kd supportive fine-tuning
const float KD_ULTRA_FINE_STEP = 0.01;        // Kd supportive precision

// Precision Stages
enum TuningPrecision {
  PRECISION_COARSE,     // Pencarian awal dengan step besar
  PRECISION_FINE,       // Pencarian menengah dengan step sedang
  PRECISION_ULTRA_FINE  // Pencarian akhir dengan step kecil
};

// Scoring Weights - Kp-First RPM Strategy
// Kp (response time dan immediate reaction) adalah PRIORITAS UTAMA untuk RPM control
const float OVERSHOOT_WEIGHT = 1.5;            // Bobot overshoot (moderat, Kp toleran overshoot)
const float RISE_TIME_WEIGHT = 0.008;          // Bobot rise time (penting untuk Kp response)
const float ERROR_WEIGHT = 6.0;               // Bobot average error (balanced untuk Kp)
const float STABILITY_WEIGHT = 4.0;            // Bobot stabilitas (moderat untuk Kp)
const float INITIAL_BURST_PENALTY = 1.0;       // Penalty burst (minimal, Kp dapat handle burst)

// Performance Thresholds - Kp-First Response Optimization
const float EXCELLENT_SCORE_THRESHOLD = 25.0;  // Threshold untuk Kp quality
const float GOOD_SCORE_THRESHOLD = 40.0;       // Threshold untuk Kp evaluation

// Basic tuning factors (Kp-first RPM strategy)
const float KP_AGGRESSIVE_REDUCTION = 0.6;     // Moderat untuk RPM (Kp focus)
const float KP_MODERATE_REDUCTION = 0.8;       // Moderat untuk RPM (Kp focus)
const float KP_INCREASE_FACTOR = 1.4;          // Agresif untuk Kp dominance
const float KD_DAMPING_INCREASE = 1.8;         // Moderat untuk Kp stability
const float KI_OPTIMIZATION_FACTOR = 1.15;     // Ki supportive optimization
const float ACCEPTABLE_SCORE_THRESHOLD = 45.0; // Threshold untuk Kp quality

// Display & Debug
const unsigned long DEBUG_PRINT_INTERVAL = 500; // Interval print debug (ms)
const float INITIAL_BEST_SCORE = 999999.0;     // Skor awal yang sangat tinggi

// =============================================
// STATE MACHINE & ENUMS
// =============================================

enum TuningState {
  TUNING_IDLE,
  TUNING_STARTING,
  TUNING_RAMP_UP,
  TUNING_ANALYZING,
  TUNING_COOLDOWN,
  TUNING_UPDATING_PARAMS,
  TUNING_SWITCH_MOTOR,     // State untuk switch dari kanan ke kiri di TUNE_BOTH
  TUNING_FINISHED
};

// =============================================
// GLOBAL VARIABLES
// =============================================

// State Management
TuningState currentTuningState = TUNING_IDLE;
TuningTarget currentTuningTarget = TUNE_BOTH;
TuningPrecision currentPrecision = PRECISION_COARSE;
int tuningCycleCount = 0;
int stagnationCount = 0;               // Hitung berapa siklus tanpa improvement
int precisionStageCount = 0;           // Counter untuk precision stage transition (FIXED: start from 0)

// Dual Motor Tuning Variables for TUNE_BOTH
bool isRightMotorPhase = true;         // true = sedang tune motor kanan, false = sedang tune motor kiri
bool rightMotorCompleted = false;      // Flag motor kanan sudah selesai
bool leftMotorCompleted = false;       // Flag motor kiri sudah selesai

// Best parameters untuk masing-masing motor (TUNE_BOTH mode)
double bestKpRight, bestKiRight, bestKdRight;
double bestKpLeft, bestKiLeft, bestKdLeft;
float bestScoreRight = INITIAL_BEST_SCORE;
float bestScoreLeft = INITIAL_BEST_SCORE;

// Multi-stage Tuning Control
bool kpOptimized = false;              // Flag untuk Kp sudah optimal
bool kiOptimized = false;              // Flag untuk Ki sudah optimal  
bool kdOptimized = false;              // Flag untuk Kd sudah optimal
float bestScoreInStage = 999999.0;     // Best score dalam stage saat ini
int cyclesWithoutImprovement = 0;      // Siklus tanpa improvement

// Precision Search Parameters
float currentKpStep = KP_COARSE_STEP;
float currentKiStep = KI_COARSE_STEP;
float currentKdStep = KD_COARSE_STEP;

// Timing
unsigned long cycleStartTime = 0;

// Performance Metrics
struct PerformanceMetrics {
  float maxOvershoot;
  unsigned long riseTime;
  unsigned long settlingTime;
  bool hasCrossed10pct;
  bool hasCrossed90pct;
  float totalError;
  int sampleCount;
  float initialBurst;         // Track initial burst speed (BARU)
  bool burstDetected;         // Flag untuk deteksi burst (BARU)
  unsigned long burstTime;    // Waktu saat burst terjadi (BARU)
  
  void reset() {
    maxOvershoot = 0;
    riseTime = 0;
    settlingTime = 0;
    hasCrossed10pct = false;
    hasCrossed90pct = false;
    totalError = 0;
    sampleCount = 0;
    initialBurst = 0;
    burstDetected = false;
    burstTime = 0;
  }
  
  float getAverageError() const {
    return (sampleCount > 0) ? (totalError / sampleCount) : 0;
  }
};

PerformanceMetrics currentMetrics;

// PID Parameters (keeping original variables for compatibility)
double bestKp, bestKi, bestKd;
double currentKp, currentKi, currentKd;
float bestScore = INITIAL_BEST_SCORE;

// =============================================
// HELPER FUNCTIONS
// =============================================

// Fungsi untuk mendapatkan RPM rata-rata dari kedua motor (untuk backward compatibility)
float getAverageRPM() {
  return (float)(rpm_depan_kanan + rpm_depan_kiri) / 2.0;
}

// Helper function untuk constrain PID values
void constrainPIDValues(double& kp, double& ki, double& kd, double minVal = 0.0, double maxVal = 200.0) {
  kp = constrain(kp, minVal, maxVal);
  ki = constrain(ki, minVal, maxVal);
  kd = constrain(kd, minVal, maxVal);
}

// Helper function untuk menghitung performance score dengan Kp-First Strategy
float calculatePerformanceScore() {
  float avgError = currentMetrics.getAverageError();
  float overshoot = currentMetrics.maxOvershoot;
  unsigned long riseTime = currentMetrics.riseTime;
  
  // Kp-First RPM score: response time dan immediate reaction adalah PRIORITAS UTAMA
  float score = overshoot * OVERSHOOT_WEIGHT + 
                (float)riseTime * RISE_TIME_WEIGHT + 
                avgError * ERROR_WEIGHT;
  
  // Kp-First penalty: Kp terlalu kecil untuk RPM = buruk response
  if (currentKp < 1.0) score += 30.0;   // Heavy penalty untuk Kp terlalu kecil
  if (currentKp < 3.0) score += 15.0;   // Moderate penalty
  
  // Kp-First bonus: Kp optimal untuk RPM response = bagus
  if (currentKp >= 5.0 && currentKp <= 25.0) score -= 8.0;  // Bonus Kp optimal range
  
  // Ki/Kd penalty: terlalu besar tidak baik untuk Kp-First RPM
  if (currentKi > 10.0) score += 5.0;   // Penalty Ki terlalu besar (Kp should dominate)
  if (currentKd > 1.0) score += 6.0;    // Penalty Kd terlalu besar
  
  // Kp-First balance check: rasio Kp vs Ki untuk RPM response
  float kpKiRatio = currentKp / (currentKi + 0.01);  // Prevent division by zero
  if (kpKiRatio < 0.8) score += 12.0;   // Kp terlalu kecil dibanding Ki
  if (kpKiRatio > 3.0) score -= 5.0;    // Kp dominan = bagus untuk response
  
  // Response bonus untuk Kp immediate reaction
  if (riseTime < 2000 && overshoot < 20.0) {
    float responseBonus = (2000.0 - (float)riseTime) / 200.0;
    score -= responseBonus;  // Bonus untuk response cepat dengan overshoot terkontrol
  }
  
  // Stability bonus untuk Kp performance
  if (currentMetrics.sampleCount > 50) {
    float stability = (float)currentMetrics.sampleCount / 100.0;
    score -= stability * STABILITY_WEIGHT;  // Stability penting untuk Kp performance
  }
  
  return score;
}

// Helper function untuk print debug info
void printCycleInfo(int cycle, double kp, double ki, double kd) {
  Serial.println("======================================");
  Serial.printf("Memulai siklus tuning %d/%d\n", cycle, MAX_TUNING_CYCLES);
  Serial.printf("Testing: Kp=%.4f, Ki=%.4f, Kd=%.4f\n", kp, ki, kd);
}

// Helper function untuk print hasil analisis
void printAnalysisResults(float score, float bestScore) {
  Serial.printf("Hasil siklus:\n");
  Serial.printf("  - Overshoot: %.2f%%\n", currentMetrics.maxOvershoot);
  Serial.printf("  - Rise Time: %lu ms\n", currentMetrics.riseTime);
  Serial.printf("  - Avg Error: %.2f RPM\n", currentMetrics.getAverageError());
  Serial.printf("Skor: %.2f (Terbaik sejauh ini: %.2f)\n", score, bestScore);
}

// Helper function untuk update best parameters
bool updateBestParameters(float score) {
  if (currentTuningTarget == TUNE_BOTH) {
    // Untuk TUNE_BOTH, simpan parameter terbaik untuk motor yang sedang di-tune
    if (isRightMotorPhase) {
      if (score < bestScoreRight) {
        bestScoreRight = score;
        bestKpRight = currentKp;
        bestKiRight = currentKi;
        bestKdRight = currentKd;
        Serial.println(">>> DITEMUKAN PARAMETER TERBAIK BARU UNTUK MOTOR KANAN! <<<");
        return true;
      }
    } else {
      if (score < bestScoreLeft) {
        bestScoreLeft = score;
        bestKpLeft = currentKp;
        bestKiLeft = currentKi;
        bestKdLeft = currentKd;
        Serial.println(">>> DITEMUKAN PARAMETER TERBAIK BARU UNTUK MOTOR KIRI! <<<");
        return true;
      }
    }
  } else {
    // Untuk TUNE_RIGHT dan TUNE_LEFT, gunakan logic original
    if (score < bestScore) {
      bestScore = score;
      bestKp = currentKp;
      bestKi = currentKi;
      bestKd = currentKd;
      Serial.println(">>> DITEMUKAN PARAMETER TERBAIK BARU! <<<");
      return true;
    }
  }
  return false;
}

// Fungsi untuk mendapatkan RPM sesuai target tuning
float getTargetRPM() {
  switch (currentTuningTarget) {
    case TUNE_RIGHT:
      return (float)rpm_depan_kanan;
    case TUNE_LEFT:
      return (float)rpm_depan_kiri;
    case TUNE_BOTH:
    default:
      // Untuk TUNE_BOTH, return RPM motor yang sedang di-tune
      if (isRightMotorPhase) {
        return (float)rpm_depan_kanan;
      } else {
        return (float)rpm_depan_kiri;
      }
  }
}

// Fungsi untuk mengatur target RPM sesuai motor yang di-tuning
void setTuningTargetRPM(float targetRPM) {
  Serial.printf("[DEBUG] setTuningTargetRPM called: %.1f RPM, Target: %d\n", targetRPM, currentTuningTarget);
  
  // Debug: Print current PID values
  Serial.printf("[DEBUG] Right Motor PID: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", 
                pidConfigRight.kp, pidConfigRight.ki, pidConfigRight.kd);
  Serial.printf("[DEBUG] Left Motor PID: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", 
                pidConfigLeft.kp, pidConfigLeft.ki, pidConfigLeft.kd);
  
  // Store PWM values before applying
  int oldPwmKanan = pwmKanan;
  int oldPwmKiri = pwmKiri;
  
  switch (currentTuningTarget) {
    case TUNE_RIGHT:
      rpmMotor(targetRPM, 0);  // Hanya motor kanan, motor kiri diam
      Serial.printf("[DEBUG] Setting RIGHT motor to %.1f RPM\n", targetRPM);
      Serial.printf("[DEBUG] PWM Output - Kanan: %d (was: %d), Kiri: %d (was: %d)\n", 
                    pwmKanan, oldPwmKanan, pwmKiri, oldPwmKiri);
      break;
    case TUNE_LEFT:
      rpmMotor(0, targetRPM);  // Hanya motor kiri, motor kanan diam
      Serial.printf("[DEBUG] Setting LEFT motor to %.1f RPM\n", targetRPM);
      Serial.printf("[DEBUG] PWM Output - Kanan: %d (was: %d), Kiri: %d (was: %d)\n", 
                    pwmKanan, oldPwmKanan, pwmKiri, oldPwmKiri);
      break;
    case TUNE_BOTH:
    default:
      // Untuk TUNE_BOTH, jalankan motor yang sedang di-tune, motor lain diam
      if (isRightMotorPhase) {
        rpmMotor(targetRPM, 0);  // Hanya motor kanan, motor kiri diam
        Serial.printf("[DEBUG] TUNE_BOTH: Setting RIGHT motor to %.1f RPM (LEFT motor OFF)\n", targetRPM);
      } else {
        rpmMotor(0, targetRPM);  // Hanya motor kiri, motor kanan diam
        Serial.printf("[DEBUG] TUNE_BOTH: Setting LEFT motor to %.1f RPM (RIGHT motor OFF)\n", targetRPM);
      }
      Serial.printf("[DEBUG] PWM Output - Kanan: %d (was: %d), Kiri: %d (was: %d)\n", 
                    pwmKanan, oldPwmKanan, pwmKiri, oldPwmKiri);
      break;
  }
}

// Fungsi untuk mengatur nilai PID sesuai target motor
void setTuningPID(double kp, double ki, double kd) {
  switch (currentTuningTarget) {
    case TUNE_RIGHT:
      pidConfigRight.kp = kp;
      pidConfigRight.ki = ki;
      pidConfigRight.kd = kd;
      break;
    case TUNE_LEFT:
      pidConfigLeft.kp = kp;
      pidConfigLeft.ki = ki;
      pidConfigLeft.kd = kd;
      break;
    case TUNE_BOTH:
    default:
      // Untuk TUNE_BOTH, update motor yang sedang di-tune
      if (isRightMotorPhase) {
        pidConfigRight.kp = kp;
        pidConfigRight.ki = ki;
        pidConfigRight.kd = kd;
        Serial.printf("[DEBUG] Updating RIGHT motor PID: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", kp, ki, kd);
      } else {
        pidConfigLeft.kp = kp;
        pidConfigLeft.ki = ki;
        pidConfigLeft.kd = kd;
        Serial.printf("[DEBUG] Updating LEFT motor PID: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", kp, ki, kd);
      }
      break;
  }
  
  // Reset semua PID data untuk memulai dari kondisi bersih
  for(int i = 0; i < numOutputs; i++) {
    pidData[i].error = 0.0;
    pidData[i].previousError = 0.0;
    pidData[i].integral = 0.0;
    pidData[i].derivative = 0.0;
  }
}

// Fungsi utama yang dipanggil di loop()
void handleAutoTuning() {
  if (currentTuningState == TUNING_IDLE) {
    return; // Tidak melakukan apa-apa jika tidak sedang tuning
  }

  unsigned long currentTime = millis();
  
  // Debug output untuk monitoring state
  static unsigned long lastDebugTime = 0;
  if (currentTime - lastDebugTime >= 1000) {  // Print setiap detik
    Serial.printf("[DEBUG] AutoTuning State: %d, Target: %d\n", currentTuningState, currentTuningTarget);
    lastDebugTime = currentTime;
  }

  switch (currentTuningState) {
    case TUNING_STARTING:
      Serial.println("======================================");
      Serial.printf("Memulai siklus tuning %d/%d\n", tuningCycleCount + 1, MAX_TUNING_CYCLES);
      Serial.printf("Testing: Kp=%.4f, Ki=%.4f, Kd=%.4f\n", currentKp, currentKi, currentKd);
      
      // Reset metrik untuk siklus ini
      currentMetrics.reset();
      
      // Terapkan nilai PID baru
      setTuningPID(currentKp, currentKi, currentKd);
      
      // Mulai tes - motor dari berhenti ke target RPM
      setTuningTargetRPM(TUNING_TARGET_RPM);
      cycleStartTime = currentTime;
      currentTuningState = TUNING_RAMP_UP;
      break;

    case TUNING_RAMP_UP:
      // Analisis selama motor berakselerasi
      if (currentTime - cycleStartTime < TEST_DURATION_MS) {
        float currentRPM = getTargetRPM();
        unsigned long elapsedTime = currentTime - cycleStartTime;
        
        // Deteksi initial burst (motor tiba-tiba kencang di awal)
        // Burst terdeteksi jika dalam 500ms pertama RPM melebihi target >30%
        if (!currentMetrics.burstDetected && elapsedTime < 500) {
          if (currentRPM > TUNING_TARGET_RPM * 1.3) {
            currentMetrics.initialBurst = currentRPM;
            currentMetrics.burstDetected = true;
            currentMetrics.burstTime = elapsedTime;
            Serial.printf("[BURST DETECTED] RPM=%.1f at %lums (%.1f%% over target)\n", 
                         currentRPM, elapsedTime, ((currentRPM - TUNING_TARGET_RPM) / TUNING_TARGET_RPM) * 100.0);
          }
        }
        
        // Hitung Total Error (IAE - Integral of Absolute Error)
        float error = (currentRPM > TUNING_TARGET_RPM) ? (currentRPM - TUNING_TARGET_RPM) : (TUNING_TARGET_RPM - currentRPM);
        currentMetrics.totalError += error;
        currentMetrics.sampleCount++;
        
        // Hitung Overshoot
        if (currentRPM > TUNING_TARGET_RPM) {
          float overshoot = ((currentRPM - TUNING_TARGET_RPM) / TUNING_TARGET_RPM) * 100.0;
          if (overshoot > currentMetrics.maxOvershoot) {
            currentMetrics.maxOvershoot = overshoot;
          }
        }
        
        // Hitung Rise Time (10% -> 90%)
        if (!currentMetrics.hasCrossed10pct && currentRPM >= TUNING_TARGET_RPM * 0.1) {
          currentMetrics.riseTime = currentTime; // Waktu mulai
          currentMetrics.hasCrossed10pct = true;
        }
        if (currentMetrics.hasCrossed10pct && !currentMetrics.hasCrossed90pct && currentRPM >= TUNING_TARGET_RPM * 0.9) {
          currentMetrics.riseTime = currentTime - currentMetrics.riseTime; // Total waktu rise
          currentMetrics.hasCrossed90pct = true;
        }

        // Print real-time data REALTIME tanpa delay (setiap loop iteration)
        static unsigned long lastRealTimeUpdate = 0;
        if (currentTime - lastRealTimeUpdate >= 50) {  // Update setiap 50ms untuk real-time responsif
          Serial.printf("t=%lums, RPM=%.1f, Target=%d, Error=%.1f\n", 
                       currentTime - cycleStartTime, currentRPM, TUNING_TARGET_RPM, error);
          lastRealTimeUpdate = currentTime;
        }

      } else {
        // Waktu tes habis, lanjut ke analisis
        currentTuningState = TUNING_ANALYZING;
      }
      break;

    case TUNING_ANALYZING:
    {
      // Evaluasi hasil tes menggunakan currentMetrics
      float avgError = currentMetrics.getAverageError();
      Serial.printf("Hasil siklus:\n");
      Serial.printf("  - Overshoot: %.2f%%\n", currentMetrics.maxOvershoot);
      Serial.printf("  - Rise Time: %lu ms\n", currentMetrics.riseTime);
      Serial.printf("  - Avg Error: %.2f RPM\n", avgError);
      
      // Tambahkan info burst jika terdeteksi
      if (currentMetrics.burstDetected) {
        Serial.printf("  - Initial Burst: %.1f RPM at %lums (%.1f%% over target)\n",
                     currentMetrics.initialBurst, currentMetrics.burstTime,
                     ((currentMetrics.initialBurst - TUNING_TARGET_RPM) / TUNING_TARGET_RPM) * 100.0);
      }
      
      // Hitung "Skor" dengan penalty untuk burst
      float score = calculatePerformanceScore();
      
      // Tambahkan penalty besar jika ada initial burst
      if (currentMetrics.burstDetected) {
        float burstPenalty = ((currentMetrics.initialBurst - TUNING_TARGET_RPM) / TUNING_TARGET_RPM) * 100.0 * INITIAL_BURST_PENALTY;
        score += burstPenalty;
        Serial.printf("  - Burst Penalty: %.2f\n", burstPenalty);
      }
      
      Serial.printf("Skor: %.2f (Terbaik sejauh ini: %.2f)\n", score, bestScore);

      // Update best parameters jika diperlukan
      bool improved = updateBestParameters(score);
      
      // Track improvement untuk precision stage transition
      if (improved) {
        cyclesWithoutImprovement = 0;
      } else {
        cyclesWithoutImprovement++;
      }
      
      // Hentikan motor untuk cooldown
      setTuningTargetRPM(0);
      cycleStartTime = currentTime;
      currentTuningState = TUNING_COOLDOWN;
    }
      break;

    case TUNING_COOLDOWN:
      // Tunggu motor berhenti sebelum siklus berikutnya
      if (currentTime - cycleStartTime >= COOLDOWN_DURATION_MS) {
        tuningCycleCount++;
        
        // Khusus untuk TUNE_BOTH: check apakah perlu switch motor
        if (currentTuningTarget == TUNE_BOTH) {
          // FIXED: Pembagian cycle yang lebih seimbang
          const int CYCLES_PER_MOTOR = MAX_TUNING_CYCLES / 2;  // 7 cycles per motor
          
          if (isRightMotorPhase && tuningCycleCount >= CYCLES_PER_MOTOR) {
            // Selesai tuning motor kanan (0-6 = 7 cycles), switch ke motor kiri
            rightMotorCompleted = true;
            currentTuningState = TUNING_SWITCH_MOTOR;
          } else if (!isRightMotorPhase && tuningCycleCount >= MAX_TUNING_CYCLES) {
            // Selesai tuning motor kiri (7-14 = 8 cycles total), finish
            leftMotorCompleted = true;
            currentTuningState = TUNING_FINISHED;
          } else {
            // Lanjut tuning motor yang sama
            currentTuningState = TUNING_UPDATING_PARAMS;
          }
        } else {
          // Untuk TUNE_RIGHT dan TUNE_LEFT, logic original
          if (tuningCycleCount >= MAX_TUNING_CYCLES) {
            currentTuningState = TUNING_FINISHED;
          } else {
            currentTuningState = TUNING_UPDATING_PARAMS;
          }
        }
      }
      break;

    case TUNING_SWITCH_MOTOR:
      // State untuk switch dari motor kanan ke motor kiri (hanya untuk TUNE_BOTH)
      Serial.println("\n=== SWITCHING FROM RIGHT MOTOR TO LEFT MOTOR ===");
      Serial.printf("Right motor best: Kp=%.4f, Ki=%.4f, Kd=%.4f (Score: %.2f)\n", 
                   bestKpRight, bestKiRight, bestKdRight, bestScoreRight);
      
      // Switch ke motor kiri
      isRightMotorPhase = false;
      tuningCycleCount = MAX_TUNING_CYCLES/2; // Reset cycle count untuk motor kiri
      
      // Set starting parameters untuk motor kiri dengan RPM-optimized values
      currentKp = pidConfigLeft.kp;
      currentKi = pidConfigLeft.ki;
      currentKd = pidConfigLeft.kd;
      
      // Apply Kp-First RPM constraints - BOOSTED untuk motor response
      if (currentKp < 1.0 || currentKp > 50.0) {
        currentKp = 8.0; // BOOST: Kp lebih tinggi untuk motor response
        Serial.printf("WARNING: Left motor Kp BOOSTED (was %.3f), starting with %.1f for stronger response\n", pidConfigLeft.kp, currentKp);
      }
      if (currentKi < 0.5 || currentKi > 50.0) {
        currentKi = 2.0; // Ki supportive untuk Kp dominance
        Serial.printf("WARNING: Left motor Ki set (was %.3f), starting with %.1f supportive to Kp\n", pidConfigLeft.ki, currentKi);
      }
      if (currentKd > 50) {
        currentKd = 0.1; // Kd minimal untuk Kp focus
        Serial.printf("WARNING: Left motor Kd set (was %.3f), starting with %.2f minimal for Kp focus\n", pidConfigLeft.kd, currentKd);
      }
      
      Serial.println("Starting Kp-First RPM tuning for LEFT motor...");
      Serial.printf("Starting with: Kp=%.4f (Kp-FIRST PRIORITY), Ki=%.4f, Kd=%.4f\n", currentKp, currentKi, currentKd);
      
      currentTuningState = TUNING_STARTING;
      break;

    case TUNING_UPDATING_PARAMS:
    {
      // Kp-First RPM Tuning Algorithm - Prioritas Kp untuk response time
      Serial.printf("\n[RPM TUNING CYCLE %d] Kp-First Strategy Analysis:\n", tuningCycleCount);
      Serial.printf("  Overshoot=%.2f%%, RiseTime=%lums, AvgError=%.2f RPM\n", 
                   currentMetrics.maxOvershoot, currentMetrics.riseTime, currentMetrics.getAverageError());
      
      if (currentMetrics.burstDetected) {
        Serial.printf("  InitialBurst=%.1f RPM at %lums\n", 
                     currentMetrics.initialBurst, currentMetrics.burstTime);
      }
      
      // Check apakah perlu ganti precision stage
      if (checkPrecisionStageTransition()) {
        updatePrecisionStage();
        Serial.printf(">>> SWITCHING TO %s PRECISION STAGE <<<\n", getPrecisionStageName());
      }
      
      // Kp-First Intelligent Parameter Adjustment
      adjustParametersIntelligently();
      
      // Constrain values dengan range yang wajar untuk RPM control
      constrainPIDValues(currentKp, currentKi, currentKd, 0.1, 50.0);
      
      Serial.printf(">>> Kp-FIRST PID: Kp=%.4f, Ki=%.4f, Kd=%.4f\n", currentKp, currentKi, currentKd);
      Serial.printf("Precision: %s, Cycles: %d\n\n", getPrecisionStageName(), tuningCycleCount);
      
      currentTuningState = TUNING_STARTING;
      break;
    }

    case TUNING_FINISHED:
      Serial.println("\n======================================");
      Serial.println("AUTO-TUNING SELESAI!");
      Serial.printf("Parameter terbaik: Kp=%.4f, Ki=%.4f, Kd=%.4f\n", bestKp, bestKi, bestKd);
      Serial.printf("Skor terbaik: %.2f\n", bestScore);
      Serial.println("Menyimpan ke Preferences...");
      
      // Simpan hasil terbaik ke tempat yang sesuai
      switch (currentTuningTarget) {
        case TUNE_RIGHT:
          pidConfigRight.kp = bestKp;
          pidConfigRight.ki = bestKi;
          pidConfigRight.kd = bestKd;
          savePIDParametersRight();
          Serial.println("Hasil tuning disimpan untuk motor kanan");
          Serial1.println("AUTOTUNE_RIGHT:COMPLETED:" + String(bestKp, 3) + "," + String(bestKi, 3) + "," + String(bestKd, 3));
          break;
        case TUNE_LEFT:
          pidConfigLeft.kp = bestKp;
          pidConfigLeft.ki = bestKi;
          pidConfigLeft.kd = bestKd;
          savePIDParametersLeft();
          Serial.println("Hasil tuning disimpan untuk motor kiri");
          Serial1.println("AUTOTUNE_LEFT:COMPLETED:" + String(bestKp, 3) + "," + String(bestKi, 3) + "," + String(bestKd, 3));
          break;
        case TUNE_BOTH:
        default:
          // Untuk TUNE_BOTH, gunakan parameter terbaik masing-masing motor (BERBEDA!)
          pidConfigLeft.kp = bestKpLeft;
          pidConfigLeft.ki = bestKiLeft;
          pidConfigLeft.kd = bestKdLeft;
          pidConfigRight.kp = bestKpRight;
          pidConfigRight.ki = bestKiRight;
          pidConfigRight.kd = bestKdRight;
          
          // Simpan ke preferences untuk kedua motor
          savePIDParametersLeft();
          savePIDParametersRight();
          
          Serial.println("=== HASIL TUNING DUAL MOTOR (BERBEDA) ===");
          Serial.printf("Motor KANAN: Kp=%.4f, Ki=%.4f, Kd=%.4f (Score: %.2f)\n", 
                       bestKpRight, bestKiRight, bestKdRight, bestScoreRight);
          Serial.printf("Motor KIRI:  Kp=%.4f, Ki=%.4f, Kd=%.4f (Score: %.2f)\n", 
                       bestKpLeft, bestKiLeft, bestKdLeft, bestScoreLeft);
          Serial.println("Hasil tuning disimpan untuk kedua motor dengan parameter berbeda");
          
          // Send separate responses for each motor
          Serial1.println("AUTOTUNE_RIGHT:COMPLETED:" + String(bestKpRight, 3) + "," + String(bestKiRight, 3) + "," + String(bestKdRight, 3));
          Serial1.println("AUTOTUNE_LEFT:COMPLETED:" + String(bestKpLeft, 3) + "," + String(bestKiLeft, 3) + "," + String(bestKdLeft, 3));
          Serial1.println("AUTOTUNE_BOTH:COMPLETED:DUAL_MOTOR_DIFFERENT_PARAMS");
          
          // Untuk TUNE_BOTH, jangan panggil setTuningPID karena akan menimpa nilai yang berbeda
          setTuningTargetRPM(0); // Matikan motor
          Serial.println("Tuning berhasil! Parameter optimal telah disimpan.");
          Serial.println("======================================");
          currentTuningState = TUNING_IDLE; // Selesai
          return; // Keluar dari function tanpa mengeksekusi kode setelah switch
      }
      
      // Terapkan PID terbaik (hanya untuk TUNE_RIGHT dan TUNE_LEFT)
      setTuningPID(bestKp, bestKi, bestKd);
      setTuningTargetRPM(0); // Matikan motor
      
      Serial.println("Tuning berhasil! Parameter optimal telah disimpan.");
      Serial.println("======================================");
      
      // Note: rpmControlActive tetap false setelah tuning
      // User harus mengirim command RPM baru untuk mengaktifkan kembali
      Serial.println("[AUTO-TUNER] RPM control remains disabled. Send new RPM command to activate.");
      
      currentTuningState = TUNING_IDLE; // Selesai
      break;
  }
}

// Fungsi untuk memulai proses tuning dari luar (misal via Serial)
void startAutoTuning() {
  startAutoTuningGeneric(TUNE_BOTH);
}

// Fungsi untuk memulai proses tuning motor kanan saja
void startAutoTuningRight() {
  startAutoTuningGeneric(TUNE_RIGHT);
}

// Fungsi untuk memulai proses tuning motor kiri saja
void startAutoTuningLeft() {
  startAutoTuningGeneric(TUNE_LEFT);
}

// Fungsi generic untuk memulai tuning dengan target spesifik
void startAutoTuningGeneric(TuningTarget target) {
  if (currentTuningState != TUNING_IDLE) {
    Serial.println("Error: Tuning sedang berjalan.");
    return;
  }
  
  // CRITICAL FIX: Disable persistent RPM control untuk auto tuner
  // Auto tuner harus mengontrol motor secara eksklusif
  rpmControlActive = false;
  targetRpmKanan = 0.0;
  targetRpmKiri = 0.0;
  Serial.println("[AUTO-TUNER] Persistent RPM control disabled for tuning");
  
  currentTuningTarget = target;
  
  String targetName;
  switch (target) {
    case TUNE_RIGHT:
      targetName = "Motor Kanan";
      break;
    case TUNE_LEFT:
      targetName = "Motor Kiri";
      break;
    case TUNE_BOTH:
    default:
      targetName = "Kedua Motor";
      break;
  }
  
  Serial.println("======================================");
  Serial.println("MEMULAI AUTO-TUNING PID RPM - " + targetName);
  Serial.printf("Target RPM: %d\n", TUNING_TARGET_RPM);
  Serial.printf("Durasi per tes: %lu detik\n", TEST_DURATION_MS / 1000);
  Serial.printf("Maksimum siklus: %d\n", MAX_TUNING_CYCLES);
  Serial.printf("Estimasi total waktu: %.1f menit\n", 
                (MAX_TUNING_CYCLES * (TEST_DURATION_MS + COOLDOWN_DURATION_MS)) / 60000.0);
  Serial.println("======================================");
  
  // Mulai dengan nilai PID saat ini sesuai target
  loadPIDParameters(); // Muat dari preferences
  
  Serial.println("=== STARTING FROM SAVED PID VALUES ===");
  Serial.printf("Right Motor Saved: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", pidConfigRight.kp, pidConfigRight.ki, pidConfigRight.kd);
  Serial.printf("Left Motor Saved:  Kp=%.3f, Ki=%.3f, Kd=%.3f\n", pidConfigLeft.kp, pidConfigLeft.ki, pidConfigLeft.kd);
  
  switch (currentTuningTarget) {
    case TUNE_RIGHT:
      // START dari PID tersimpan sebagai baseline
      currentKp = pidConfigRight.kp;
      currentKi = pidConfigRight.ki;
      currentKd = pidConfigRight.kd;
      Serial.printf("TUNE_RIGHT: Starting from saved PID: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", currentKp, currentKi, currentKd);
        
      break;
    case TUNE_LEFT:
      // START dari PID tersimpan sebagai baseline
      currentKp = pidConfigLeft.kp;
      currentKi = pidConfigLeft.ki;
      currentKd = pidConfigLeft.kd;
      Serial.printf("TUNE_LEFT: Starting from saved PID: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", currentKp, currentKi, currentKd);
      
      // Hanya adjust jika nilai benar-benar tidak wajar untuk RPM control
      if (currentKp <= 0.0 || currentKp > 50.0) {
        currentKp = 10.0; // Kp fallback untuk response yang baik
        Serial.printf("INFO: Left motor Kp fallback (was %.3f), using %.1f as starting point\n", pidConfigLeft.kp, currentKp);
      }
      if (currentKi <= 0.0 || currentKi > 30.0) {
        currentKi = 3.0; // Ki fallback supportive untuk Kp
        Serial.printf("INFO: Left motor Ki fallback (was %.3f), using %.1f as starting point\n", pidConfigLeft.ki, currentKi);
      }
      if (currentKd < 0.0 || currentKd > 5.0) {
        currentKd = 0.1; // Fallback untuk Kd
        Serial.printf("INFO: Left motor Kd fallback (was %.3f), using %.2f as starting point\n", pidConfigLeft.kd, currentKd);
      }
      
      Serial.printf("FINAL LEFT starting values: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", currentKp, currentKi, currentKd);
      break;
    case TUNE_BOTH:
    default:
      // Untuk TUNE_BOTH, mulai dengan motor kanan dulu
      isRightMotorPhase = true;
      rightMotorCompleted = false;
      leftMotorCompleted = false;
      
      // Inisialisasi best scores untuk dual motor
      bestScoreRight = INITIAL_BEST_SCORE;
      bestScoreLeft = INITIAL_BEST_SCORE;
      
      // Starting point dari PID tersimpan motor kanan
      currentKp = pidConfigRight.kp;
      currentKi = pidConfigRight.ki;
      currentKd = pidConfigRight.kd;
      Serial.printf("TUNE_BOTH: Starting RIGHT phase from saved PID: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", currentKp, currentKi, currentKd);
      
      // Hanya adjust jika nilai benar-benar tidak wajar untuk RPM control
      if (currentKp <= 0.0 || currentKp > 50.0) {
        currentKp = 10.0; // Kp fallback untuk response yang baik
        Serial.printf("INFO: TUNE_BOTH RIGHT Kp fallback (was %.3f), using %.1f as starting point\n", pidConfigRight.kp, currentKp);
      }
      if (currentKi <= 0.0 || currentKi > 30.0) {
        currentKi = 3.0; // Ki fallback supportive untuk Kp
        Serial.printf("INFO: TUNE_BOTH RIGHT Ki fallback (was %.3f), using %.1f as starting point\n", pidConfigRight.ki, currentKi);
      }
      if (currentKd < 0.0 || currentKd > 5.0) {
        currentKd = 0.1; // Fallback untuk Kd
        Serial.printf("INFO: TUNE_BOTH RIGHT Kd fallback (was %.3f), using %.2f as starting point\n", pidConfigRight.kd, currentKd);
      }
      
      Serial.printf("FINAL TUNE_BOTH RIGHT starting values: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", currentKp, currentKi, currentKd);
      Serial.println("TUNE_BOTH: Starting with RIGHT motor first (RPM Kp-First Strategy), then LEFT motor");
      break;
  }
  
  // Inisialisasi nilai terbaik
  bestKp = currentKp;
  bestKi = currentKi;
  bestKd = currentKd;
  bestScore = 999999.0;
  tuningCycleCount = 0;
  
  Serial.printf("Memulai dengan: Kp=%.4f, Ki=%.4f, Kd=%.4f\n", currentKp, currentKi, currentKd);
  
  currentTuningState = TUNING_STARTING;
}

// Fungsi untuk membatalkan tuning
void cancelAutoTuning() {
  if (currentTuningState != TUNING_IDLE) {
    Serial.println("Membatalkan auto-tuning...");
    setTuningTargetRPM(0); // Matikan motor
    currentTuningState = TUNING_IDLE;
    
    // Kembalikan ke parameter sebelumnya
    loadPIDParameters();
    Serial.println("Tuning dibatalkan. Parameter dikembalikan ke nilai sebelumnya.");
    
    // Note: rpmControlActive tetap false, user harus kirim RPM command
    Serial.println("[AUTO-TUNER] RPM control remains disabled after cancel.");
  } else {
    Serial.println("Tidak ada tuning yang sedang berjalan.");
  }
}

// Fungsi untuk cek status tuning
bool isTuningActive() {
  return (currentTuningState != TUNING_IDLE);
}

// Fungsi untuk mendapatkan progress tuning dalam persen
int getTuningProgress() {
  if (currentTuningState == TUNING_IDLE) return 0;
  if (currentTuningState == TUNING_FINISHED) return 100;
  return (tuningCycleCount * 100) / MAX_TUNING_CYCLES;
}

// =============================================
// PRECISION TUNING FUNCTIONS - ADVANCED
// =============================================

// Check apakah saatnya pindah ke stage precision berikutnya - Kp-FIRST TRANSITION
bool checkPrecisionStageTransition() {
  precisionStageCount++;
  
  // 2 cycles tanpa improvement untuk transition cepat
  if (cyclesWithoutImprovement >= 2) {
    return true;
  }
  
  // Kp-First: threshold untuk cepat switch ke fine-tuning
  if (bestScore < EXCELLENT_SCORE_THRESHOLD && currentPrecision == PRECISION_COARSE) {
    return true;
  }
  
  // 5 cycles per stage untuk 15 total cycles
  if (precisionStageCount >= 5) {
    return true;
  }
  
  return false;
}

// Update ke precision stage berikutnya
void updatePrecisionStage() {
  if (currentPrecision == PRECISION_COARSE) {
    currentPrecision = PRECISION_FINE;
    currentKpStep = KP_FINE_STEP;
    currentKiStep = KI_FINE_STEP;
    currentKdStep = KD_FINE_STEP;
    // Switching to FINE tuning stage
    
  } else if (currentPrecision == PRECISION_FINE) {
    currentPrecision = PRECISION_ULTRA_FINE;
    currentKpStep = KP_ULTRA_FINE_STEP;
    currentKiStep = KI_ULTRA_FINE_STEP;
    currentKdStep = KD_ULTRA_FINE_STEP;
    // Switching to ULTRA-FINE tuning stage
  }
  
  precisionStageCount = 0;
  cyclesWithoutImprovement = 0;
  bestScoreInStage = bestScore;
}

// Intelligent parameter adjustment berdasarkan precision level
void adjustParametersIntelligently() {
  float overshoot = currentMetrics.maxOvershoot;
  float avgError = currentMetrics.totalError / (currentMetrics.sampleCount > 1 ? currentMetrics.sampleCount : 1);
  unsigned long riseTime = currentMetrics.riseTime;
  
  // Strategy berdasarkan precision stage
  if (currentPrecision == PRECISION_COARSE) {
    // Coarse tuning: fokus pada perubahan besar untuk menemukan range optimal
    adjustParametersCoarse(overshoot, avgError, riseTime);
    
  } else if (currentPrecision == PRECISION_FINE) {
    // Fine tuning: perbaiki dengan step menengah
    adjustParametersFine(overshoot, avgError, riseTime);
    
  } else {
    // Ultra-fine tuning: optimasi akhir dengan step kecil
    adjustParametersUltraFine(overshoot, avgError, riseTime);
  }
}

// Coarse adjustment untuk pencarian range optimal - Kp Priority
void adjustParametersCoarse(float overshoot, float avgError, unsigned long riseTime) {
  if (overshoot > HIGH_OVERSHOOT_THRESHOLD) {
    currentKp -= currentKpStep * 1.5;  // Kurangi Kp untuk control overshoot
    currentKd += currentKdStep * 2.0;  // Tambah damping
    Serial.println(">>> Kp-FIRST: Reducing Kp due to high overshoot, increasing Kd");
    
  } else if (avgError > 8.0 && riseTime > SLOW_RISE_TIME_MS) {
    currentKp += currentKpStep * 2.0;  // BOOST Kp untuk response yang lebih cepat
    currentKi += currentKiStep * 0.8;  // Tambah Ki supportive
    Serial.println(">>> Kp-FIRST: Boosting Kp for faster response, mild Ki increase");
    
  } else if (riseTime < 1000 && overshoot < LOW_OVERSHOOT_THRESHOLD) {
    currentKp += currentKpStep * 0.8;  // Optimasi Kp untuk response
    currentKi += currentKiStep * 0.5;  // Ki supportive
    Serial.println(">>> Kp-FIRST: Optimizing Kp for good response");
    
  } else {
    // Exploratory adjustment dengan Kp priority
    if (tuningCycleCount % 3 == 0) {
      currentKp += currentKpStep * 1.0;  // Kp gets priority
      Serial.println(">>> Kp-FIRST: Exploratory Kp increase");
    } else if (tuningCycleCount % 3 == 1) {
      currentKi += currentKiStep * 0.4;  // Ki supportive
      Serial.println(">>> Kp-FIRST: Exploratory Ki increase");
    } else {
      currentKd += currentKdStep * 0.3;  // Kd minimal
      Serial.println(">>> Kp-FIRST: Exploratory Kd increase");
    }
  }
}

// Fine adjustment untuk optimasi menengah - Kp Priority
void adjustParametersFine(float overshoot, float avgError, unsigned long riseTime) {
  if (overshoot > MEDIUM_OVERSHOOT_THRESHOLD) {
    currentKp -= currentKpStep * 0.8;  // Fine reduce Kp
    currentKd += currentKdStep * 1.0;  // Increase damping
    Serial.println(">>> Kp-FIRST FINE: Reducing Kp, increasing Kd for overshoot control");
    
  } else if (avgError > 4.0) {
    currentKp += currentKpStep * 1.0;  // Fine increase Kp untuk response
    currentKi += currentKiStep * 0.6;  // Ki supportive
    Serial.println(">>> Kp-FIRST FINE: Increasing Kp for better response");
    
  } else {
    // Fine optimization dengan Kp focus
    if (overshoot < 2.0 && avgError < 2.0) {
      currentKp += currentKpStep * 0.5;  // Fine Kp adjustment
      Serial.println(">>> Kp-FIRST FINE: Fine Kp optimization");
    } else {
      currentKd += currentKdStep * 0.4;  // Fine Kd adjustment
      Serial.println(">>> Kp-FIRST FINE: Fine Kd adjustment");
    }
  }
}

// Ultra-fine adjustment untuk optimasi akhir - Kp Priority
void adjustParametersUltraFine(float overshoot, float avgError, unsigned long riseTime) {
  // Micro-adjustments untuk mencapai optimum dengan Kp focus
  if (overshoot > 2.0) {
    currentKp -= currentKpStep * 0.3;  // Micro Kp reduction
    Serial.println(">>> Kp-FIRST ULTRA-FINE: Micro Kp reduction");
    
  } else if (avgError > 1.5) {
    currentKp += currentKpStep * 0.4;  // Micro Kp increase untuk response
    Serial.println(">>> Kp-FIRST ULTRA-FINE: Micro Kp increase");
    
  } else {
    // Random walk untuk mencari optimum lokal dengan Kp bias
    float randomSeed = (float)((tuningCycleCount * 37) % 100) / 100.0 - 0.5; // -0.5 to +0.5
    if (randomSeed > 0.2 || randomSeed < -0.2) {
      currentKp += currentKpStep * randomSeed * 0.5;  // Kp gets bigger range
      currentKi += currentKiStep * randomSeed * 0.2;  // Ki smaller range  
      currentKd += currentKdStep * randomSeed * 0.1;  // Kd smallest range
      Serial.println(">>> Kp-FIRST ULTRA-FINE: Micro random walk with Kp bias");
    }
  }
}

// Get precision stage name for debug
const char* getPrecisionStageName() {
  switch (currentPrecision) {
    case PRECISION_COARSE: return "COARSE";
    case PRECISION_FINE: return "FINE";
    case PRECISION_ULTRA_FINE: return "ULTRA-FINE";
    default: return "UNKNOWN";
  }
}
