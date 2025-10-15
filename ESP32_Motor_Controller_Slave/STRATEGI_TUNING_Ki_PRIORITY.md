# Strategi Auto-Tuning dengan Prioritas Ki

## Latar Belakang Masalah

### Gejala yang Diamati

- Motor **tiba-tiba bergerak kencang sekali** di awal (initial burst)
- Setelah burst awal, motor baru menggunakan kecepatan normal
- Perilaku ini tidak ideal dan mengganggu performa AGV

### Analisis Root Cause

Parameter **Ki (Integral)** memiliki pengaruh sangat besar terhadap perilaku motor karena:

1. **Akumulasi Error**: Ki mengakumulasi error dari waktu ke waktu
2. **Initial Burst Effect**: Jika Ki terlalu tinggi, akumulasi error di awal menyebabkan output PWM melompat tinggi
3. **Steady-State Impact**: Ki yang optimal penting untuk menghilangkan steady-state error tanpa menyebabkan burst

## Perubahan Strategi Tuning

### 1. **Step Size Ki Diperbesar** (Lebih Agresif)

```cpp
// SEBELUM:
const float KI_COARSE_STEP = 0.05;
const float KI_FINE_STEP = 0.01;
const float KI_ULTRA_FINE_STEP = 0.002;

// SEKARANG:
const float KI_COARSE_STEP = 0.1;      // 2x lebih besar
const float KI_FINE_STEP = 0.03;       // 3x lebih besar
const float KI_ULTRA_FINE_STEP = 0.008; // 4x lebih besar
```

**Alasan**: Ki perlu adjustment range yang lebih lebar karena pengaruhnya sangat signifikan terhadap motor behavior.

### 2. **Scoring Weight Ditingkatkan**

```cpp
// SEBELUM:
const float OVERSHOOT_WEIGHT = 5.0;
const float ERROR_WEIGHT = 2.5;
const float STABILITY_WEIGHT = 1.5;

// SEKARANG:
const float OVERSHOOT_WEIGHT = 6.0;           // +20%
const float ERROR_WEIGHT = 3.5;               // +40%
const float STABILITY_WEIGHT = 2.0;           // +33%
const float INITIAL_BURST_PENALTY = 4.0;      // BARU!
```

**Alasan**: Error dan stability lebih penting untuk mendeteksi masalah Ki, overshoot juga indikator Ki terlalu tinggi.

### 3. **Deteksi Initial Burst** (Fitur Baru)

```cpp
struct PerformanceMetrics {
  // ... existing fields ...
  float initialBurst;         // Track initial burst speed
  bool burstDetected;         // Flag untuk deteksi burst
  unsigned long burstTime;    // Waktu saat burst terjadi
};
```

**Cara Deteksi**:

- Monitor RPM dalam **500ms pertama** setelah motor start
- Jika RPM melebihi target **>30%** → burst terdeteksi
- Catat magnitude burst dan timing untuk analisis

**Output Debug**:

```
[BURST DETECTED] RPM=65.3 at 250ms (63.2% over target)
```

### 4. **Algoritma Adjustment Ki-Priority**

#### Strategi Baru dalam `TUNING_UPDATING_PARAMS`:

```cpp
// PRIORITAS 1: Atasi Initial Burst (Ki terlalu tinggi)
if (currentMetrics.burstDetected) {
  if (burstPercent > 50%) {
    currentKi -= currentKiStep * 3.0;  // Drastic reduction
    currentKp -= currentKpStep * 0.5;
  }
  else if (burstPercent > 30%) {
    currentKi -= currentKiStep * 2.0;  // Significant reduction
    currentKd += currentKdStep * 0.8;
  }
  else if (burstPercent > 15%) {
    currentKi -= currentKiStep * 1.2;  // Moderate reduction
  }
  else {
    currentKi -= currentKiStep * 0.5;  // Fine reduction
  }
}

// PRIORITAS 2: Response lambat → Naikkan Ki lebih banyak dari Kp
else if (slow_rise && no_overshoot) {
  currentKi += currentKiStep * 1.5;  // Ki priority
  currentKp += currentKpStep * 0.8;  // Kp secondary
}

// PRIORITAS 3: Steady-state error tinggi → Optimasi Ki
else if (high_avg_error && safe_overshoot) {
  currentKi += currentKiStep * 1.0;
}

// PRIORITAS 4: Fine-tuning Ki exploratory
else {
  if (cycle % 2 == 0) {
    currentKi += currentKiStep * 0.4;
  }
}
```

## Perbandingan: Sebelum vs Sesudah

### Strategi Lama (Generic)

```
- Fokus seimbang Kp, Ki, Kd
- Ki adjustment step kecil (0.05, 0.01, 0.002)
- Tidak ada deteksi burst
- Kp lebih dominan dalam adjustment
```

### Strategi Baru (Ki-Priority)

```
✓ Fokus utama pada Ki
✓ Ki adjustment step 2-4x lebih besar
✓ Deteksi initial burst otomatis
✓ Ki mendapat perhatian khusus di setiap decision
✓ Penalty besar untuk burst behavior
```

## Scoring System dengan Burst Penalty

### Formula Lengkap:

```cpp
score = (overshoot * 6.0) +
        (riseTime * 0.015) +
        (avgError * 3.5) +
        (stability * 2.0);

if (burstDetected) {
  burstPenalty = (burstPercent * 4.0);
  score += burstPenalty;
}
```

### Contoh Perhitungan:

**Case 1: Motor dengan Burst**

- Overshoot: 8% → 48.0 points
- Rise time: 1200ms → 18.0 points
- Avg error: 4.5 RPM → 15.75 points
- **Burst: 45% → 180.0 points penalty**
- **Total: 261.75** (score buruk karena burst)

**Case 2: Motor Tanpa Burst**

- Overshoot: 3% → 18.0 points
- Rise time: 1500ms → 22.5 points
- Avg error: 3.2 RPM → 11.2 points
- Burst: None → 0.0 points
- **Total: 51.7** (score baik, tidak ada burst)

## Debug Output

### Format Log Baru:

```
[TUNING CYCLE 5] Menganalisis hasil:
  Overshoot=8.50%, RiseTime=1250ms, AvgError=4.20 RPM
  InitialBurst=58.3 RPM at 320ms
  - Burst Penalty: 145.00

>>> ACTION: Significant Ki reduction (burst >30%)
New parameters: Kp=18.200, Ki=0.084, Kd=0.142
Precision: COARSE, Cycles without improvement: 2
```

## Ekspektasi Hasil

### Target Performance:

1. **Eliminasi Burst**: Motor start smooth tanpa lonjakan tiba-tiba
2. **Response Balance**: Rise time tetap cepat tapi controlled
3. **Steady-State Optimal**: Error minimum di kecepatan konstan
4. **Ki Range Optimal**: Biasanya 0.01 - 0.15 untuk motor DC dengan encoder

### Testing Procedure:

1. Start auto-tuning: `AUTOTUNERIGHT` atau `AUTOTUNELEFT`
2. Observe logs untuk burst detection
3. Monitor Ki adjustment trajectory
4. Verify final Ki value mencegah burst
5. Test motor start dari 0 → 40 RPM manually untuk konfirmasi

## Proteksi Safety

```cpp
// Ki tidak boleh terlalu kecil (integral action hilang)
if (currentKi < 0.001) currentKi = 0.001;

// Constrain dalam range aman
constrainPIDValues(currentKp, currentKi, currentKd, 0.1, 150.0);
```

## Precision Stages

Algorithm tetap menggunakan multi-stage precision:

1. **COARSE**: Pencarian awal Ki range optimal dengan step besar
2. **FINE**: Refinement Ki dengan step menengah
3. **ULTRA-FINE**: Final optimization Ki dengan step kecil

Transisi stage terjadi jika:

- 8 cycles tanpa improvement
- Score sudah excellent (<15.0)
- Sudah 15 cycles di stage yang sama

## Kesimpulan

Strategi Ki-Priority ini dirancang khusus untuk mengatasi masalah **initial burst** yang disebabkan oleh Ki terlalu tinggi. Dengan:

- Detection otomatis
- Adjustment agresif pada Ki
- Penalty scoring untuk burst behavior
- Step size lebih besar untuk Ki

Auto-tuning akan menemukan nilai Ki optimal yang memberikan **smooth acceleration** tanpa burst, sambil tetap menjaga **steady-state performance** yang baik.

---

**Update**: 14 Oktober 2025
**Author**: GitHub Copilot
**Status**: Implemented & Ready for Testing
