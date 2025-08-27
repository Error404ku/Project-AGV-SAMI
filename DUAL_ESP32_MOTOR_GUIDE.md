# ESP32 D### ESP32 Master (AGV Controller):
- **Serial1**: TX=14, RX=13 untuk komunikasi dengan ESP32 Motor
- **Baudrate**: 115200

### ESP32 Slave (Motor Controller):
- **Serial1**: RX=41, TX=42
- **Motor 1 (Kiri)**: D1=4, D2=5, PWM=6
- **Motor 2 (Kanan)**: D1=7, D2=15, PWM=16ol System

## Deskripsi
Sistem ini menggunakan 2 ESP32 untuk kontrol motor:
- **ESP32 Master**: Menjalankan logika AGV dan mengirim perintah motor
- **ESP32 Slave**: Menerima perintah dan mengontrol motor secara langsung

## Pin Configuration

### ESP32 Master (AGV Controller)
- **Serial Communication**: Serial0 (default pins) untuk komunikasi dengan ESP32 Slave

### ESP32 Slave (Motor Controller)
- **Serial Communication**: 
  - RX: Pin 41
  - TX: Pin 42
- **Motor 1 (Kiri)**:
  - D1: Pin 4
  - D2: Pin 5  
  - PWM: Pin 6
- **Motor 2 (Kanan)**:
  - D1: Pin 7
  - D2: Pin 15
  - PWM: Pin 16

## Protocol Komunikasi

### Format Perintah
Format: `L[speed_kiri]R[speed_kanan]\n`

### Contoh Perintah
- `L2000R2000\n` - Kedua motor maju dengan kecepatan 2000 (sekitar 50% dari maksimal)
- `L-1000R1000\n` - Putar kiri (motor kiri mundur 1000, motor kanan maju 1000)
- `L4095R4095\n` - Maju dengan kecepatan maksimal (100%)
- `L0R0\n` - Stop semua motor

### Range Kecepatan
- **12-bit PWM Range**: -4095 to 4095
- **Minimum**: -4095 (mundur maksimal)
- **Maximum**: 4095 (maju maksimal)
- **Stop**: 0

## Cara Upload Program

### 1. Upload ke ESP32 Master
1. Buka Arduino IDE
2. Pilih board "ESP32S3 Dev Module"
3. Upload seluruh folder `agv_sami` ke ESP32 pertama

### 2. Upload ke ESP32 Slave
1. Buka file `ESP32_Motor_Controller_Slave.ino`
2. Pilih board "ESP32S3 Dev Module"  
3. Upload ke ESP32 kedua

## Koneksi Hardware

### Antar ESP32
```
ESP32 Master (Serial1) <---> ESP32 Slave (Serial1)
Master TX (Pin 14) ----------> Slave RX (Pin 41)
Master RX (Pin 13) ----------> Slave TX (Pin 42)
GND ----------------------------> GND
```

### ESP32 Slave ke Motor Driver
```
Motor 1 (Kiri):
Pin 4 (D1) -----> Motor Driver IN1
Pin 5 (D2) -----> Motor Driver IN2  
Pin 6 (PWM) ----> Motor Driver ENA

Motor 2 (Kanan):
Pin 7 (D1) -----> Motor Driver IN3
Pin 15 (D2) ----> Motor Driver IN4
Pin 16 (PWM) ---> Motor Driver ENB
```

## Troubleshooting

### Motor Tidak Bergerak
1. Cek koneksi serial antar ESP32
2. Cek apakah ESP32 Slave sudah upload program
3. Monitor Serial ESP32 Slave untuk melihat perintah yang diterima

### Komunikasi Gagal
1. Pastikan baudrate sama (115200)
2. Cek koneksi TX-RX antar ESP32
3. Pastikan ground (GND) terhubung

### Motor Bergerak Terbalik
- Edit fungsi `setMotorSpeed()` di ESP32 Slave
- Tukar pin D1 dan D2 untuk motor yang terbalik

## Testing

### Test Komunikasi
1. Upload program ke kedua ESP32
2. Buka Serial Monitor ESP32 Slave
3. ESP32 Master akan mengirim perintah stop saat startup
4. Anda harus melihat "Received command: L0R0" di Serial Monitor

### Test Manual Motor
Tambahkan kode ini di loop() ESP32 Master untuk test:
```cpp
// Test motor - hapus setelah testing
kirimPerintahMotor(2000, 2000); // Maju 50% kecepatan
delay(2000);
kirimPerintahMotor(0, 0);       // Stop  
delay(1000);

// Atau gunakan fungsi persentase
motorPersentase(50, 50);        // Maju 50%
delay(2000);
motorPersentase(0, 0);          // Stop
delay(1000);
```

## Fungsi Yang Tersedia

### Fungsi Dasar
- `kirimPerintahMotor(speedKiri, speedKanan)` - Kirim perintah PWM langsung (range: -4095 to 4095)
- `motorPersentase(persenKiri, persenKanan)` - Kirim perintah dengan persentase (range: -100% to 100%)

### Fungsi Gerakan
- `motorMaju(speed)` - Gerak maju dengan kecepatan tertentu
- `motorMundur(speed)` - Gerak mundur dengan kecepatan tertentu  
- `motorKiri(speed)` - Putar kiri dengan kecepatan tertentu
- `motorKanan(speed)` - Putar kanan dengan kecepatan tertentu
- `motorBerhenti()` - Stop semua motor
