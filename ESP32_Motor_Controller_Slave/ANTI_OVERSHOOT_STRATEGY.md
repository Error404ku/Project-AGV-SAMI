# Perbaikan Auto-Tuning: Anti-Overshoot Strategy

## Masalah yang Diatasi

### Gejala:

- **Nilai Kp terlalu besar** menyebabkan overshoot tinggi
- Motor RPM melebihi target secara berlebihan
- Response terlalu agresif dan tidak stabil
- Oscillation di sekitar setpoint

### Root Cause:

1. **Starting Kp terlalu tinggi** (20.0) → immediate overshoot
2. **Kp step size terlalu besar** → adjustment tidak halus
3. **Overshoot threshold terlalu tinggi** → tidak sensitif terhadap overshoot kecil
4. **Scoring weight overshoot terlalu rendah** → algorithm tidak cukup "takut" terhadap overshoot

## Perubahan Anti-Overshoot Strategy

### 1. **Kp Step Size Dikurangi** (Lebih Konservatif)

```cpp
// SEBELUM (Ki Priority):
const float KP_COARSE_STEP = 1.0;
const float KP_FINE_STEP = 0.2;
const float KP_ULTRA_FINE_STEP = 0.02;

// SEKARANG (Anti-Overshoot):
const float KP_COARSE_STEP = 0.8;      // Dikurangi 20%
const float KP_FINE_STEP = 0.15;       // Dikurangi 25%
const float KP_ULTRA_FINE_STEP = 0.015; // Dikurangi 25%
```

**Dampak**: Adjustment Kp lebih halus, mengurangi kemungkinan "melompat" ke nilai yang menyebabkan overshoot.

### 2. **Overshoot Scoring Weight Diperbesar** (Lebih Sensitif)

```cpp
// SEBELUM:
const float OVERSHOOT_WEIGHT = 6.0;

// SEKARANG:
const float OVERSHOOT_WEIGHT = 8.0;     // +33% lebih sensitif
const float INITIAL_BURST_PENALTY = 5.0; // +25% penalty
```

**Dampak**: Algorithm akan "sangat menghindari" overshoot, membuat parameter yang menghasilkan overshoot mendapat score buruk.

### 3. **Overshoot Threshold Lebih Ketat**

```cpp
// SEBELUM:
const float HIGH_OVERSHOOT_THRESHOLD = 10.0;   // 10%
const float MEDIUM_OVERSHOOT_THRESHOLD = 5.0;  // 5%
const float LOW_OVERSHOOT_THRESHOLD = 2.0;     // 2%

// SEKARANG:
const float HIGH_OVERSHOOT_THRESHOLD = 5.0;    // 5% (dikurangi 50%)
const float MEDIUM_OVERSHOOT_THRESHOLD = 2.5;  // 2.5% (dikurangi 50%)
const float LOW_OVERSHOOT_THRESHOLD = 1.0;     // 1% (dikurangi 50%)
```

**Dampak**: Algorithm akan bereaksi terhadap overshoot yang lebih kecil, tidak menunggu hingga overshoot besar.

### 4. **Starting Kp Value Lebih Konservatif**

```cpp
// SEBELUM:
if (currentKp < 10.0) {
  currentKp = 20.0; // Terlalu tinggi!
}

// SEKARANG:
if (currentKp < 5.0 || currentKp > 25.0) {
  currentKp = 12.0; // Lebih konservatif, turun 40%
}
```

**Dampak**: Starting point yang lebih aman, mengurangi kemungkinan overshoot dari awal tuning.

### 5. **Multiple Overshoot Level Handling** (Baru!)

```cpp
// Strategy lama: hanya 1 level overshoot handling
if (overshoot > HIGH_OVERSHOOT_THRESHOLD) {
  currentKp -= currentKpStep * 1.5;
}

// Strategy baru: 3 level graduated response
if (overshoot > HIGH_OVERSHOOT_THRESHOLD) {          // >5%
  currentKp -= currentKpStep * 2.5;  // Aggressive reduction
} else if (overshoot > MEDIUM_OVERSHOOT_THRESHOLD) { // >2.5%
  currentKp -= currentKpStep * 1.8;  // Moderate reduction
} else if (overshoot > LOW_OVERSHOOT_THRESHOLD) {    // >1%
  currentKp -= currentKpStep * 1.0;  // Light reduction
}
```

**Dampak**: Response yang **graduated** terhadap tingkat overshoot, tidak hanya "all or nothing".

## Perbandingan Scoring

### Contoh Case: Overshoot 3%

**Scoring Lama (Ki Priority)**:

```
Overshoot: 3% × 6.0 = 18.0 points
Rise time: 1200ms × 0.015 = 18.0 points
Avg error: 2.5 RPM × 3.5 = 8.75 points
Total: 44.75 points
Action: Tidak ada (threshold HIGH = 10%)
```

**Scoring Baru (Anti-Overshoot)**:

```
Overshoot: 3% × 8.0 = 24.0 points (+33%)
Rise time: 1200ms × 0.01 = 12.0 points (-33%)
Avg error: 2.5 RPM × 3.0 = 7.5 points
Total: 43.5 points
Action: MEDIUM overshoot reduction (threshold = 2.5%)
```

**Hasil**: Algorithm sekarang akan **mengurangi Kp** pada overshoot 3%, sedangkan sebelumnya diabaikan.

## Progression Strategy

### Stage 1: Coarse Tuning (Anti-Overshoot)

- Kp step: 0.8 (dari 1.0)
- Priority: Cari Kp yang tidak overshoot
- Target: Overshoot <5%

### Stage 2: Fine Tuning

- Kp step: 0.15 (dari 0.2)
- Priority: Optimize response time dengan overshoot <2.5%
- Target: Balance speed vs stability

### Stage 3: Ultra-Fine Tuning

- Kp step: 0.015 (dari 0.02)
- Priority: Precision tuning dengan overshoot <1%
- Target: Optimal performance

## Expected Results

### Target Performance:

1. **Overshoot**: ≤1% (dari sebelumnya bisa >10%)
2. **Rise Time**: Slightly slower tapi controlled (acceptable trade-off)
3. **Steady-State Error**: Tetap minimal dengan Ki optimization
4. **Stability**: Significant improvement, no oscillation

### Typical Final Kp Range:

- **Sebelum**: 15-30 (sering overshoot)
- **Sekarang**: 8-15 (controlled, stable)

### Debug Output Changes:

```
[TUNING CYCLE 8] Menganalisis hasil:
  Overshoot=3.20%, RiseTime=1450ms, AvgError=2.10 RPM

>>> ACTION: Moderate Kp reduction (medium overshoot, no burst)
New parameters: Kp=10.350, Ki=0.067, Kd=0.128
```

## Testing Procedure

### Pre-Test:

```
Motor Test → RPM Test
Set target: 40 RPM
Observe: Apakah ada overshoot >1%?
```

### Auto-Tuning Test:

```
1. Run: AUTOTUNERIGHT atau AUTOTUNELEFT
2. Monitor logs untuk overshoot detection
3. Verify: Kp reduction saat overshoot >2.5%
4. Check final Kp: should be 8-15 range
```

### Post-Test Validation:

```
1. Manual RPM test: 0→40 RPM
2. Expected: Smooth acceleration, no overshoot
3. Settling time: <3 seconds
4. Steady-state error: <1 RPM
```

## Safety Mechanisms

### Kp Minimum Protection:

```cpp
// Tidak biarkan Kp terlalu kecil
if (currentKp < 0.1) currentKp = 0.1;
constrainPIDValues(currentKp, currentKi, currentKd, 0.1, 150.0);
```

### Overshoot Emergency Reduction:

```cpp
// Jika overshoot >10% (emergency), kurangi Kp drastis
if (overshoot > 10.0) {
  currentKp *= 0.5; // Cut by half immediately
}
```

## Summary

Strategi **Anti-Overshoot** ini dirancang untuk mengatasi masalah Kp terlalu besar dengan:

✅ **Kp step size lebih kecil** → adjustment halus
✅ **Starting Kp lebih rendah** → start point aman  
✅ **Overshoot threshold ketat** → sensitif terhadap overshoot kecil
✅ **Scoring weight tinggi** → heavily penalize overshoot
✅ **Multi-level reduction** → graduated response
✅ **Better balance** → prioritize stability over speed

Expected outcome: **Smooth, controlled motor response tanpa overshoot** sambil tetap mempertahankan reasonable rise time.

---

**Update**: 14 Oktober 2025  
**Author**: GitHub Copilot  
**Status**: Implemented - Ready for Hardware Testing
