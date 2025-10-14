# Auto-Tuner PID RPM - ESP32 Motor Controller Slave

## Deskripsi

Fitur Auto-Tuner PID RPM adalah sistem cerdas yang secara otomatis menyesuaikan parameter PID (`Kp`, `Ki`, `Kd`) untuk mendapatkan kontrol RPM motor yang optimal pada AGV.

## Konsep Kerja

Auto-tuner menggunakan metode **Analisis Offline Cerdas** yang:

1. Menjalankan serangkaian tes motor dari kondisi berhenti ke target RPM (40 RPM)
2. Menganalisis respon motor (overshoot, rise time, error)
3. Menggunakan algoritma heuristik untuk menyempurnakan parameter PID
4. Menyimpan hasil terbaik ke Preferences (EEPROM)

## Cara Penggunaan

### 1. Via Serial Monitor (Debug)

Buka Serial Monitor pada baud rate 115200, kemudian ketik:

- `tune` - Mulai auto-tuning
- `cancel` - Batalkan auto-tuning
- `status` - Cek status dan progress tuning
- `help` - Tampilkan bantuan

### 2. Via Command dari Master ESP32 (AGV SAMI)

Master ESP32 dapat mengirim command melalui Serial1:

- `AUTOTUNE` atau `TUNE` - Mulai auto-tuning
- `TUNECANCEL` atau `CANCEL` - Batalkan auto-tuning
- `TUNESTATUS` - Cek status tuning

### 3. Response dari Slave ke Master

- `AUTOTUNE:STARTED` - Tuning dimulai
- `AUTOTUNE:ALREADY_RUNNING` - Tuning sudah berjalan
- `AUTOTUNE:CANCELLED` - Tuning dibatalkan
- `AUTOTUNE:PROGRESS:<persen>` - Progress tuning (0-100%)
- `AUTOTUNE:IDLE` - Tuning tidak aktif

## Parameter Tuning

### Target RPM

- **Default**: 40 RPM (dapat diubah di `TUNING_TARGET_RPM`)
- **Alasan**: RPM operasional realistis untuk AGV

### Durasi per Siklus

- **Test Duration**: 5 detik per tes
- **Cooldown**: 2 detik jeda antar tes
- **Total per siklus**: 7 detik

### Estimasi Waktu

- **Maksimum siklus**: 30 siklus
- **Total waktu**: ~3.5 menit
- **Catatan**: Tuning bisa selesai lebih cepat jika konvergensi tercapai

## Algoritma Heuristik

### Analisis Respon

1. **Overshoot**: Persentase RPM yang melampaui target
2. **Rise Time**: Waktu dari 10% ke 90% target RPM
3. **Average Error**: Rata-rata selisih RPM dengan target

### Aturan Penyesuaian

- **Overshoot > 20%**: Kurangi Kp drastis (×0.8), naikkan Kd (×1.15)
- **Overshoot 10-20%**: Kurangi Kp sedang (×0.9), naikkan Kd sedikit (×1.05)
- **Rise Time > 2s + Overshoot < 2%**: Naikkan Kp (×1.2)
- **Rise Time > 1.5s**: Naikkan Kp sedang (×1.1)
- **Respon bagus**: Optimalisasi Ki (×1.05)

### Batas Keamanan

- **Kp**: 0.5 - 100.0
- **Ki**: 0.01 - 50.0
- **Kd**: 0.001 - 50.0

## Persiapan Tuning

### 1. Posisi AGV

- Letakkan AGV di atas tumpuan/jack agar roda berputar bebas
- Pastikan roda tidak menyentuh lantai untuk keamanan

### 2. Kondisi

- Baterai dalam kondisi baik (minimal 70%)
- Tidak ada beban tambahan pada AGV
- Area sekitar aman dari gangguan

### 3. Monitoring

- Buka Serial Monitor untuk melihat progress real-time
- Perhatikan pesan status dan parameter yang diuji

## Output dan Hasil

### Selama Tuning

```
======================================
Memulai siklus tuning 1/30
Testing: Kp=20.0000, Ki=0.5000, Kd=0.1000
t=500ms, RPM=12.3, Target=40, Error=27.7
t=1000ms, RPM=35.6, Target=40, Error=4.4
...
Hasil siklus:
  - Overshoot: 8.50%
  - Rise Time: 1250 ms
  - Avg Error: 5.23 RPM
Skor: 42.35 (Terbaik sejauh ini: 42.35)
>>> DITEMUKAN PARAMETER TERBAIK BARU! <<<
```

### Setelah Selesai

```
======================================
AUTO-TUNING SELESAI!
Parameter terbaik: Kp=18.5000, Ki=0.6500, Kd=0.2000
Skor terbaik: 28.75
Menyimpan ke Preferences...
PID Parameters saved to flash
Tuning berhasil! Parameter optimal telah disimpan.
======================================
```

## Troubleshooting

### Tuning Tidak Mulai

- Cek apakah ada tuning lain yang sedang berjalan
- Pastikan koneksi serial dan motor normal
- Restart ESP32 jika perlu

### Hasil Tuning Buruk

- Pastikan encoder berfungsi dengan baik
- Cek kondisi mekanis roda dan motor
- Tuning ulang dalam kondisi berbeda (beban, voltase)

### Motor Tidak Bergerak Saat Tuning

- Cek koneksi kabel motor
- Pastikan encoder memberikan feedback RPM
- Verifikasi nilai `perRotasi` sesuai encoder

## Integrasi dengan AGV SAMI

Auto-tuner dirancang untuk berintegrasi seamless dengan ESP32 Master (AGV SAMI):

1. **Master dapat memulai tuning** saat AGV idle
2. **Progress monitoring** real-time dari master
3. **Hasil otomatis tersinkronisasi** ke master setelah tuning selesai
4. **Operasi normal AGV** dapat dilanjutkan setelah tuning

## File yang Terlibat

- `auto_tuner.ino` - Core logic auto-tuning
- `config.h` - Deklarasi fungsi dan konstanta
- `serial.ino` - Command handling dari master
- `preferences.ino` - Penyimpanan parameter
- `ESP32_Motor_Controller_Slave.ino` - Main loop integration

## Modifikasi dan Tuning

### Mengubah Target RPM

Edit konstanta di `auto_tuner.ino`:

```cpp
const int TUNING_TARGET_RPM = 40; // Ubah sesuai kebutuhan
```

### Mengubah Durasi Tes

```cpp
const unsigned long TEST_DURATION_MS = 5000; // 5 detik
const unsigned long COOLDOWN_DURATION_MS = 2000; // 2 detik
```

### Mengubah Batas Siklus

```cpp
const int MAX_TUNING_CYCLES = 30; // Maksimum 30 siklus
```

### Custom Scoring Formula

Edit bagian `TUNING_ANALYZING` di `handleAutoTuning()`:

```cpp
float score = (maxOvershoot * 3.0) + (riseTime * 0.01) + (avgError * 2.0);
```

## Keamanan

1. **Watchdog Timer** - Sistem akan restart jika hang
2. **Parameter Constraints** - Nilai PID dibatasi rentang aman
3. **Emergency Stop** - Tuning dapat dibatalkan kapan saja
4. **Motor Safety** - Motor otomatis berhenti setelah tuning

---

**Catatan**: Auto-tuner ini dioptimalkan untuk motor DC dengan encoder pada sistem AGV. Hasil terbaik didapat dalam kondisi AGV yang stabil dan terkalibrasi dengan baik.
