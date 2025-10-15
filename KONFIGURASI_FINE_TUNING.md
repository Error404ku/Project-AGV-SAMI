# 🎛️ KONFIGURASI FINE TUNING SYSTEM

## 📍 LOKASI KONFIGURASI

Semua konfigurasi fine tuning berada di file:
`ESP32_Motor_Controller_Slave/auto_tuner.ino` (baris 8-60)

## 🔧 1. STEP SIZES - PRECISION LEVELS

### COARSE STEP (Pencarian Awal)

```cpp
const float KP_COARSE_STEP = 5.0;              // Langkah kasar untuk Kp
const float KI_COARSE_STEP = 0.05;             // Langkah kasar untuk Ki
const float KD_COARSE_STEP = 0.1;              // Langkah kasar untuk Kd
```

### FINE STEP (Pencarian Menengah)

```cpp
const float KP_FINE_STEP = 1.0;                // Langkah halus untuk Kp
const float KI_FINE_STEP = 0.01;               // Langkah halus untuk Ki
const float KD_FINE_STEP = 0.02;               // Langkah halus untuk Kd
```

### ULTRA-FINE STEP (Pencarian Akhir)

```cpp
const float KP_ULTRA_FINE_STEP = 0.2;          // Langkah ultra halus untuk Kp
const float KI_ULTRA_FINE_STEP = 0.002;        // Langkah ultra halus untuk Ki
const float KD_ULTRA_FINE_STEP = 0.005;        // Langkah ultra halus untuk Kd
```

## 📊 2. SCORING WEIGHTS (Bobot Penilaian)

```cpp
const float OVERSHOOT_WEIGHT = 4.0;            // Bobot overshoot dalam scoring
const float RISE_TIME_WEIGHT = 0.02;           // Bobot rise time dalam scoring
const float ERROR_WEIGHT = 2.5;                // Bobot average error dalam scoring
const float STABILITY_WEIGHT = 1.5;            // Bobot stabilitas dalam scoring
```

## 🎯 3. PERFORMANCE THRESHOLDS (Ambang Performa)

```cpp
const float HIGH_OVERSHOOT_THRESHOLD = 20.0;   // Overshoot tinggi
const float MEDIUM_OVERSHOOT_THRESHOLD = 10.0; // Overshoot sedang
const float LOW_OVERSHOOT_THRESHOLD = 2.0;     // Overshoot rendah

const float EXCELLENT_SCORE_THRESHOLD = 15.0;  // Score excellent untuk switch precision
const float GOOD_SCORE_THRESHOLD = 25.0;       // Score baik untuk evaluasi

const unsigned long SLOW_RISE_TIME_MS = 2000;  // Rise time dianggap lambat
const unsigned long MEDIUM_RISE_TIME_MS = 1500; // Rise time sedang
```

## ⏱️ 4. TRANSITION TIMING (Kapan Pindah Stage)

Di fungsi `checkPrecisionStageTransition()` (baris ~591):

```cpp
// Jika sudah 8 cycles tanpa improvement significant, naik ke precision berikutnya
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
```

## 🎚️ 5. CARA MENGUBAH KONFIGURASI

### Untuk Precision Yang Lebih Halus:

```cpp
// Kurangi step sizes
const float KP_ULTRA_FINE_STEP = 0.1;          // Dari 0.2 ke 0.1
const float KI_ULTRA_FINE_STEP = 0.001;        // Dari 0.002 ke 0.001
```

### Untuk Tuning Yang Lebih Cepat:

```cpp
// Tingkatkan step sizes
const float KP_COARSE_STEP = 8.0;              // Dari 5.0 ke 8.0
const float KI_COARSE_STEP = 0.08;             // Dari 0.05 ke 0.08
```

### Untuk Lebih Fokus pada Overshoot:

```cpp
const float OVERSHOOT_WEIGHT = 6.0;            // Dari 4.0 ke 6.0
const float HIGH_OVERSHOOT_THRESHOLD = 15.0;   // Dari 20.0 ke 15.0
```

### Untuk Transition Yang Lebih Patient:

```cpp
// Di checkPrecisionStageTransition()
if (cyclesWithoutImprovement >= 12) {          // Dari 8 ke 12
if (precisionStageCount >= 20) {               // Dari 15 ke 20
```

## 🧪 6. TESTING KONFIGURASI BARU

### 1. Edit Konfigurasi

Edit nilai-nilai di `auto_tuner.ino` baris 20-60

### 2. Compile & Upload

```bash
arduino-cli compile --fqbn esp32:esp32:esp32s3 .
arduino-cli upload --fqbn esp32:esp32:esp32s3 -p COM9 .
```

### 3. Test Fine Tuning

```
RESET_PID          # Reset ke default
AUTOTUNE_RIGHT     # Test motor kanan
AUTOTUNE_LEFT      # Test motor kiri
```

### 4. Monitor Hasil

Perhatikan serial output untuk:

- Stage transitions: "Switching to FINE tuning stage"
- Parameter adjustments: "Action: Fine increase Kp and Ki"
- Final results: "AUTOTUNE_RIGHT:COMPLETED:..."

## 📈 7. CONTOH KONFIGURASI CUSTOM

### Untuk Motor Yang Sensitif:

```cpp
const float KP_COARSE_STEP = 2.0;              // Lebih halus
const float KI_COARSE_STEP = 0.02;             // Lebih halus
const float OVERSHOOT_WEIGHT = 6.0;            // Lebih strict pada overshoot
```

### Untuk Motor Yang Lambat:

```cpp
const float KP_COARSE_STEP = 8.0;              // Lebih agresif
const float SLOW_RISE_TIME_MS = 3000;          // Toleransi lebih tinggi
const float KP_INCREASE_FACTOR = 1.2;          // Dari 1.1 ke 1.2
```

### Untuk Precision Maksimal:

```cpp
const float KP_ULTRA_FINE_STEP = 0.05;         // Sangat halus
const float KI_ULTRA_FINE_STEP = 0.0005;       // Sangat halus
// Tambah cycles per stage
if (precisionStageCount >= 25) {               // Dari 15 ke 25
```

## 🎯 8. REKOMENDASI BERDASARKAN KONDISI

### Motor Bergetar (Oscillation):

- Kurangi `KP_COARSE_STEP` dan `KP_FINE_STEP`
- Tingkatkan `KD_COARSE_STEP` untuk damping lebih
- Tingkatkan `OVERSHOOT_WEIGHT`

### Motor Lambat Respon:

- Tingkatkan `KP_COARSE_STEP`
- Kurangi `SLOW_RISE_TIME_MS` threshold
- Tingkatkan `KP_INCREASE_FACTOR`

### Butuh Precision Tinggi:

- Kurangi semua `*_ULTRA_FINE_STEP`
- Tingkatkan `precisionStageCount` limit
- Kurangi `EXCELLENT_SCORE_THRESHOLD`

Semua perubahan konfigurasi ini akan langsung berpengaruh pada proses auto-tuning setelah compile dan upload ulang! 🚀
