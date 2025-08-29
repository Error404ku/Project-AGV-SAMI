# Perbaikan Slave ESP32 Motor Controller

## Masalah yang Diperbaiki

1. **Overflow Nilai Error PID**
   - Sebelumnya nilai error bisa mencapai `-805306368, 1077843260` yang menunjukkan overflow dalam kalkulasi
   - Diperbaiki dengan penambahan `constrain()` pada nilai error dan integral
   
2. **Tipe Data yang Tidak Konsisten**
   - Penggunaan format `%d` untuk menampilkan nilai double pada Serial.print
   - Diperbaiki dengan penggunaan `%.2f` dan `Serial.printf()` untuk format yang benar

3. **Nilai PID yang Terlalu Agresif**
   - Nilai default `KP=1.0, KI=0.15` terlalu tinggi yang menyebabkan osilasi
   - Diganti dengan nilai yang lebih konservatif: `KP=0.5, KI=0.01`

4. **Penanganan Watchdog Timer**
   - Sebelumnya ada error `task_wdt: esp_task_wdt_reset(707): task not found`
   - Menambahkan `esp_task_wdt_reset()` pada loop utama

## Detail Perbaikan

### 1. File `pid.ino`
- Batasan nilai error yang lebih ketat (-50 sampai +50) untuk mencegah overflow
- Validasi nilai input dan output untuk mencegah NaN/Infinity
- Penambahan filter sederhana pada derivative untuk mengurangi noise
- Pemisahan perhitungan P, I, D untuk validasi masing-masing term
- Reset integral saat error kecil untuk mencegah osilasi

### 2. File `rpm.ino`
- Validasi nilai RPM input dan output dengan batasan yang lebih ketat
- Penambahan reset PID otomatis saat error terlalu besar
- Format tampilan nilai error dengan benar (%.2f)
- Pengurangan nilai target RPM maksimum (90% dari max) untuk keamanan

### 3. File `preferences.ino`
- Penggunaan nilai default PID yang lebih konservatif (KP=0.5, KI=0.01)
- Validasi nilai PID dengan constrain() untuk mencegah nilai ekstrim
- Penambahan fungsi resetPID() untuk reset PID controller individual

### 4. File `ESP32_Motor_Controller_Slave.ino`
- Penambahan esp_task_wdt_reset() di loop utama
- Peningkatan timeout watchdog timer menjadi 5 detik

## Rekomendasi Penggunaan

1. **Tuning PID**
   - Mulai dengan nilai KP dan KI yang kecil (sesuai default baru)
   - Naikkan KP secara bertahap sampai respon cukup cepat tanpa osilasi
   - Tambahkan KI secara perlahan untuk mengurangi steady-state error
   - KD bisa tetap 0 jika tidak diperlukan

2. **Monitoring**
   - Perhatikan nilai error pada Serial Monitor, pastikan tidak melebihi ±50
   - Jika error masih besar, pertimbangkan untuk menurunkan nilai KP

3. **Batasan RPM**
   - Jika motor masih tidak stabil, kurangi nilai maxrpm di config.h

## Status Perbaikan

- [x] Perbaiki overflow pada nilai error
- [x] Perbaiki format tampilan nilai pada Serial Monitor
- [x] Turunkan nilai default KP dan KI
- [x] Tambahkan penanganan Watchdog Timer
- [x] Tambahkan reset PID otomatis
- [x] Tambahkan batasan yang lebih ketat pada nilai-nilai PID

## Catatan Tambahan

Jika masih terjadi masalah, pertimbangkan untuk:
1. Menggunakan teknik filter tambahan pada pembacaan encoder
2. Menambahkan debouncing pada pembacaan encoder
3. Mengurangi frekuensi update PID (menambah intervalrpm)
4. Mengimplementasikan mekanisme anti-windup yang lebih sophisticated
