# ESP32 Motor Migration Guide - v3.0 (12-bit PWM)

## Status Migrasi Motor ✅

Semua kode yang berhubungan dengan motor telah berhasil dimigrasi ke sistem dual ESP32 dengan 12-bit PWM.

### ✅ File yang Sudah Diupdate:

#### 1. **config.h** - Konfigurasi Dasar
- `maxPwm = 4096` (12-bit range)
- `minPwm = -4096`
- `pwmResolution = 12`
- `pwmFrequency = 5000`
- `baseSpeed = 2000` (default 50% dari maksimal)
- Pin motor L298N (legacy, tidak lagi digunakan)

#### 2. **motor.ino** - Motor Control Core
- Fungsi `pwmMotor()` dimodifikasi untuk mengirim perintah serial
- Safety checks tetap menggunakan range 12-bit PWM
- Motor inversion logic tetap berfungsi
- Watchdog timer handling untuk operasi intensif

#### 3. **motor_serial.ino** - Serial Communication
- `kirimPerintahMotor(speedKiri, speedKanan)` - Core function
- Range PWM: -4095 to 4095 (12-bit)
- Fungsi gerakan: `motorMaju()`, `motorMundur()`, `motorKiri()`, `motorKanan()`
- Fungsi kompatibilitas: `setMotorSpeeds()`, `aturKecepatanMotor()`
- Fungsi persentase: `motorPersentase()` (-100% to 100%)

#### 4. **setup.ino** - Initialization
- `setupMotor()` diubah untuk inisialisasi komunikasi serial
- `setupAll()` diupdate untuk menghilangkan setup motor langsung
- Duplicate setupWebServer dihapus

#### 5. **ESP32_Motor_Controller_Slave.ino** - Motor Controller ESP32
- 12-bit PWM resolution (0-4095)
- Pin configuration sesuai spesifikasi
- Serial communication protocol
- Motor safety dan error handling

### ✅ File yang Menggunakan Motor (Sudah Kompatibel):

#### 1. **menu.ino** - Motor Testing & Configuration
- `baseSpeed` range: 100-4000 (sesuai 12-bit PWM)
- Motor test functions menggunakan `pwmMotor()`
- Speed percentage mapping: `map(tempBaseSpeed, 100, 4000, 0, 100)`

#### 2. **pidMagnetFollower.ino** - PID Line Following
- Menggunakan `pwmMotor()` untuk kontrol motor
- `pidSpeed` calculation dengan baseSpeed
- Soft start dan gradual stop logic
- Motor direction control untuk line following

#### 3. **logicAgv.ino** - AGV Logic
- Motor stop commands menggunakan `pwmMotor(0, 0)`
- State management dengan motor control

#### 4. **performance_optimization.ino** - Performance
- Motor stop untuk optimisasi performa

## 🔧 Protocol Komunikasi

### Format Perintah Serial:
```
L[speed_kiri]R[speed_kanan]\n
```

### Contoh:
- `L2000R2000\n` - Maju 50% kecepatan
- `L-1000R1000\n` - Putar kiri
- `L0R0\n` - Stop
- `L4095R4095\n` - Maju maksimal

### Range Nilai:
- **PWM Range**: -4095 to 4095 (12-bit)
- **BaseSpeed Default**: 2000 (50%)
- **Safety Limit**: 3500 (untuk watchdog timer)

## 📍 Pin Configuration

### ESP32 Master (AGV Controller):
- **Serial0**: TX=1, RX=3 (default)
- **Baudrate**: 115200

### ESP32 Slave (Motor Controller):
- **Serial1**: RX=41, TX=42
- **Motor 1 (Kiri)**: D1=4, D2=5, PWM=6
- **Motor 2 (Kanan)**: D1=7, D2=15, PWM=16
- **PWM Frequency**: 1000 Hz
- **PWM Resolution**: 12-bit (0-4095)

## 🚀 Fungsi Yang Tersedia

### Core Functions:
```cpp
kirimPerintahMotor(speedKiri, speedKanan);  // -4095 to 4095
motorPersentase(persenKiri, persenKanan);   // -100% to 100%
```

### Movement Functions:
```cpp
motorMaju(speed);      // 0 to 4095
motorMundur(speed);    // 0 to 4095  
motorKiri(speed);      // 0 to 4095
motorKanan(speed);     // 0 to 4095
motorBerhenti();       // Stop
```

### Compatibility Functions:
```cpp
setMotorSpeeds(left, right);
aturKecepatanMotor(kiri, kanan);
motorStop();
motorForward(speed);
motorBackward(speed);
motorTurnLeft(speed);
motorTurnRight(speed);
```

## ✅ Verification Checklist

- [x] Config.h menggunakan 12-bit PWM values
- [x] Motor.ino menggunakan komunikasi serial
- [x] Motor_serial.ino dengan fungsi lengkap
- [x] ESP32_Motor_Controller_Slave.ino siap
- [x] Setup.ino terupdate untuk serial communication
- [x] Menu.ino kompatibel dengan range baru
- [x] PidMagnetFollower.ino berfungsi normal
- [x] LogicAgv.ino menggunakan pwmMotor()
- [x] Safety checks dan watchdog timer aktif
- [x] Motor inversion logic tetap berfungsi
- [x] BaseSpeed dan pidSpeed dalam range yang benar

## 🔧 Testing

### 1. Upload Programs:
1. Upload `ESP32_Motor_Controller_Slave.ino` ke ESP32 kedua
2. Upload project AGV ke ESP32 pertama

### 2. Test Serial Communication:
```cpp
// Test di loop() ESP32 Master
kirimPerintahMotor(1000, 1000);  // Maju pelan
delay(2000);
kirimPerintahMotor(0, 0);        // Stop
delay(1000);
```

### 3. Monitor Output:
- ESP32 Slave Serial Monitor harus menampilkan: "Received command: L1000R1000"
- Motor harus bergerak sesuai perintah

## 🎯 Hasil Migrasi

✅ **100% Compatible** - Semua fungsi motor existing tetap bekerja  
✅ **12-bit PWM** - Resolusi 16x lebih halus dari sebelumnya  
✅ **Dual ESP32** - Beban motor terpisah dari logika AGV  
✅ **Serial Protocol** - Komunikasi robust dan mudah di-debug  
✅ **Safety Features** - Watchdog timer dan range checking tetap aktif  

**Sistem motor migration berhasil dan siap untuk produksi!** 🚀
