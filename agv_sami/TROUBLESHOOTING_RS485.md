# Troubleshooting Guide - Unified RS485 System

## Masalah Umum dan Solusi

### 1. Sensor Magnet Tidak Terbaca

**Gejala:**

- Error code 0x02, 0x03, atau 0x04 pada Serial Monitor
- Device status menunjukkan "MF:ERR" atau "MB:ERR"
- Tidak ada data magnet yang diterima

**Kemungkinan Penyebab:**

- Baudrate tidak sesuai (pastikan 9600)
- Address sensor tidak sesuai (default: Front=1, Back=4)
- Kabel RS485 A/B terbalik
- Power supply sensor tidak stabil
- Jarak kabel RS485 terlalu panjang tanpa termination

**Solusi:**

1. **Cek Baudrate:**

   ```cpp
   // Di config.h, pastikan:
   int BAUDRATE_RS485 = 9600;
   ```

2. **Cek Address Sensor:**

   - Gunakan software sensor untuk memastikan address
   - Front magnet harus address 1
   - Back magnet harus address 4

3. **Cek Wiring:**

   ```
   ESP32 Pin 17 (TX) -> RS485 Module DI
   ESP32 Pin 18 (RX) -> RS485 Module RO
   ESP32 Pin 36      -> RS485 Module DE & RE
   RS485 A           -> Sensor A+
   RS485 B           -> Sensor B-
   ```

4. **Test Individual Sensor:**
   - Upload kode MAGNETIC.ino untuk test sensor individual
   - Pastikan sensor berfungsi sebelum integrasi

### 2. Ultrasonic Sensor Tidak Terbaca

**Gejala:**

- Device status menunjukkan "UF:ERR" atau "UB:ERR"
- Tidak ada data jarak yang diterima
- CRC checksum error

**Kemungkinan Penyebab:**

- Address ultrasonic tidak sesuai (Front=2, Back=3)
- Format data packet tidak sesuai
- Interference pada komunikasi serial

**Solusi:**

1. **Cek Address Ultrasonic:**

   - Front ultrasonic: address 2
   - Back ultrasonic: address 3

2. **Monitor Packet Data:**
   - Aktifkan debug di processUltrasonicData()
   - Cek format packet: [Address][0x03][Data...][CRC]

### 3. Device Switching Terlalu Cepat

**Gejala:**

- Communication timeout frequent
- Data sensor tidak stabil
- Error komunikasi intermittent

**Solusi:**

1. **Perlambat Device Switching:**

   ```cpp
   // Di unified_rs485.ino, ubah:
   const unsigned long deviceSwitchInterval = 200; // dari 100ms ke 200ms
   ```

2. **Tambah Delay Komunikasi:**
   ```cpp
   // Sudah ditambahkan delay(10) di communicateWithMagnet functions
   ```

### 4. Debugging Steps

**Step 1: Monitor Serial Output**

```
=== SETUP UNIFIED RS485 SYSTEM ===
RS485 control pins initialized
Serial1 initialized at baudrate: 9600
Magnet Front Modbus initialized with address: 1
Magnet Back Modbus initialized with address: 4
Sensor data arrays initialized
=== UNIFIED RS485 SETUP COMPLETE ===
```

**Step 2: Monitor Device Communication**

```
Communicating with: Magnet Front
Magnet Front - Median: 1234, Position: 0xFFFF
Communicating with: Ultrasonic Front
Communicating with: Ultrasonic Back
Communicating with: Magnet Back
Magnet Back - Median: 5678, Position: 0x0000
```

**Step 3: Check Device Status**

```cpp
// Tambahkan di loop utama untuk monitoring:
Serial.println(getDeviceStatusString());
// Output: "Devices: MF:OK UF:OK UB:ERR MB:OK"
```

### 5. Error Codes Modbus

| Error Code | Deskripsi            | Solusi                        |
| ---------- | -------------------- | ----------------------------- |
| 0x01       | Illegal Function     | Cek function code Modbus      |
| 0x02       | Illegal Data Address | Cek register address (0x0000) |
| 0x03       | Illegal Data Value   | Cek jumlah register (2)       |
| 0x04       | Slave Device Failure | Cek power dan koneksi sensor  |
| 0xE0       | Invalid Slave ID     | Cek address sensor            |
| 0xE1       | Invalid Function     | Cek Modbus function           |
| 0xE2       | Response Timed Out   | Cek baudrate dan wiring       |
| 0xE3       | Invalid CRC          | Cek integritas data           |

### 6. Optimasi Performa

**Untuk Mengurangi Latency:**

1. Kurangi device switch interval untuk sensor aktif saja
2. Prioritaskan magnet front untuk line following
3. Gunakan interrupt untuk ultrasonic data

**Untuk Stabilitas:**

1. Tambah termination resistor 120Ω di ujung kabel RS485
2. Gunakan twisted pair cable untuk RS485
3. Pisahkan power supply sensor dari ESP32

### 7. Konfigurasi Hardware

**RS485 Module:**

- VCC: 3.3V atau 5V
- GND: Ground
- DI: Data Input (dari ESP32 TX)
- RO: Receive Output (ke ESP32 RX)
- DE: Driver Enable
- RE: Receiver Enable (gabung dengan DE)
- A+: RS485 A line
- B-: RS485 B line

**Sensor Magnet CCF-NS16-4M:**

- Power: 12-24V DC
- A+: RS485 A line
- B-: RS485 B line
- Default Address: 1
- Default Baudrate: 9600

### 8. Test Procedure

1. **Test Individual Components:**

   - Test magnet sensor dengan MAGNETIC.ino
   - Test ultrasonic sensor individual
   - Test RS485 module dengan simple echo

2. **Test Integrated System:**

   - Upload unified system
   - Monitor serial output
   - Check device status
   - Verify sensor data

3. **Performance Test:**
   - Test line following accuracy
   - Test obstacle detection
   - Test device switching timing
   - Test error recovery

### 9. Kode Debug Tambahan

**Untuk monitoring real-time:**

```cpp
// Tambahkan di loop utama:
void debugSensorStatus() {
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 1000) {
    lastDebug = millis();

    Serial.println("=== SENSOR STATUS ===");
    Serial.println(getDeviceStatusString());

    uint8_t* magnetData = getCurrentMagnetData();
    Serial.print("Active segments: ");
    for (int i = 0; i < 16; i++) {
      if (magnetData[i]) {
        Serial.print(i+1);
        Serial.print(" ");
      }
    }
    Serial.println();

    Serial.print("Error value: ");
    Serial.println(errorValue);
    Serial.println("=====================");
  }
}
```

**Untuk disable debug output:**

```cpp
// Comment out semua Serial.print di unified_rs485.ino
// untuk mengurangi overhead komunikasi
```

### 10. Kontak Support

Jika masalah masih berlanjut:

1. Capture serial output lengkap
2. Dokumentasikan konfigurasi hardware
3. Test dengan kode individual sensor
4. Periksa power supply dan grounding
5. Gunakan oscilloscope untuk analisis sinyal RS485
