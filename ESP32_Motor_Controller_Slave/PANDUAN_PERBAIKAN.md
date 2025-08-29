// Panduan Implementasi Perbaikan

Berikut langkah-langkah untuk mengimplementasikan perbaikan pada kode ESP32 Motor Controller Slave:

1. **Ubah definisi numOutputs pada config.h**
   - Buka file config.h
   - Ubah `#define numOutputs 5` menjadi `#define numOutputs 2`
   - Ini penting karena kode hanya menggunakan 2 indeks PID (0 dan 1)

2. **Perbaiki fungsi computePID di pid.ino**
   - Ganti isi fungsi computePID dengan fungsi computePID_fixed dari file pid_fixed.ino
   - Fungsi yang diperbaiki menambahkan debug output dan penanganan tipe data yang lebih baik

3. **Perbaiki fungsi rpmMotor di rpm.ino**
   - Ganti isi fungsi rpmMotor dengan fungsi rpmMotor_fixed dari file pid_fixed.ino
   - Fungsi baru membatasi nilai integral untuk mencegah overflow

4. **Perbarui fungsi setTargetRPM di rpm.ino**
   - Ganti untuk menggunakan rpmMotor yang sudah diperbaiki

5. **Perbarui panggilan ke setTargetRPM di serial.ino**
   - Tidak perlu mengubah nama fungsi jika Anda sudah memperbaiki fungsi aslinya

6. **Tambahkan penanganan watchdog timer yang lebih baik**
   - Di setup(), tambahkan:
     ```cpp
     // Initialize watchdog timer with longer timeout
     esp_task_wdt_init(5, true); // 5 second timeout
     esp_task_wdt_add(NULL); // Add current thread to WDT watch
     ```
   
   - Di loop(), tambahkan:
     ```cpp
     // Reset watchdog timer dalam loop utama
     esp_task_wdt_reset();
     ```

7. **Perbaiki tipe data dan print format di rpm.ino**
   - Pastikan menggunakan printf dengan format yang sesuai (%f untuk float/double)
   - Ubah `Serial.printf("error RPM - Kanan: %d, Kiri: %d\n", pidData[0].error, pidData[1].error);`
   - Menjadi `Serial.printf("Error RPM (double) - Kanan: %.2f, Kiri: %.2f\n", pidData[0].error, pidData[1].error);`

Penjelasan Masalah:
1. **Overflow pada Error PID**: 
   - Nilai error yang sangat besar (-805306368, 1077843260, dll.) menunjukkan adanya overflow pada tipe data
   - Pembatasan nilai integral dapat mencegah overflow ini
   
2. **Bug pada numOutputs**: 
   - Array pidData[numOutputs] didefinisikan dengan ukuran 5, tapi hanya indeks 0 dan 1 yang digunakan
   - Kode mungkin mengakses indeks yang tidak valid, menyebabkan perilaku tidak terduga
   
3. **Watchdog Timer Issues**: 
   - Pesan "task_wdt: esp_task_wdt_reset(707): task not found" menunjukkan kode mencoba mereset WDT untuk task yang tidak terdaftar
   - Menambahkan registrasi task yang benar dan reset WDT di loop utama akan mengatasi masalah ini
