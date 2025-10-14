// test_auto_tuner.ino
// File ini adalah contoh kode untuk testing auto-tuner secara manual
// Tidak perlu dicompile dengan project utama

/*
CARA TESTING AUTO-TUNER:

1. Upload kode lengkap ke ESP32 Motor Controller Slave
2. Buka Serial Monitor (115200 baud)
3. Letakkan AGV di atas tumpuan agar roda berputar bebas
4. Ketik command berikut:

BASIC TESTING:
- help          -> Lihat semua command
- status        -> Cek PID saat ini dan status tuning
- tune          -> Mulai auto-tuning (estimasi 3.5 menit)
- cancel        -> Batalkan tuning jika sedang berjalan

MONITOR PROGRESS:
Selama tuning berjalan, Anda akan melihat output seperti:
```
======================================
Memulai siklus tuning 5/30
Testing: Kp=18.5000, Ki=0.6000, Kd=0.1500
t=500ms, RPM=8.2, Target=40, Error=31.8
t=1000ms, RPM=32.1, Target=40, Error=7.9
t=1500ms, RPM=41.3, Target=40, Error=1.3
...
Hasil siklus:
  - Overshoot: 3.25%
  - Rise Time: 1350 ms
  - Avg Error: 6.14 RPM
Skor: 28.95 (Terbaik sejauh ini: 28.95)
>>> DITEMUKAN PARAMETER TERBAIK BARU! <<<
```

TESTING DARI MASTER ESP32:
Dari ESP32 Master (AGV SAMI), kirim via Serial1:
- AUTOTUNE      -> Mulai tuning
- TUNESTATUS    -> Cek progress
- TUNECANCEL    -> Batalkan tuning

EXPECTED RESULTS:
Setelah tuning selesai, Anda harus melihat:
1. Parameter PID baru yang dioptimalkan
2. Konfirmasi penyimpanan ke Preferences
3. Respon motor yang lebih smooth dan cepat

TROUBLESHOOTING:
- Jika motor tidak bergerak: Cek koneksi dan encoder
- Jika tuning terlalu lama: Bisa berhenti manual dengan 'cancel'
- Jika hasil buruk: Coba tuning ulang atau cek kondisi mekanis

VALIDASI HASIL:
Setelah tuning, tes manual dengan:
1. Reset ESP32
2. Cek apakah parameter tersimpan dengan 'status'
3. Tes gerak motor dengan command RPM
4. Bandingkan smoothness sebelum vs sesudah tuning
*/

// Fungsi untuk validasi hasil tuning
void validateTuningResults() {
  Serial.println("=== VALIDASI HASIL TUNING ===");
  Serial.printf("PID Optimal: Kp=%.4f, Ki=%.4f, Kd=%.4f\n", 
                pidConfig.kp, pidConfig.ki, pidConfig.kd);
  
  // Test respon motor dengan target RPM berbeda
  int testRPMs[] = {20, 40, 60, -30};
  
  for(int i = 0; i < 4; i++) {
    Serial.printf("\nTesting %d RPM...\n", testRPMs[i]);
    rpmMotor(testRPMs[i], testRPMs[i]);
    
    delay(3000); // Tunggu 3 detik
    
    float avgRPM = getAverageRPM();
    float error = abs(testRPMs[i] - avgRPM);
    
    Serial.printf("Target: %d, Actual: %.1f, Error: %.1f\n", 
                  testRPMs[i], avgRPM, error);
    
    if(error < 5.0) {
      Serial.println("✓ BAGUS - Error < 5 RPM");
    } else if(error < 10.0) {
      Serial.println("⚠ CUKUP - Error 5-10 RPM");
    } else {
      Serial.println("✗ PERLU TUNING ULANG - Error > 10 RPM");
    }
    
    rpmMotor(0, 0); // Stop motor
    delay(1000);
  }
  
  Serial.println("\n=== VALIDASI SELESAI ===");
}

// Fungsi untuk benchmark performa PID
void benchmarkPID() {
  Serial.println("=== BENCHMARK PERFORMA PID ===");
  
  unsigned long startTime = millis();
  rpmMotor(40, 40);
  
  float maxOvershoot = 0;
  unsigned long riseTime = 0;
  bool crossed90pct = false;
  
  while(millis() - startTime < 5000) {
    float rpm = getAverageRPM();
    
    if(rpm > 40) {
      float overshoot = ((rpm - 40) / 40) * 100;
      if(overshoot > maxOvershoot) maxOvershoot = overshoot;
    }
    
    if(!crossed90pct && rpm >= 36) { // 90% of 40
      riseTime = millis() - startTime;
      crossed90pct = true;
    }
    
    delay(50);
  }
  
  rpmMotor(0, 0);
  
  Serial.printf("Benchmark Results:\n");
  Serial.printf("- Rise Time: %lu ms\n", riseTime);
  Serial.printf("- Max Overshoot: %.2f%%\n", maxOvershoot);
  
  // Scoring (sama seperti auto-tuner)
  float score = (maxOvershoot * 3.0) + (riseTime * 0.01);
  Serial.printf("- Performance Score: %.2f (lower is better)\n", score);
  
  if(score < 30) {
    Serial.println("🎉 EXCELLENT - PID tuning sangat bagus!");
  } else if(score < 50) {
    Serial.println("👍 GOOD - PID tuning bagus");
  } else if(score < 80) {
    Serial.println("👌 FAIR - PID tuning cukup, masih bisa diperbaiki");
  } else {
    Serial.println("🔧 POOR - Perlu tuning ulang");
  }
}
