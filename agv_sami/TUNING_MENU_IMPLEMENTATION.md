# Implementasi Menu Tuning RPM - ESP32 Master (AGV SAMI)

## Fitur yang Telah Diimplementasikan

### 1. Menu "Tuning RPM" di Menu Utama

- Ditambahkan item menu baru "Tuning RPM" sebagai item ke-17 di menu utama
- Dapat diakses melalui navigasi UP/DOWN di menu utama
- Terintegrasi dengan sistem menu existing

### 2. Sub-Menu Tuning RPM

Terdiri dari 3 pilihan:

- **Start Tuning**: Memulai proses auto-tuning PID RPM di motor controller slave
- **Cancel Tuning**: Membatalkan proses tuning yang sedang berjalan
- **Status**: Melihat status dan progress tuning secara real-time

### 3. Komunikasi Serial dengan Motor Controller Slave

- Menggunakan protokol yang sudah ada di `motor_serial.ino`
- Mengirim command: `TUNE`, `CANCEL`, `TUNESTATUS`
- Menerima response: `AUTOTUNE:STARTED`, `AUTOTUNE:PROGRESS:xx`, `AUTOTUNE:CANCELLED`, dll.

### 4. Status Monitoring Real-time

- Auto-refresh status setiap 2 detik saat viewing status
- Progress bar visual untuk menunjukkan kemajuan tuning (0-100%)
- Status messages yang informatif dalam bahasa Indonesia

## File yang Dimodifikasi

### 1. `agv_sami/menu.h`

- Ditambah konstanta: `MENU_RPM_TUNING`, `MENU_RPM_TUNE_STATUS`, dll.
- Ditambah deklarasi fungsi: `displayRpmTuningMenu()`, `handleRpmTuningMenu()`, dll.

### 2. `agv_sami/config.h`

- Ditambah variabel: `selectedTuningItem`, `tuningStatus`, `tuningProgress`
- Ditambah konstanta: `TUNING_STATUS_INTERVAL`
- Ditambah deklarasi fungsi motor serial communication

### 3. `agv_sami/menu.ino`

- Diubah array `menuItems` dari 16 menjadi 17 item
- Ditambah case handling untuk selectedItem == 16 (Tuning RPM)
- Ditambah case `MENU_RPM_TUNING` dan `MENU_RPM_TUNE_STATUS` di switch statement
- Ditambah implementasi 4 fungsi baru:
  - `displayRpmTuningMenu()`
  - `handleRpmTuningMenu()`
  - `displayTuningStatus()`
  - `sendTuningCommand()`
  - `parseTuningResponse()`

### 4. `agv_sami/motor_serial.ino`

- Ditambah handling untuk message `AUTOTUNE:*` di `processMotorControllerMessage()`
- Memanggil `parseTuningResponse()` untuk mem-parse response dari slave

## Cara Penggunaan

### 1. Akses Menu Tuning

1. Dari menu utama, scroll down sampai "Tuning RPM"
2. Tekan tombol START/SELECT (A) untuk masuk

### 2. Memulai Tuning

1. Pilih "Start Tuning" (default selected)
2. Tekan START/SELECT (A)
3. Layar akan menunjukkan "Memulai tuning... Mohon tunggu..."
4. Kembali ke menu otomatis setelah 2 detik

### 3. Monitor Progress

1. Pilih "Status" di menu tuning
2. Tekan START/SELECT (A)
3. Layar akan menunjukkan:
   - Status tuning (IDLE/STARTED/PROGRESS/CANCELLED)
   - Progress percentage (jika sedang berjalan)
   - Progress bar visual
4. Status auto-refresh setiap 2 detik
5. Tekan STOP/BACK (B) untuk kembali ke menu tuning

### 4. Membatalkan Tuning

1. Pilih "Cancel Tuning" di menu tuning
2. Tekan START/SELECT (A)
3. Layar akan menunjukkan "Membatalkan tuning..."

## Status Messages

- **IDLE**: Tuning tidak aktif
- **STARTED**: Tuning baru dimulai
- **PROGRESS**: Tuning sedang berjalan dengan progress xx%
- **CANCELLED**: Tuning dibatalkan
- **RUNNING**: Tuning sudah berjalan (jika mencoba start lagi)

## Protocol Communication

### Commands dari Master ke Slave:

```
TUNE        -> Mulai auto-tuning
CANCEL      -> Batalkan tuning
TUNESTATUS  -> Minta status tuning
```

### Responses dari Slave ke Master:

```
AUTOTUNE:STARTED          -> Tuning berhasil dimulai
AUTOTUNE:ALREADY_RUNNING  -> Tuning sudah berjalan
AUTOTUNE:CANCELLED        -> Tuning berhasil dibatalkan
AUTOTUNE:NOT_RUNNING      -> Tidak ada tuning yang berjalan
AUTOTUNE:IDLE             -> Tuning tidak aktif
AUTOTUNE:PROGRESS:xx      -> Progress tuning xx% (0-100)
```

## Integration dengan Sistem Existing

### 1. Menu System

- Menggunakan sistem menu existing tanpa breaking changes
- Compatible dengan semua fungsi navigasi yang ada
- Mengikuti pattern yang sama dengan menu lainnya

### 2. Serial Communication

- Menggunakan protokol komunikasi yang sudah ada
- Compatible dengan semua command existing
- Tidak mengganggu komunikasi PID dan RPM yang sudah ada

### 3. Display System

- Menggunakan LCD display functions yang sudah ada
- Compatible dengan refresh mechanism existing
- Mengikuti pattern displayHeader/displayFooter yang sama

## Testing & Validation

### Sebelum Testing:

1. Pastikan ESP32 Motor Controller Slave sudah diupload dengan kode auto-tuner
2. Pastikan koneksi serial antara Master dan Slave berfungsi normal
3. Letakkan AGV di atas tumpuan agar roda berputar bebas

### Testing Flow:

1. Upload kode Master yang sudah dimodifikasi
2. Akses menu "Tuning RPM" dari LCD
3. Test semua sub-menu (Start, Cancel, Status)
4. Verifikasi komunikasi serial di Serial Monitor
5. Validasi progress monitoring real-time

### Expected Results:

- Menu navigation smooth tanpa hang
- Command terkirim ke slave dengan benar
- Response dari slave ter-parse dengan benar
- Status display update sesuai progress tuning
- Kembali ke operasi normal setelah tuning selesai

## Troubleshooting

### Menu Tidak Muncul:

- Cek kompilasi tanpa error
- Pastikan semua file dimodifikasi dengan benar
- Restart ESP32 Master

### Command Tidak Terkirim:

- Cek koneksi Serial1 antara Master dan Slave
- Monitor Serial untuk debug messages
- Pastikan Slave sudah running dan siap menerima command

### Status Tidak Update:

- Cek parsing function berjalan dengan benar
- Pastikan format response dari Slave sesuai protokol
- Cek auto-refresh timing (2 detik interval)

---

**Catatan**: Implementasi ini fully backward compatible dan tidak mengubah fungsi existing system. Fitur tuning adalah tambahan yang independen dan dapat digunakan kapan saja tanpa mengganggu operasi AGV normal.
