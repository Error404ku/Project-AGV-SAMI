# Strategi Dual-Motor Tuning untuk TUNE_BOTH

## Konsep Dual-Motor Tuning

### Masalah dengan Strategi Lama:
```cpp
// STRATEGI LAMA (SALAH):
case TUNE_BOTH:
  pidConfigLeft.kp = samePID;   // ❌ Motor kiri dan kanan SAMA
  pidConfigRight.kp = samePID;  // ❌ Padahal karakteristiknya BERBEDA
```

**Problem**: Motor kanan dan kiri memiliki karakteristik fisik yang berbeda:
- **Toleransi manufaktur** berbeda
- **Friction** berbeda  
- **Load balancing** berbeda
- **Encoder precision** bisa berbeda
- **Wiring resistance** berbeda

### Solusi: Independent Tuning
```cpp
// STRATEGI BARU (BENAR):
Motor Kanan: Kp=12.3, Ki=0.08, Kd=0.15  // Optimal untuk motor kanan
Motor Kiri:  Kp=10.7, Ki=0.09, Kd=0.12  // Optimal untuk motor kiri (BERBEDA!)
```

## Alur Kerja Dual-Motor Tuning

### Phase 1: Tuning Motor Kanan (Cycle 1-15)
```
1. Set: isRightMotorPhase = true
2. Target: rpmMotor(40, 0) // Kanan ON, Kiri OFF
3. Measurement: rpm_depan_kanan only
4. Optimization: Kp, Ki, Kd untuk karakteristik motor kanan
5. Best: bestKpRight, bestKiRight, bestKdRight, bestScoreRight
```

### Phase 2: Switch Motor (Cycle 15)
```
1. State: TUNING_SWITCH_MOTOR
2. Log: "SWITCHING FROM RIGHT MOTOR TO LEFT MOTOR"
3. Save: Right motor best parameters
4. Reset: isRightMotorPhase = false
5. Load: Left motor starting parameters
```

### Phase 3: Tuning Motor Kiri (Cycle 16-30)
```
1. Set: isRightMotorPhase = false  
2. Target: rpmMotor(0, 40) // Kanan OFF, Kiri ON
3. Measurement: rpm_depan_kiri only
4. Optimization: Kp, Ki, Kd untuk karakteristik motor kiri
5. Best: bestKpLeft, bestKiLeft, bestKdLeft, bestScoreLeft
```

### Phase 4: Final Save (Different Parameters)
```
pidConfigRight: bestKpRight, bestKiRight, bestKdRight
pidConfigLeft:  bestKpLeft, bestKiLeft, bestKdLeft
savePIDParametersRight() + savePIDParametersLeft()
```

## Struktur Code Changes

### 1. **New State Machine State**
```cpp
enum TuningState {
  // ... existing states ...
  TUNING_SWITCH_MOTOR,     // State untuk switch dari kanan ke kiri
  TUNING_FINISHED
};
```

### 2. **Dual Motor Variables**
```cpp
// Dual Motor Tuning Variables for TUNE_BOTH
bool isRightMotorPhase = true;         // true = kanan, false = kiri
bool rightMotorCompleted = false;      
bool leftMotorCompleted = false;       

// Best parameters untuk masing-masing motor
double bestKpRight, bestKiRight, bestKdRight;
double bestKpLeft, bestKiLeft, bestKdLeft;
float bestScoreRight = INITIAL_BEST_SCORE;
float bestScoreLeft = INITIAL_BEST_SCORE;
```

### 3. **Updated Helper Functions**

#### `setTuningPID()`:
```cpp
case TUNE_BOTH:
  if (isRightMotorPhase) {
    pidConfigRight.kp = kp;  // Update hanya motor kanan
  } else {
    pidConfigLeft.kp = kp;   // Update hanya motor kiri
  }
```

#### `getTargetRPM()`:
```cpp
case TUNE_BOTH:
  if (isRightMotorPhase) {
    return (float)rpm_depan_kanan;  // Read hanya motor kanan
  } else {
    return (float)rpm_depan_kiri;   // Read hanya motor kiri
  }
```

#### `setTuningTargetRPM()`:
```cpp
case TUNE_BOTH:
  if (isRightMotorPhase) {
    rpmMotor(targetRPM, 0);  // Kanan ON, Kiri OFF
  } else {
    rpmMotor(0, targetRPM);  // Kanan OFF, Kiri ON
  }
```

#### `updateBestParameters()`:
```cpp
if (currentTuningTarget == TUNE_BOTH) {
  if (isRightMotorPhase) {
    // Update best untuk motor kanan
    if (score < bestScoreRight) {
      bestScoreRight = score;
      bestKpRight = currentKp;
      // ...
    }
  } else {
    // Update best untuk motor kiri
    if (score < bestScoreLeft) {
      bestScoreLeft = score;
      bestKpLeft = currentKp;
      // ...
    }
  }
}
```

### 4. **TUNING_SWITCH_MOTOR State Handler**
```cpp
case TUNING_SWITCH_MOTOR:
  Serial.println("=== SWITCHING FROM RIGHT MOTOR TO LEFT MOTOR ===");
  Serial.printf("Right motor best: Kp=%.4f, Ki=%.4f, Kd=%.4f (Score: %.2f)\n", 
               bestKpRight, bestKiRight, bestKdRight, bestScoreRight);
  
  // Switch to left motor
  isRightMotorPhase = false;
  tuningCycleCount = MAX_TUNING_CYCLES/2;
  
  // Load left motor starting parameters
  currentKp = pidConfigLeft.kp;
  // Apply constraints...
  
  currentTuningState = TUNING_STARTING;
```

### 5. **Updated TUNING_COOLDOWN Logic**
```cpp
case TUNING_COOLDOWN:
  if (currentTuningTarget == TUNE_BOTH) {
    if (isRightMotorPhase && tuningCycleCount >= MAX_TUNING_CYCLES/2) {
      // Switch to left motor
      currentTuningState = TUNING_SWITCH_MOTOR;
    } else if (!isRightMotorPhase && tuningCycleCount >= MAX_TUNING_CYCLES) {
      // Finish dual tuning
      currentTuningState = TUNING_FINISHED;
    }
    // ...
  }
```

## Expected Results

### Debug Output Contoh:
```
======================================
MEMULAI AUTO-TUNING PID RPM - Kedua Motor
TUNE_BOTH: Starting with RIGHT motor first, then LEFT motor
Memulai dengan: Kp=12.000, Ki=0.050, Kd=0.100

[TUNING CYCLE 8] RIGHT MOTOR
>>> ACTION: Moderate Kp reduction (medium overshoot, no burst)
New parameters: Kp=10.350, Ki=0.067, Kd=0.128

=== SWITCHING FROM RIGHT MOTOR TO LEFT MOTOR ===
Right motor best: Kp=10.350, Ki=0.067, Kd=0.128 (Score: 18.75)
Starting tuning for LEFT motor...
Starting with: Kp=12.000, Ki=0.050, Kd=0.100

[TUNING CYCLE 23] LEFT MOTOR  
>>> ACTION: Light Kp reduction (low overshoot)
New parameters: Kp=11.200, Ki=0.075, Kd=0.110

=== HASIL TUNING DUAL MOTOR (BERBEDA) ===
Motor KANAN: Kp=10.350, Ki=0.067, Kd=0.128 (Score: 18.75)
Motor KIRI:  Kp=11.200, Ki=0.075, Kd=0.110 (Score: 22.40)
Hasil tuning disimpan untuk kedua motor dengan parameter berbeda

AUTOTUNE_RIGHT:COMPLETED:10.350,0.067,0.128
AUTOTUNE_LEFT:COMPLETED:11.200,0.075,0.110
AUTOTUNE_BOTH:COMPLETED:DUAL_MOTOR_DIFFERENT_PARAMS
```

### Final Preferences:
```
pidConfigRight: Kp=10.350, Ki=0.067, Kd=0.128
pidConfigLeft:  Kp=11.200, Ki=0.075, Kd=0.110
```

### Motor Performance:
```
Motor Kanan: rpm 0→40 dengan Kp=10.350 (optimal untuk karakteristiknya)
Motor Kiri:  rpm 0→40 dengan Kp=11.200 (optimal untuk karakteristiknya)
```

## Benefits

### 1. **True Optimization**
- Setiap motor mendapat parameter PID yang **optimal untuk karakteristiknya**
- Tidak ada kompromi "satu ukuran untuk semua"

### 2. **Better Performance**  
- Motor kanan: overshoot minimal dengan Kp yang tepat
- Motor kiri: overshoot minimal dengan Kp yang berbeda (tapi tepat)
- Overall: AGV bergerak lebih stabil dan presisi

### 3. **Independent Tuning**
- Jika motor kanan bermasalah, hanya perlu re-tune kanan
- Parameter motor kiri tetap optimal

### 4. **Realistic Approach**
- Mengakui bahwa motor fisik **TIDAK IDENTIK**
- Sesuai dengan kenyataan manufacturing tolerance

## Testing Procedure

### 1. Test TUNE_BOTH:
```
Command: AUTOTUNEBOTH
Expected: 
- Phase 1: Tune motor kanan (cycle 1-15)
- Phase 2: Switch ke motor kiri (cycle 16-30)  
- Result: Parameter berbeda untuk kanan vs kiri
```

### 2. Verify Different Parameters:
```
After tuning:
pidConfigRight.kp != pidConfigLeft.kp  // Should be DIFFERENT!
pidConfigRight.ki != pidConfigLeft.ki
pidConfigRight.kd != pidConfigLeft.kd
```

### 3. Performance Test:
```
Test 1: rpmMotor(40, 0) // Kanan saja
Test 2: rpmMotor(0, 40) // Kiri saja  
Test 3: rpmMotor(40, 40) // Kedua motor
Expected: Smooth response, minimal overshoot untuk semua test
```

## Summary

✅ **Motor kanan dan kiri di-tune secara terpisah**  
✅ **Parameter PID berbeda sesuai karakteristik masing-masing**  
✅ **Phase-based tuning: kanan dulu, lalu kiri**  
✅ **Independent best parameter tracking**  
✅ **Realistic approach untuk hardware yang tidak identik**  

Strategi ini menghasilkan **true dual-motor optimization** dimana setiap motor mendapat parameter PID yang benar-benar optimal untuk karakteristik fisiknya masing-masing.

---
**Update**: 14 Oktober 2025  
**Author**: GitHub Copilot  
**Status**: Implemented - Dual Motor Strategy Ready for Testing