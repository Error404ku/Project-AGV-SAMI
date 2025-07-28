# Menu RS485 Debug - Panduan Penggunaan

## Overview
Fitur Menu RS485 Debug telah ditambahkan ke sistem AGV untuk memudahkan troubleshooting dan monitoring komunikasi RS485 secara real-time melalui LCD dan tombol navigasi.

## Akses Menu Debug

1. **Masuk ke Menu Utama** - Tekan tombol STOP saat AGV mode aktif
2. **Navigasi ke "Magnet Check"** - Gunakan tombol UP/DOWN untuk scroll
3. **Pilih Menu** - Tekan tombol START untuk masuk ke "Magnet+RS485 Debug"

## Kontrol Debug via Tombol

### Tampilan LCD
```
Magnet+RS485 Debug
UP:BasicDbg     ON
DN:DetailDbg    OFF
LF:Rst RT:Tst B:Back
```

### Fungsi Tombol

| Tombol | Fungsi | Deskripsi |
|--------|--------|----------|
| **UP** | Toggle Basic Debug | Mengaktifkan/menonaktifkan debug dasar RS485 |
| **DOWN** | Toggle Detailed Debug | Mengaktifkan/menonaktifkan debug detail sensor |
| **LEFT** | Reset Statistics | Mereset semua statistik komunikasi |
| **RIGHT** | Test Individual Devices | Menjalankan test komunikasi per device |
| **STOP** | Back to Main Menu | Kembali ke menu utama |

## Status Indikator

### Basic Debug (UP)
- **ON**: Menampilkan status komunikasi real-time di Serial Monitor
- **OFF**: Debug output dinonaktifkan

### Detailed Debug (DOWN)
- **ON**: Menampilkan data sensor detail (nilai magnet, jarak ultrasonic)
- **OFF**: Hanya status komunikasi dasar

## Feedback Visual

### Status Toggle
- Status ON/OFF ditampilkan di kolom kanan LCD (posisi 15,1 dan 15,2)
- Update real-time saat tombol ditekan

### Action Messages
- **"Stats Reset!"** - Muncul selama 1 detik saat LEFT ditekan
- **"Testing Devices!"** - Muncul selama 1 detik saat RIGHT ditekan
- Pesan otomatis hilang dan kembali ke menu normal

## Output Serial Monitor

### Basic Debug Output
```
=== DEVICE STATUS ===
Devices: MF:OK UF:OK UB:ERR MB:OK
Last Communication Times:
  Magnet Front: 1234ms ago
  Ultrasonic Front: 567ms ago
  Ultrasonic Back: TIMEOUT
  Magnet Back: 890ms ago
```

### Detailed Debug Output
```
=== SENSOR DATA ===
Magnet Front: [1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0]
Active Segments: 1,3,5
Error Value: 2

Ultrasonic Front: [120,125,130,135,140] cm
Ultrasonic Back: [200,205,210,215,220] cm
Obstacle Status: Front=Clear, Back=Clear
```

### Statistics Output
```
=== COMMUNICATION STATISTICS ===
Magnet Front: 95.2% (476/500) - Timeouts: 24
Magnet Back: 98.1% (490/500) - Timeouts: 10
Ultrasonic Front: 92.3% (461/500) - CRC Errors: 15, Timeouts: 24
Ultrasonic Back: 89.7% (448/500) - CRC Errors: 28, Timeouts: 24
```

## Troubleshooting Berdasarkan Menu

### Success Rate Rendah (<90%)
1. **Aktifkan Basic Debug** (UP)
2. **Monitor output Serial** untuk error patterns
3. **Test Individual Devices** (RIGHT) untuk isolasi masalah
4. **Periksa hardware** jika device tertentu konsisten error

### Timeout Errors Frequent
1. **Periksa power supply** sensor
2. **Cek koneksi RS485** (A+, B-, GND)
3. **Verifikasi baudrate** dan address sensor

### CRC Errors (Ultrasonic)
1. **Periksa interference** elektromagnetik
2. **Gunakan kabel twisted pair** untuk RS485
3. **Tambah termination resistor** 120Ω

## Tips Penggunaan

### Monitoring Normal
1. **Aktifkan Basic Debug** untuk monitoring rutin
2. **Nonaktifkan Detailed Debug** untuk mengurangi overhead
3. **Reset Statistics** secara berkala untuk data fresh

### Troubleshooting Intensif
1. **Aktifkan semua debug modes**
2. **Test Individual Devices** untuk isolasi
3. **Monitor output Serial** untuk pattern analysis
4. **Dokumentasikan error patterns** untuk analisis

### Production Mode
1. **Nonaktifkan semua debug** untuk performa optimal
2. **Gunakan hanya saat diperlukan** troubleshooting
3. **Reset statistics** sebelum operasi normal

## Integrasi dengan Debug Commands

Menu ini terintegrasi dengan sistem debug command via Serial Monitor:

| Menu Action | Serial Command | Fungsi |
|-------------|----------------|--------|
| UP (Basic) | `b` | Toggle Basic Debug |
| DOWN (Detail) | `d` | Toggle Detailed Debug |
| LEFT (Reset) | `r` | Reset Statistics |
| RIGHT (Test) | `i` | Individual Device Test |

## Keunggulan Fitur

### User-Friendly
- **Interface LCD** yang mudah dibaca
- **Kontrol tombol** yang intuitif
- **Feedback visual** real-time

### Comprehensive Monitoring
- **Status komunikasi** semua device
- **Statistik success rate** per device
- **Error tracking** dengan detail

### Non-Intrusive
- **Tidak mengganggu** operasi normal AGV
- **Dapat diaktifkan/nonaktifkan** sesuai kebutuhan
- **Overhead minimal** saat debug dinonaktifkan

## Maintenance

### Regular Checks
- **Monitor success rate** mingguan
- **Reset statistics** bulanan
- **Test individual devices** saat ada masalah

### Performance Optimization
- **Nonaktifkan debug** saat tidak diperlukan
- **Monitor timing** untuk optimasi interval
- **Dokumentasikan** pattern error untuk improvement

## Changelog

### v1.0 - Initial Release
- Implementasi menu debug RS485 di LCD
- Integrasi dengan sistem debug existing
- Kontrol via tombol navigasi
- Feedback visual real-time
- Action messages dengan auto-clear
- Status indicator ON/OFF

---

**Catatan**: Fitur ini melengkapi sistem debug RS485 yang sudah ada dan memberikan akses mudah untuk troubleshooting tanpa perlu akses ke Serial Monitor.