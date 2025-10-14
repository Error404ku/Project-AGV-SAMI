// auto_tuner.ino
// Adaptive PID Auto-Tuning untuk RPM Motor
// Menggunakan metode Analisis Offline Cerdas dengan State Machine

// =============================================
// CONSTANTS & CONFIGURATION
// =============================================

// Tuning Parameters
const int TUNING_TARGET_RPM = 40;              // Target RPM untuk pengujian
const unsigned long TEST_DURATION_MS = 5000;   // Durasi satu siklus tes (5 detik)
const unsigned long COOLDOWN_DURATION_MS = 2000; // Waktu motor berhenti antar tes
const int MAX_TUNING_CYCLES = 30;              // Maksimum iterasi penyempurnaan

// Analysis Thresholds
const float RISE_TIME_START_PERCENT = 0.1;     // 10% untuk start rise time
const float RISE_TIME_END_PERCENT = 0.9;       // 90% untuk end rise time
const float HIGH_OVERSHOOT_THRESHOLD = 10.0;   // Overshoot tinggi
const float MEDIUM_OVERSHOOT_THRESHOLD = 5.0;  // Overshoot sedang
const float LOW_OVERSHOOT_THRESHOLD = 2.0;     // Overshoot rendah
const unsigned long SLOW_RISE_TIME_MS = 2000;  // Rise time dianggap lambat
const unsigned long MEDIUM_RISE_TIME_MS = 1500; // Rise time sedang

// PID Adjustment Factors - Improved Precision
const float KP_COARSE_STEP = 2.5;              // Langkah kasar untuk Kp
const float KP_FINE_STEP = 0.5;                // Langkah halus untuk Kp
const float KP_ULTRA_FINE_STEP = 0.05;          // Langkah ultra halus untuk Kp

const float KI_COARSE_STEP = 0.05;             // Langkah kasar untuk Ki
const float KI_FINE_STEP = 0.01;               // Langkah halus untuk Ki
const float KI_ULTRA_FINE_STEP = 0.002;        // Langkah ultra halus untuk Ki

const float KD_COARSE_STEP = 0.1;              // Langkah kasar untuk Kd
const float KD_FINE_STEP = 0.02;               // Langkah halus untuk Kd
const float KD_ULTRA_FINE_STEP = 0.005;        // Langkah ultra halus untuk Kd

// Precision Stages
enum TuningPrecision {
  PRECISION_COARSE,     // Pencarian awal dengan step besar
  PRECISION_FINE,       // Pencarian menengah dengan step sedang
  PRECISION_ULTRA_FINE  // Pencarian akhir dengan step kecil
};

// Scoring Weights - Improved Balance
const float OVERSHOOT_WEIGHT = 5.0;            // Bobot overshoot dalam scoring
const float RISE_TIME_WEIGHT = 0.02;           // Bobot rise time dalam scoring  
const float ERROR_WEIGHT = 2.5;                // Bobot average error dalam scoring
const float STABILITY_WEIGHT = 1.5;            // Bobot stabilitas dalam scoring

// Performance Thresholds
const float EXCELLENT_SCORE_THRESHOLD = 15.0;  // Score excellent untuk switch precision
const float GOOD_SCORE_THRESHOLD = 25.0;       // Score baik untuk evaluasi

// Basic tuning factors (backward compatibility)
const float KP_AGGRESSIVE_REDUCTION = 0.8;
const float KP_MODERATE_REDUCTION = 0.9;
const float KP_INCREASE_FACTOR = 1.1;
const float KD_DAMPING_INCREASE = 1.2;
const float KI_OPTIMIZATION_FACTOR = 1.05;
const float ACCEPTABLE_SCORE_THRESHOLD = 30.0; // Skor dianggap dapat diterima

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
int precisionStageCount = 25;           // Hitung siklus per stage precision

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
  
  void reset() {
    maxOvershoot = 0;
    riseTime = 0;
    settlingTime = 0;
    hasCrossed10pct = false;
    hasCrossed90pct = false;
    totalError = 0;
    sampleCount = 0;
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

// Helper function untuk menghitung performance score
float calculatePerformanceScore() {
  float avgError = currentMetrics.getAverageError();
  return (currentMetrics.maxOvershoot * OVERSHOOT_WEIGHT) + 
         (currentMetrics.riseTime * RISE_TIME_WEIGHT) + 
         (avgError * ERROR_WEIGHT);
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
  if (score < bestScore) {
    bestScore = score;
    bestKp = currentKp;
    bestKi = currentKi;
    bestKd = currentKd;
    Serial.println(">>> DITEMUKAN PARAMETER TERBAIK BARU! <<<");
    return true;
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
      return (float)(rpm_depan_kanan + rpm_depan_kiri) / 2.0;
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
      rpmMotor(targetRPM, targetRPM);  // Kedua motor
      Serial.printf("[DEBUG] Setting BOTH motors to %.1f RPM\n", targetRPM);
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
      pidConfig.kp = kp;
      pidConfig.ki = ki;
      pidConfig.kd = kd;
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

        // Print real-time data setiap 500ms
        if ((currentTime - cycleStartTime) % 500 < 50) {
          Serial.printf("t=%lums, RPM=%.1f, Target=%d, Error=%.1f\n", 
                       currentTime - cycleStartTime, currentRPM, TUNING_TARGET_RPM, error);
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
      
      // Hitung "Skor" menggunakan helper function
      float score = calculatePerformanceScore();
      
      Serial.printf("Skor: %.2f (Terbaik sejauh ini: %.2f)\n", score, bestScore);

      // Update best parameters jika diperlukan
      updateBestParameters(score);
      
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
        if (tuningCycleCount >= MAX_TUNING_CYCLES) {
          currentTuningState = TUNING_FINISHED;
        } else {
          currentTuningState = TUNING_UPDATING_PARAMS;
        }
      }
      break;

    case TUNING_UPDATING_PARAMS:
      // Algoritma Heuristik untuk Update Parameter PID menggunakan currentMetrics
      Serial.printf("Menganalisis hasil: Overshoot=%.2f%%, RiseTime=%lums\n", 
                   currentMetrics.maxOvershoot, currentMetrics.riseTime);
      
      if (currentMetrics.maxOvershoot > HIGH_OVERSHOOT_THRESHOLD) { 
        // Overshoot terlalu tinggi - sistem terlalu agresif
        currentKp *= KP_AGGRESSIVE_REDUCTION; // Kurangi Kp secara signifikan
        currentKd *= KD_DAMPING_INCREASE; // Naikkan Kd untuk meredam
        Serial.println("Action: Mengurangi Kp, menaikkan Kd (overshoot tinggi)");
        
      } else if (currentMetrics.maxOvershoot > MEDIUM_OVERSHOOT_THRESHOLD) { 
        // Overshoot sedang
        currentKp *= KP_MODERATE_REDUCTION; // Kurangi Kp sedikit
        currentKd *= KI_OPTIMIZATION_FACTOR; // Naikkan Kd sedikit
        Serial.println("Action: Mengurangi Kp sedikit (overshoot sedang)");
        
      } else if (currentMetrics.maxOvershoot < LOW_OVERSHOOT_THRESHOLD && currentMetrics.riseTime > SLOW_RISE_TIME_MS) { 
        // Lambat dan tidak ada overshoot - perlu lebih agresif
        currentKp *= KP_INCREASE_FACTOR; // Naikkan Kp untuk mempercepat
        Serial.println("Action: Menaikkan Kp (respon lambat, no overshoot)");
        
      } else if (currentMetrics.riseTime > MEDIUM_RISE_TIME_MS) { 
        // Respon masih lambat
        currentKp *= 1; // Naikkan Kp sedang
        Serial.println("Action: Menaikkan Kp sedang (respon lambat)");
        
      } else { 
        // Respon sudah cukup bagus, optimalkan Ki untuk steady-state
        currentKi *= KI_OPTIMIZATION_FACTOR; // Naikkan Ki sedikit
        Serial.println("Action: Menaikkan Ki (optimalisasi steady-state)");
      }
      
      // Pastikan nilai tidak menjadi nol atau negatif, dan dalam batas wajar
      constrainPIDValues(currentKp, currentKi, currentKd);

      Serial.printf("Parameter baru: Kp=%.4f, Ki=%.4f, Kd=%.4f\n", currentKp, currentKi, currentKd);
      currentTuningState = TUNING_STARTING;
      break;

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
          pidConfig.kp = bestKp;
          pidConfig.ki = bestKi;
          pidConfig.kd = bestKd;
          savePIDParameters();
          Serial.println("Hasil tuning disimpan untuk mode umum");
          Serial1.println("AUTOTUNE:COMPLETED:" + String(bestKp, 3) + "," + String(bestKi, 3) + "," + String(bestKd, 3));
          break;
      }
      
      // Terapkan PID terbaik
      setTuningPID(bestKp, bestKi, bestKd);
      setTuningTargetRPM(0); // Matikan motor
      
      Serial.println("Tuning berhasil! Parameter optimal telah disimpan.");
      Serial.println("======================================");
      
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
  switch (currentTuningTarget) {
    case TUNE_RIGHT:
      currentKp = pidConfigRight.kp;
      currentKi = pidConfigRight.ki;
      currentKd = pidConfigRight.kd;
      // Fix untuk motor kanan: pastikan Kp minimal 10.0
      if (currentKp < 10.0) {
        currentKp = 20.0; // Start dengan nilai yang lebih realistis
        Serial.printf("WARNING: Right motor Kp too low (%.3f), starting with %.1f\n", pidConfigRight.kp, currentKp);
      }
      break;
    case TUNE_LEFT:
      currentKp = pidConfigLeft.kp;
      currentKi = pidConfigLeft.ki;
      currentKd = pidConfigLeft.kd;
      // Fix untuk motor kiri: pastikan Kp minimal 10.0
      if (currentKp < 10.0) {
        currentKp = 20.0; // Start dengan nilai yang lebih realistis
        Serial.printf("WARNING: Left motor Kp too low (%.3f), starting with %.1f\n", pidConfigLeft.kp, currentKp);
      }
      break;
    case TUNE_BOTH:
    default:
      currentKp = pidConfig.kp;
      currentKi = pidConfig.ki;
      currentKd = pidConfig.kd;
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

// Check apakah saatnya pindah ke stage precision berikutnya
bool checkPrecisionStageTransition() {
  precisionStageCount++;
  
  // Jika sudah 10 cycles tanpa improvement significant, naik ke precision berikutnya
  if (cyclesWithoutImprovement >= 8) {
    return true;
  }
  
  // Jika score sudah sangat baik, langsung ke fine tuning
  if (bestScore < EXCELLENT_SCORE_THRESHOLD && currentPrecision == PRECISION_COARSE) {
    return true;
  }
  
  // Jika sudah 15 cycles di stage ini, pindah ke berikutnya
  if (precisionStageCount >= 15) {
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

// Coarse adjustment untuk pencarian range optimal
void adjustParametersCoarse(float overshoot, float avgError, unsigned long riseTime) {
  if (overshoot > HIGH_OVERSHOOT_THRESHOLD) {
    currentKp -= currentKpStep * 2.0;  // Kurangi Kp agresif
    currentKd += currentKdStep * 1.5;  // Tambah damping
    // Action: Aggressive Kp reduction, increase Kd
    
  } else if (avgError > 10.0 && riseTime > SLOW_RISE_TIME_MS) {
    currentKp += currentKpStep * 1.5;  // Tambah Kp untuk response
    currentKi += currentKiStep * 1.2;  // Tambah Ki untuk steady-state
    // Action: Increase Kp and Ki for better response
    
  } else if (riseTime < 500 && overshoot < LOW_OVERSHOOT_THRESHOLD) {
    currentKi += currentKiStep;        // Optimasi steady-state
    // Action: Optimize Ki for steady-state
    
  } else {
    // Exploratory adjustment
    if (tuningCycleCount % 3 == 0) {
      currentKp += currentKpStep * 0.5;
    } else if (tuningCycleCount % 3 == 1) {
      currentKi += currentKiStep * 0.3;
    } else {
      currentKd += currentKdStep * 0.2;
    }
    // Action: Exploratory parameter adjustment
  }
}

// Fine adjustment untuk optimasi menengah
void adjustParametersFine(float overshoot, float avgError, unsigned long riseTime) {
  if (overshoot > MEDIUM_OVERSHOOT_THRESHOLD) {
    currentKp -= currentKpStep;
    currentKd += currentKdStep * 0.5;
    // Action: Fine reduce Kp, slight increase Kd
    
  } else if (avgError > 5.0) {
    currentKp += currentKpStep * 0.5;
    currentKi += currentKiStep * 0.8;
    // Action: Fine increase Kp and Ki
    
  } else {
    // Fine optimization based on current best parameter
    if (overshoot < 1.0 && avgError < 3.0) {
      currentKi += currentKiStep * 0.3;  // Very fine Ki adjustment
    } else {
      currentKd += currentKdStep * 0.2;  // Very fine Kd adjustment
    }
    // Action: Very fine parameter optimization
  }
}

// Ultra-fine adjustment untuk optimasi akhir
void adjustParametersUltraFine(float overshoot, float avgError, unsigned long riseTime) {
  // Micro-adjustments untuk mencapai optimum
  if (overshoot > 1.0) {
    currentKp -= currentKpStep * 0.5;
    // Action: Micro Kp reduction
    
  } else if (avgError > 2.0) {
    currentKi += currentKiStep * 0.5;
    // Action: Micro Ki increase
    
  } else {
    // Random walk untuk mencari optimum lokal
    float randomSeed = (float)((tuningCycleCount * 37) % 100) / 100.0 - 0.5; // -0.5 to +0.5
    if (randomSeed > 0.3 || randomSeed < -0.3) {
      currentKp += currentKpStep * randomSeed * 0.3;
      currentKi += currentKiStep * randomSeed * 0.2;
      currentKd += currentKdStep * randomSeed * 0.1;
      // Action: Micro random walk optimization
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

// Improved scoring dengan lebih banyak faktor
float calculateImprovedScore(float overshoot, unsigned long riseTime, float avgError) {
  // Base score dari performance metrics
  float score = overshoot * OVERSHOOT_WEIGHT + 
                (float)riseTime * RISE_TIME_WEIGHT + 
                avgError * ERROR_WEIGHT;
  
  // Penalty untuk parameter yang ekstrem
  if (currentKp < 1.0 || currentKp > 100.0) score += 10.0;
  if (currentKi < 0.001 || currentKi > 1.0) score += 5.0;
  if (currentKd < 0.0 || currentKd > 2.0) score += 5.0;
  
  // Bonus untuk parameter yang seimbang
  float balance = (currentKp > 30.0 ? currentKp - 30.0 : 30.0 - currentKp) + 
                 (currentKi > 0.1 ? (currentKi - 0.1) * 100 : (0.1 - currentKi) * 100) + 
                 (currentKd > 0.05 ? (currentKd - 0.05) * 200 : (0.05 - currentKd) * 200);
  score += balance * 0.1;
  
  // Stability bonus jika metrik konsisten
  if (currentMetrics.sampleCount > 50) {
    float stability = (float)currentMetrics.sampleCount / 100.0;
    score -= stability * STABILITY_WEIGHT;
  }
  
  return score;
}
