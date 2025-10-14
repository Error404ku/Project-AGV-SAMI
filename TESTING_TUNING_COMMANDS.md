# Testing Protocol - Auto Tuning Motor Individual

## Masalah yang Ditemukan

Fungsi `sendTuningCommand()` di `menu.ino` tidak mengirim perintah ke slave ESP32 dengan benar. Perintah hanya dikirim ke Serial debug, tidak ke Serial yang berkomunikasi dengan motor controller slave.

## Perbaikan yang Dilakukan

### 1. Fix sendTuningCommand Function (menu.ino)

```cpp
// SEBELUM (SALAH):
void sendTuningCommand(String command) {
  // Serial1.println(command);  // Dikomentari!
  Serial.println(command);     // Hanya debug
}

// SESUDAH (BENAR):
void sendTuningCommand(String command) {
  Serial.println(command);     // Kirim ke slave via Serial
  Serial.println("Sending tuning command: " + command);  // Debug
}
```

### 2. Enhanced Response Handler (motor_serial.ino)

```cpp
// Tambahan handler untuk respons AUTOTUNE_RIGHT dan AUTOTUNE_LEFT
} else if (message.startsWith("AUTOTUNE:") ||
           message.startsWith("AUTOTUNE_RIGHT:") ||
           message.startsWith("AUTOTUNE_LEFT:")) {
  parseTuningResponse(message);
}
```

## Testing Steps

### 1. Upload Firmware

- ✅ ESP32 Motor Controller Slave uploaded ke COM11
- ✅ AGV SAMI Master uploaded ke COM3

### 2. Test Individual Motor Tuning

#### A. Test Tune Right Motor:

1. Masuk menu: Main Menu → Tuning RPM → Tune Right Motor
2. Pilih "Start Tuning"
3. **Verifikasi**: Hanya motor kanan yang bergerak
4. Monitor Serial untuk melihat perintah "TUNE_RIGHT" dikirim
5. Tunggu proses auto-tuning selesai

#### B. Test Tune Left Motor:

1. Masuk menu: Main Menu → Tuning RPM → Tune Left Motor
2. Pilih "Start Tuning"
3. **Verifikasi**: Hanya motor kiri yang bergerak
4. Monitor Serial untuk melihat perintah "TUNE_LEFT" dikirim
5. Tunggu proses auto-tuning selesai

### 3. Verification Points

#### Serial Communication:

- Master harus mengirim: `TUNE_RIGHT` atau `TUNE_LEFT`
- Slave harus merespons: `AUTOTUNE_RIGHT:STARTED` atau `AUTOTUNE_LEFT:STARTED`
- Progress updates: `AUTOTUNE_RIGHT:PROGRESS:XX`
- Completion: `AUTOTUNE_RIGHT:COMPLETED:kp,ki,kd`

#### Motor Movement:

- **TUNE_RIGHT**: Hanya motor kanan berputar (rpm_depan_kanan > 0, rpm_depan_kiri = 0)
- **TUNE_LEFT**: Hanya motor kiri berputar (rpm_depan_kiri > 0, rpm_depan_kanan = 0)

#### PID Storage:

- Right motor PID disimpan ke `pidConfigRight`
- Left motor PID disimpan ke `pidConfigLeft`
- Values tersimpan di EEPROM slave ESP32

## Expected Results

### Right Motor Tuning:

```
auto_tuner.ino: setTuningTargetRPM() → rpmMotor(targetRPM, 0)
rpm.ino: Motor 2 (pins 7,15,16) bergerak
Serial: "TUNE_RIGHT" → "AUTOTUNE_RIGHT:STARTED"
```

### Left Motor Tuning:

```
auto_tuner.ino: setTuningTargetRPM() → rpmMotor(0, targetRPM)
rpm.ino: Motor 1 (pins 5,4,6) bergerak
Serial: "TUNE_LEFT" → "AUTOTUNE_LEFT:STARTED"
```

## Troubleshooting

### Jika Motor Salah Bergerak:

- Periksa mapping di `rpm.ino`: Motor 1=KIRI, Motor 2=KANAN
- Verifikasi `setMotorSpeed(1, pwmKiri)` dan `setMotorSpeed(2, pwmKanan)`

### Jika Tidak Ada Respons:

- Monitor Serial untuk memastikan perintah terkirim
- Periksa koneksi Serial antara master dan slave
- Restart kedua ESP32

### Jika Tuning Tidak Dimulai:

- Pastikan tidak ada tuning lain yang sedang berjalan
- Cek `isTuningActive()` status di slave
- Verifikasi komunikasi Serial

## Files Modified

1. `agv_sami/menu.ino` - Fixed sendTuningCommand()
2. `agv_sami/motor_serial.ino` - Enhanced response handler
3. `ESP32_Motor_Controller_Slave/auto_tuner.ino` - Individual motor targeting
4. `ESP32_Motor_Controller_Slave/serial.ino` - TUNE_RIGHT/TUNE_LEFT handlers
5. `ESP32_Motor_Controller_Slave/rpm.ino` - Corrected motor mapping

## Final Verification

Setelah testing, pastikan:

- [ ] "Tune Right Motor" hanya menggerakkan motor kanan
- [ ] "Tune Left Motor" hanya menggerakkan motor kiri
- [ ] PID values tersimpan dengan benar untuk setiap motor
- [ ] Serial communication berfungsi normal
- [ ] AGV dapat menggunakan PID yang sudah di-tune secara individual
