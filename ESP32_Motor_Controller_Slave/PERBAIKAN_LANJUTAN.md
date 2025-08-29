# Perbaikan Lebih Lanjut

Berdasarkan pengamatan pada output serial monitor, masih terdapat beberapa masalah utama:

1. Format string untuk menampilkan error masih menggunakan "%d" (integer) di rpmMotor(), bukan "%.2f" (float)
2. Nilai error masih mengalami overflow/underflow dengan nilai seperti -1073741824, 1073741824, -2147483648
3. Masalah dengan watchdog timer masih terjadi meskipun kita telah menambahkan kode untuk reset

Berikut langkah-langkah perbaikan lebih lanjut:

## 1. Perbaiki format print error di rpm.ino

```cpp
// Ubah:
Serial.printf("error RPM - Kanan: %d, Kiri: %d\n", pidData[0].error, pidData[1].error);

// Menjadi:
Serial.printf("Error RPM (double) - Kanan: %.2f, Kiri: %.2f\n", pidData[0].error, pidData[1].error);
```

## 2. Tambahkan fungsi watchdog yang lebih efektif

Hapus panggilan `esp_task_wdt_reset(707)` yang menyebabkan error karena task 707 tidak ada.

```cpp
// Di ESP32_Motor_Controller_Slave.ino
#include "esp_task_wdt.h"

void setup() {
  // Tambahkan kode ini untuk mengatur WDT
  esp_task_wdt_init(10, false);  // 10 detik timeout, tidak reboot otomatis
  esp_task_wdt_add(NULL);  // Daftarkan task utama ke watchdog
  // ...kode lainnya...
}

void loop() {
  // Reset watchdog di awal loop utama
  esp_task_wdt_reset();
  // ...kode lainnya...
}
```

## 3. Batasi nilai output PID dengan lebih ketat

```cpp
// Di pid.ino
double computePID(int index, double setpoint, double input, double Kp, double Ki, double Kd, int Minintegral, double Maxintegral) {
  // ...kode lainnya...
  
  // Hitung error - pastikan tipe data double digunakan dengan benar
  double error = setpoint - input;
  
  // Validasi error - batasi nilai maksimum
  error = constrain(error, -100.0, 100.0);
  
  // Store error secara aman ke dalam pidData
  pidData[index].error = error;
  
  // ...kode lainnya...
}
```

## 4. Tambahkan reset saat error terlalu besar

```cpp
// Di rpm.ino
void rpmMotor(float rpm1, float rpm2) {
  // ...kode lainnya...
  
  // Reset PID jika nilai error terlalu besar
  if (fabs(pidData[0].error) > 100.0 || fabs(pidData[1].error) > 100.0) {
    pidData[0].integral = 0;
    pidData[1].integral = 0;
    pidData[0].error = 0;
    pidData[1].error = 0;
    Serial.println("WARNING: Error terlalu besar, mereset PID");
  }
  
  // ...kode lainnya...
}
```

## 5. Gunakan nilai konstanta PID yang lebih konservatif

```cpp
// Di preferences.ino
void loadPIDParameters() {
  preferences.begin("pid", false);
  
  // Gunakan nilai default yang lebih konservatif
  pidConfig.kp = preferences.getDouble("kp", 0.5);  // Default kp lebih kecil
  pidConfig.ki = preferences.getDouble("ki", 0.01); // Default ki lebih kecil
  pidConfig.kd = preferences.getDouble("kd", 0.0);  // Default kd nol

  preferences.end();
  
  // Tambahkan batas untuk nilai-nilai ini
  pidConfig.kp = constrain(pidConfig.kp, 0.0, 10.0);
  pidConfig.ki = constrain(pidConfig.ki, 0.0, 1.0);
  pidConfig.kd = constrain(pidConfig.kd, 0.0, 1.0);
}
```

## 6. Batasi RPM maksimal secara lebih ketat jika diperlukan

```cpp
// Di config.h
#define maxrpm 60    // Turunkan RPM maksimal dari 90 ke 60 jika motor terlalu kencang
```

Perbaikan-perbaikan ini akan membantu mencegah overflow pada nilai error dan membuat kontrol PID lebih stabil.
