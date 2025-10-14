# Dokumentasi Auto Tuning Motor Kanan dan Kiri Terpisah

## Ringkasan Perubahan

Sistem AGV telah dimodifikasi untuk mendukung auto-tuning yang terpisah untuk motor kanan dan kiri karena nilai PID (Kp, Ki, Kd) kedua motor berbeda dan memerlukan kalibrasi individual.

## Struktur Sistem

```
AGV Master (agv_sami) --> Serial Commands --> AGV Slave (ESP32_Motor_Controller_Slave)
       |                                              |
   Menu Interface                              Auto Tuning Engine
   User Selection                              Individual PID Storage
```

## Fitur Baru

### 1. Menu Tuning Terpisah

- **Menu Utama**: Performance → Auto Tuning
- **Sub Menu Baru**:
  - "Tune Both Motors" (default behavior)
  - "Tune Right Motor" (motor kanan saja)
  - "Tune Left Motor" (motor kiri saja)

### 2. Command Protocol Baru

- `TUNE_BOTH`: Auto-tune kedua motor sekaligus
- `TUNE_RIGHT`: Auto-tune motor kanan saja
- `TUNE_LEFT`: Auto-tune motor kiri saja

### 3. Storage PID Terpisah

- Parameter PID disimpan secara terpisah di EEPROM
- Fungsi save terpisah untuk setiap motor
- Backward compatibility untuk sistem lama

## File yang Dimodifikasi

### AGV Master (agv_sami/)

#### menu.h

```cpp
// Fungsi baru untuk submenu tuning
void displayTuningStartMenu();
void handleTuningStartMenu();
void safeDelay(int ms);  // Watchdog-safe delay
```

#### menu.ino

- Menambah submenu tuning dengan 3 opsi
- Implementasi navigasi submenu
- Fungsi `safeDelay()` untuk mencegah watchdog timeout

#### setup.ino

- Konfigurasi watchdog timer
- Timeout 10 detik untuk stabilitas sistem

### AGV Slave (ESP32_Motor_Controller_Slave/)

#### auto_tuner.ino

```cpp
enum TuningTarget {
  TUNE_BOTH,
  TUNE_RIGHT,
  TUNE_LEFT
};

// Fungsi tuning spesifik untuk setiap target
void startAutoTuningBoth();
void startAutoTuningRight();
void startAutoTuningLeft();
```

#### serial.ino

- Handler untuk command `TUNE_RIGHT` dan `TUNE_LEFT`
- Routing ke fungsi tuning yang sesuai

#### preferences.ino

```cpp
// Fungsi save terpisah
void savePIDParametersRight(float kp, float ki, float kd);
void savePIDParametersLeft(float kp, float ki, float kd);
void savePIDParameters(float kp, float ki, float kd); // Backward compatibility
```

## Keunggulan Sistem Baru

### 1. Presisi Tuning

- Setiap motor dapat di-tune sesuai karakteristik fisiknya
- Nilai PID optimal untuk masing-masing motor
- Performa line following yang lebih baik

### 2. Fleksibilitas

- User dapat memilih tuning individual atau bersamaan
- Maintenance motor bisa dilakukan secara terpisah
- Debugging lebih mudah per motor

### 3. User Experience

- Interface menu yang intuitif
- Navigasi yang familiar dengan sistem existing
- Feedback real-time selama proses tuning

### 4. Stabilitas Sistem

- Watchdog timer untuk mencegah hang
- Safe delay functions
- Error handling yang robust

## Cara Penggunaan

### 1. Tuning Kedua Motor (Default)

1. Menu → Performance → Auto Tuning
2. Pilih "Tune Both Motors"
3. Tekan tombol untuk memulai
4. Tunggu proses selesai (~30-60 detik)

### 2. Tuning Motor Kanan Saja

1. Menu → Performance → Auto Tuning
2. Pilih "Tune Right Motor"
3. Tekan tombol untuk memulai
4. Motor kiri tetap menggunakan PID lama

### 3. Tuning Motor Kiri Saja

1. Menu → Performance → Auto Tuning
2. Pilih "Tune Left Motor"
3. Tekan tombol untuk memulai
4. Motor kanan tetap menggunakan PID lama

## Troubleshooting

### Error "esp_task_wdt_reset(707): task not found"

**Solusi**: Sudah diatasi dengan:

- Konfigurasi watchdog timer di setup
- Implementasi `safeDelay()` function
- Replace semua `delay()` dengan `safeDelay()`

### Tuning Tidak Optimal

**Tips**:

- Pastikan robot di trek yang sesuai
- Cek kondisi fisik motor dan roda
- Lakukan tuning ulang jika performa menurun

### Menu Tidak Responsif

**Cek**:

- Koneksi tombol navigasi
- Serial communication ke slave
- Status watchdog timer

## Parameter Default

### Tuning Configuration

```cpp
#define TUNING_AMPLITUDE 30.0    // Amplitude untuk oscillation test
#define TUNING_PERIOD_MS 2000    // Periode oscillation
#define TUNING_TIMEOUT_MS 60000  // Timeout 60 detik
```

### Watchdog Configuration

```cpp
#define WATCHDOG_TIMEOUT_SECONDS 10  // Timeout 10 detik
```

## Kompatibilitas

- ✅ Backward compatible dengan sistem lama
- ✅ Existing PID parameters tetap tersimpan
- ✅ Menu navigation yang familiar
- ✅ Serial protocol yang extensible

## Status Implementasi

- ✅ Auto tuning terpisah - COMPLETE
- ✅ Menu interface - COMPLETE
- ✅ Serial commands - COMPLETE
- ✅ EEPROM storage - COMPLETE
- ✅ Watchdog protection - COMPLETE
- ✅ Compilation success - VERIFIED
- ⏳ Field testing - PENDING

## Rekomendasi Selanjutnya

1. **Field Testing**: Test di trek sebenarnya
2. **Performance Monitoring**: Catat improvement dari tuning terpisah
3. **Calibration Schedule**: Tentukan frekuensi re-tuning
4. **Documentation Update**: Update manual operasional

---

_Dokumentasi dibuat: $(Get-Date)_  
_Engineer: GitHub Copilot_  
_Status: Ready for Field Testing_
