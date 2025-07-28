# Panduan Debug RS485 System

## Overview

Sistem debug RS485 telah diintegrasikan ke dalam kode AGV untuk membantu mengidentifikasi dan mengatasi masalah komunikasi RS485. Sistem ini menyediakan monitoring real-time, statistik komunikasi, dan tools diagnostik.

## File yang Ditambahkan

1. **debug_rs485.ino** - Berisi semua fungsi debug dan monitoring
2. **DEBUG_RS485_GUIDE.md** - Panduan penggunaan (file ini)

## Fitur Debug

### 1. Monitoring Real-time
- Status komunikasi setiap device
- Statistik success rate
- Waktu komunikasi terakhir
- Data sensor real-time

### 2. Error Tracking
- Error codes Modbus dengan deskripsi
- CRC errors untuk ultrasonic
- Timeout tracking
- Packet analysis

### 3. Performance Monitoring
- Timing analysis
- Communication statistics
- Device switching monitoring

## Cara Menggunakan

### Setup Awal

1. **Upload kode** yang sudah dimodifikasi ke ESP32
2. **Buka Serial Monitor** dengan baudrate 115200
3. **Reset ESP32** untuk melihat output setup

### Output Setup Normal

```
=== RS485 DEBUG SYSTEM INITIALIZED ===
Debug flags:
- Basic Debug: ON
- Detailed Debug: OFF
- Packet Debug: OFF
- Timing Debug: OFF
==========================================

=== SETUP UNIFIED RS485 SYSTEM ===
RS485 control pins initialized
Serial1 initialized at baudrate: 9600
Magnet Front Modbus initialized with address: 1
Magnet Back Modbus initialized with address: 4
Sensor data arrays initialized
=== UNIFIED RS485 SETUP COMPLETE ===

=== DIAGNOSING COMMON ISSUES ===
Configured Baudrate: 9600
Device Addresses:
  Magnet Front: 1
  Ultrasonic Front: 2
  Ultrasonic Back: 3
  Magnet Back: 4
Pin Configuration:
  TX_RS485: 17
  RX_RS485: 18
  MAX485_DE: 36
  MAX485_RE: 36
Timing Configuration:
  Device Switch Interval: 100
  Communication Timeout: 5000
Diagnosis complete.
```

### Monitoring Normal Operation

Setiap 2 detik, sistem akan menampilkan:

```
=== COMMUNICATION STATISTICS ===
Magnet Front:
  Success Rate: 95.2% (40/42)
  Failed: 2
  Timeouts: 1

Magnet Back:
  Success Rate: 98.0% (49/50)
  Failed: 1

Ultrasonic Front:
  Success Rate: 92.3% (36/39)
  Failed: 3
  CRC Errors: 2

Ultrasonic Back:
  Success Rate: 94.1% (32/34)
  Failed: 2
  CRC Errors: 1
================================

=== DEVICE STATUS ===
Devices: MF:OK UF:OK UB:OK MB:OK
Last successful communication:
  Magnet Front: 150 ms ago
  Ultrasonic Front: 200 ms ago
  Ultrasonic Back: 180 ms ago
  Magnet Back: 120 ms ago
=====================
```

### Debug Commands

Kirim karakter berikut melalui Serial Monitor:

- **b** - Toggle Basic Debug (ON/OFF)
- **d** - Toggle Detailed Debug (sensor data real-time)
- **p** - Toggle Packet Debug (raw packet data)
- **t** - Toggle Timing Debug (communication timing)
- **r** - Reset Statistics
- **i** - Test Individual Devices
- **g** - Diagnose Common Issues

### Contoh Penggunaan Debug Commands

1. **Aktifkan Detailed Debug:**
   - Kirim: `d`
   - Output: `Detailed Debug: ON`
   - Sekarang akan menampilkan data sensor real-time

2. **Test Individual Devices:**
   - Kirim: `i`
   - Sistem akan test setiap device secara individual

3. **Reset Statistics:**
   - Kirim: `r`
   - Semua statistik komunikasi akan direset

## Interpretasi Output Debug

### 1. Communication Statistics

- **Success Rate**: Persentase komunikasi berhasil
- **Failed**: Jumlah komunikasi gagal
- **Timeouts**: Jumlah timeout (tidak ada response)
- **CRC Errors**: Kesalahan checksum (khusus ultrasonic)

### 2. Device Status

- **MF:OK/ERR** - Magnet Front status
- **UF:OK/ERR** - Ultrasonic Front status
- **UB:OK/ERR** - Ultrasonic Back status
- **MB:OK/ERR** - Magnet Back status

### 3. Error Messages

```
[ERROR] Magnet Front - Modbus Error: 0xE2 (Response Timed Out)
[CRC ERROR] Expected: 0x1234, Received: 0x5678
[PACKET ERROR] Invalid header: 02 04
```

## Troubleshooting Berdasarkan Output

### 1. Success Rate Rendah (<90%)

**Kemungkinan Penyebab:**
- Kabel RS485 longgar atau rusak
- Interference elektromagnetik
- Power supply tidak stabil
- Baudrate tidak sesuai

**Solusi:**
- Cek koneksi kabel
- Tambah termination resistor 120Ω
- Pisahkan power supply sensor
- Verifikasi baudrate sensor

### 2. Timeout Errors Frequent

**Kemungkinan Penyebab:**
- Sensor tidak terhubung
- Address sensor salah
- Baudrate tidak cocok

**Solusi:**
- Cek power sensor
- Verifikasi address dengan software sensor
- Test dengan baudrate berbeda

### 3. CRC Errors (Ultrasonic)

**Kemungkinan Penyebab:**
- Noise pada komunikasi serial
- Kabel terlalu panjang
- Grounding buruk

**Solusi:**
- Gunakan twisted pair cable
- Perbaiki grounding
- Kurangi panjang kabel

### 4. Device Switching Terlalu Cepat

**Gejala:**
- Communication errors intermittent
- Data tidak stabil

**Solusi:**
```cpp
// Di unified_rs485.ino, ubah:
const unsigned long deviceSwitchInterval = 200; // dari 100ms ke 200ms
```

## Optimasi Performa

### 1. Disable Debug di Production

```cpp
// Di debug_rs485.ino, set:
bool enableRS485Debug = false;
bool enableDetailedDebug = false;
bool enablePacketDebug = false;
bool enableTimingDebug = false;
```

### 2. Prioritas Device

Untuk line following, prioritaskan magnet front:

```cpp
// Modifikasi device switching untuk prioritas magnet front
if (currentDeviceIndex == 0) {
  // Magnet front - komunikasi lebih sering
  deviceSwitchInterval = 50;
} else {
  deviceSwitchInterval = 150;
}
```

### 3. Selective Monitoring

Aktifkan debug hanya untuk device bermasalah:

```cpp
// Contoh: debug hanya ultrasonic
if (currentDeviceIndex == 1 || currentDeviceIndex == 2) {
  enableDetailedDebug = true;
} else {
  enableDetailedDebug = false;
}
```

## Tips Debug Lanjutan

### 1. Oscilloscope Analysis

Untuk analisis sinyal RS485:
- Probe A+/B- lines
- Cek voltage levels (differential ±2V minimum)
- Analisis timing dan noise

### 2. Logic Analyzer

Untuk analisis protokol:
- Monitor TX/RX pins ESP32
- Decode Modbus frames
- Verify timing requirements

### 3. Isolated Testing

Test sensor individual:
- Disconnect dari bus RS485
- Test dengan USB-RS485 converter
- Verify dengan software sensor

## Kontak Support

Jika masalah masih berlanjut setelah menggunakan debug system:

1. **Capture complete serial output** selama minimal 30 detik
2. **Dokumentasikan hardware configuration**
3. **Test individual sensors** dengan software terpisah
4. **Cek power supply** dan grounding
5. **Gunakan oscilloscope** untuk analisis sinyal

## Changelog Debug System

### v1.0 (Current)
- Basic communication monitoring
- Error tracking dan statistics
- Interactive debug commands
- Real-time sensor data display
- Timing analysis
- Individual device testing
- Common issues diagnosis

### Planned Features
- Web-based debug interface
- Data logging ke SD card
- Remote debug via WiFi
- Automatic error recovery
- Performance optimization suggestions