# 🔧 PERBAIKAN AUTO-TUNING MOTOR KIRI

## ❌ MASALAH YANG DITEMUKAN

Motor kiri tidak bergerak saat auto-tuning karena:

1. **Nilai Kp terlalu rendah**: Mulai dari 0.5 (harusnya minimal 10.0)
2. **Tidak ada protection**: Motor kanan ada check minimum Kp, motor kiri tidak
3. **PWM insufficient**: Dengan Kp=0.5, PWM output terlalu kecil untuk menggerakkan motor

## ✅ PERBAIKAN YANG TELAH DILAKUKAN

### 1. Tambah Protection untuk Motor Kiri

```cpp
// Di auto_tuner.ino, fungsi startAutoTuningGeneric()
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
```

### 2. Update Default Values yang Realistis

```cpp
// Di preferences.ino, fungsi resetPIDParameters()
pidConfigLeft.kp = 20.0;   // Was 0.5, now realistic
pidConfigLeft.ki = 0.05;   // Was 0.01, now adequate
pidConfigLeft.kd = 0.01;   // Was 0.0, now with damping
```

## 🧪 CARA TESTING SETELAH UPLOAD

### 1. Reset PID Parameters

Kirim command dari AGV Master:

```
RESET_PID
```

### 2. Verifikasi Values

Kirim command:

```
GET_PID_LEFT
```

Harusnya response: `PIDLEFT_VALUES:20.000,0.050,0.010`

### 3. Test Auto-Tuning Motor Kiri

Kirim command:

```
AUTOTUNE_LEFT
```

### 4. Monitor Serial Output

Cari output seperti:

```
WARNING: Left motor Kp too low (0.500), starting with 20.0
[TUNING] Starting AUTO-TUNING for Motor Kiri
Target RPM: 40.0, Initial PID: Kp=20.00, Ki=0.05, Kd=0.01
```

## 📊 HASIL YANG DIHARAPKAN

1. **Motor kiri akan bergerak** saat auto-tuning dimulai
2. **PWM output > 100** (bukan 44 seperti sebelumnya)
3. **RPM mencapai target 40** dalam 2-3 detik
4. **Auto-tuning selesai** dengan parameter optimal

## 🔍 TROUBLESHOOTING

Jika motor kiri masih tidak bergerak:

1. Cek wiring motor kiri (MOTOR2_D1, MOTOR2_D2, MOTOR2_PWM)
2. Cek encoder motor kiri (pins 37, 38)
3. Verifikasi pwmKiri value dengan command `GET_PWM`
4. Test manual dengan `RPM_MOTOR:0,40` (hanya motor kiri)

## 📈 PERBANDINGAN SEBELUM vs SESUDAH

| Parameter   | Motor Kanan | Motor Kiri (Before) | Motor Kiri (After) |
| ----------- | ----------- | ------------------- | ------------------ |
| Starting Kp | 41.472      | 0.5 ❌              | 20.0 ✅            |
| Protection  | Yes ✅      | No ❌               | Yes ✅             |
| PWM Output  | ~180        | ~44 ❌              | ~120+ ✅           |
| Movement    | Works ✅    | No movement ❌      | Should work ✅     |

## 🎯 NEXT STEPS

1. Upload code yang sudah diperbaiki
2. Reset PID parameters dengan `RESET_PID`
3. Test auto-tuning dengan `AUTOTUNE_LEFT`
4. Jika berhasil, lanjut test `AUTOTUNE_RIGHT` dan `AUTOTUNE_BOTH`
