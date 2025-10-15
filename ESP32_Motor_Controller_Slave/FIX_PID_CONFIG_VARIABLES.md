# Perbaikan Auto-Tuning: Variable PID Config & Preferences Save

## Masalah yang Diperbaiki

### 1. **Inconsistent PID Config Variables**

**Problem**: Code menggunakan `pidConfig` generic yang tidak sesuai dengan sistem yang sebenarnya menggunakan:
- `pidConfigLeft` untuk motor kiri 
- `pidConfigRight` untuk motor kanan

**Evidence**:
```cpp
// Yang digunakan di sistem:
pidConfigLeft.kp = kp; 
pidConfigLeft.ki = ki; 
pidConfigLeft.kd = kd;

pidConfigRight.kp = kp; 
pidConfigRight.ki = ki; 
pidConfigRight.kd = kd;

// Yang salah di auto_tuner.ino:
case TUNE_BOTH:
  pidConfig.kp = kp;  // ❌ Variable ini tidak digunakan!
  pidConfig.ki = ki;
  pidConfig.kd = kd;
```

### 2. **Preferences Save Tidak Lengkap**

**Problem**: Hasil tuning tidak tersimpan ke preferences dengan benar, terutama untuk mode `TUNE_BOTH`.

## Solusi yang Diterapkan

### 1. **Fixed setTuningPID() untuk TUNE_BOTH**

```cpp
// SEBELUM (SALAH):
case TUNE_BOTH:
default:
  pidConfig.kp = kp;    // ❌ Variable tidak digunakan
  pidConfig.ki = ki;
  pidConfig.kd = kd;
  break;

// SEKARANG (BENAR):
case TUNE_BOTH:
default:
  // Update kedua motor karena sistem menggunakan pidConfigLeft dan pidConfigRight
  pidConfigLeft.kp = kp;   // ✅ Update motor kiri
  pidConfigLeft.ki = ki;
  pidConfigLeft.kd = kd;
  pidConfigRight.kp = kp;  // ✅ Update motor kanan
  pidConfigRight.ki = ki;
  pidConfigRight.kd = kd;
  break;
```

**Impact**: Sekarang `TUNE_BOTH` akan benar-benar mengupdate kedua motor dengan parameter yang sama.

### 2. **Fixed TUNING_FINISHED Save to Preferences**

```cpp
// SEBELUM (TIDAK LENGKAP):
case TUNE_BOTH:
default:
  pidConfig.kp = bestKp;        // ❌ Variable tidak digunakan
  pidConfig.ki = bestKi;
  pidConfig.kd = bestKd;
  savePIDParameters();          // ❌ Function tidak sesuai
  break;

// SEKARANG (LENGKAP):
case TUNE_BOTH:
default:
  // Update dan simpan ke kedua motor
  pidConfigLeft.kp = bestKp;    // ✅ Update motor kiri
  pidConfigLeft.ki = bestKi;
  pidConfigLeft.kd = bestKd;
  pidConfigRight.kp = bestKp;   // ✅ Update motor kanan
  pidConfigRight.ki = bestKi;
  pidConfigRight.kd = bestKd;
  
  // Simpan ke preferences untuk kedua motor
  savePIDParametersLeft();      // ✅ Save motor kiri
  savePIDParametersRight();     // ✅ Save motor kanan
  
  Serial.println("Hasil tuning disimpan untuk kedua motor (Left & Right)");
  Serial1.println("AUTOTUNE_BOTH:COMPLETED:" + String(bestKp, 3) + "," + String(bestKi, 3) + "," + String(bestKd, 3));
  break;
```

**Impact**: Hasil tuning akan tersimpan dengan benar untuk kedua motor.

### 3. **Fixed startAutoTuningGeneric() untuk TUNE_BOTH**

```cpp
// SEBELUM (SALAH):
case TUNE_BOTH:
default:
  currentKp = pidConfig.kp;    // ❌ Variable tidak digunakan
  currentKi = pidConfig.ki;
  currentKd = pidConfig.kd;
  break;

// SEKARANG (BENAR):
case TUNE_BOTH:
default:
  // Untuk TUNE_BOTH, gunakan motor kanan sebagai referensi starting point
  currentKp = pidConfigRight.kp;  // ✅ Gunakan variable yang benar
  currentKi = pidConfigRight.ki;
  currentKd = pidConfigRight.kd;
  
  // Apply same Kp constraint untuk TUNE_BOTH
  if (currentKp < 5.0 || currentKp > 25.0) {
    currentKp = 12.0; // Start dengan nilai yang lebih konservatif
    Serial.printf("WARNING: TUNE_BOTH Kp adjusted (was %.3f), starting with %.1f to prevent overshoot\n", pidConfigRight.kp, currentKp);
  }
  break;
```

**Impact**: Starting values akan diambil dari variable yang benar.

## Alur Kerja yang Diperbaiki

### Sebelum Perbaikan:
```
TUNE_BOTH:
1. ❌ Read from pidConfig (tidak digunakan)
2. ❌ Update pidConfig selama tuning
3. ❌ Save ke pidConfig + savePIDParameters()
4. ❌ Motor tidak mendapat update parameter yang benar
```

### Setelah Perbaikan:
```
TUNE_BOTH:
1. ✅ Read from pidConfigRight sebagai starting point
2. ✅ Update pidConfigLeft dan pidConfigRight selama tuning
3. ✅ Save ke pidConfigLeft + pidConfigRight via savePIDParametersLeft() & savePIDParametersRight()
4. ✅ Kedua motor mendapat parameter tuning yang sama dan tersimpan
```

## Testing Verification

### Test 1: TUNE_BOTH Command
```
Command: AUTOTUNERIGHT atau AUTOTUNELEFT
Expected: 
- Starting values diambil dari pidConfigRight
- Tuning process normal
- Final values disimpan ke pidConfigLeft dan pidConfigRight
- Serial response: "AUTOTUNE_BOTH:COMPLETED:Kp,Ki,Kd"
```

### Test 2: Preferences Persistence
```
1. Run TUNE_BOTH
2. Restart ESP32
3. Check pidConfigLeft.kp dan pidConfigRight.kp
Expected: Both have same tuned values
```

### Test 3: Motor Response
```
1. After TUNE_BOTH tuning
2. Test motor kiri: rpmMotor(0, 40)
3. Test motor kanan: rpmMotor(40, 0)
Expected: Both motors use tuned PID parameters
```

## Debug Output Changes

### New Serial Messages:
```
WARNING: TUNE_BOTH Kp adjusted (was 20.000), starting with 12.0 to prevent overshoot
Hasil tuning disimpan untuk kedua motor (Left & Right)
AUTOTUNE_BOTH:COMPLETED:12.345,0.067,0.123
```

### Log Verification:
```
[DEBUG] Setting BOTH motors to 40.0 RPM
[DEBUG] PWM Output - Kanan: 128 (was: 0), Kiri: 128 (was: 0)
>>> ACTION: Moderate Kp reduction (medium overshoot, no burst)
New parameters: Kp=10.350, Ki=0.067, Kd=0.128
```

## Variable Mapping Summary

| Mode | Read From | Write To During Tuning | Save To Preferences |
|------|-----------|------------------------|-------------------|
| **TUNE_RIGHT** | pidConfigRight | pidConfigRight | savePIDParametersRight() |
| **TUNE_LEFT** | pidConfigLeft | pidConfigLeft | savePIDParametersLeft() |
| **TUNE_BOTH** | pidConfigRight | pidConfigLeft + pidConfigRight | savePIDParametersLeft() + savePIDParametersRight() |

## Expected Behavior

### TUNE_BOTH Results:
- **Motor Kiri**: Uses tuned Kp, Ki, Kd
- **Motor Kanan**: Uses same tuned Kp, Ki, Kd  
- **Preferences**: Both motor configs saved with identical values
- **Performance**: Synchronized behavior between both motors

### Persistence:
- Values survive ESP32 restart
- Manual PID menu shows tuned values for both motors
- Motor Test menu uses tuned parameters

---

## Summary

✅ **Fixed inconsistent variable usage** - Now using pidConfigLeft/Right instead of generic pidConfig
✅ **Fixed preferences save** - Both motors get saved properly for TUNE_BOTH
✅ **Fixed starting values** - TUNE_BOTH reads from correct variables
✅ **Improved logging** - Clear indication when both motors are updated
✅ **Maintained anti-overshoot strategy** - Kp constraints applied to TUNE_BOTH

Auto-tuning untuk mode `TUNE_BOTH` sekarang akan bekerja dengan benar dan hasil tuning akan tersimpan permanen untuk kedua motor.

---
**Update**: 14 Oktober 2025  
**Author**: GitHub Copilot  
**Status**: Fixed - Ready for Testing