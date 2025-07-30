// hook mode : naik dan turun dengan SSR relay dan limit switches
// CATATAN PENTING: SSR-40 DA hanya mengontrol ON/OFF motor, bukan arah!
// 
// SOLUSI SEMENTARA UNTUK TESTING:
// 1. Hubungkan motor hook ke SSR untuk satu arah saja (misal: NAIK)
// 2. Test dengan hook("test_5s") untuk memastikan SSR dan motor bekerja
// 3. Jika berhasil, tambahkan relay DPDT untuk kontrol arah
//
// SOLUSI PERMANEN:
// - Gunakan 2 SSR: satu untuk NAIK, satu untuk TURUN
// - Atau gunakan relay DPDT + 1 SSR untuk kontrol arah dan ON/OFF
// - Atau ganti dengan motor driver H-bridge (L298N, dll)
//
// PIN ASSIGNMENT SAAT INI:
// - Pin 21: SSR control (HIGH = motor ON, LOW = motor OFF)
// - Pin 20: Limit switch atas (LOW = tertekan)
// - Pin 19: Limit switch bawah (LOW = tertekan)

// Variabel untuk status hook motor
bool hookMotorRunning = false;
int hookDirection = 0; // 0 = berhenti, 1 = naik, -1 = turun

// Enum untuk status operasi hook
enum HookState {
  HOOK_IDLE,          // Diam, menunggu perintah
  HOOK_MOVING_UP,     // Sedang bergerak naik
  HOOK_AT_TOP,        // Berhenti di posisi atas
  HOOK_MOVING_DOWN,   // Sedang bergerak turun
  HOOK_AT_BOTTOM      // Berhenti di posisi bawah
};

HookState currentHookState = HOOK_IDLE;
unsigned long hookStateChangeTime = 0;
const long hookDelayAtLimit = 2000; // Jeda 2 detik di posisi limit

// Fungsi untuk menggerakkan hook ke atas
void moveHookUp() {
  bool limitUpActive = (digitalRead(LIMIT_SWITCH_UP_PIN) == HIGH); // HIGH = switch tertrigger
  
  if (!limitUpActive && hookDirection != 1) {
    // Pastikan pin dalam mode output
    pinMode(HOOK_RELAY_PIN, OUTPUT);
    
    // LOW untuk mengaktifkan relay SSR
    digitalWrite(HOOK_RELAY_PIN, LOW); // Aktifkan relay SSR
    
    hookDirection = 1;
    hookMotorRunning = true;
  } else if (limitUpActive) {
    stopHook();
  }
}

// Fungsi untuk menggerakkan hook ke bawah
void moveHookDown() {
  bool limitDownActive = (digitalRead(LIMIT_SWITCH_DOWN_PIN) == HIGH); // HIGH = switch tertrigger
  
  if (!limitDownActive && hookDirection != -1) {
    // Pastikan pin dalam mode output
    pinMode(HOOK_RELAY_PIN, OUTPUT);
    
    // LOW untuk mengaktifkan relay SSR
    digitalWrite(HOOK_RELAY_PIN, LOW); // Aktifkan relay SSR
    
    hookDirection = -1;
    hookMotorRunning = true;
  } else if (limitDownActive) {
    stopHook();
  }
}

// Fungsi untuk menghentikan hook
void stopHook() {
  if (hookMotorRunning) {
    // HIGH untuk mematikan relay SSR
    digitalWrite(HOOK_RELAY_PIN, HIGH); // Matikan relay SSR
    hookDirection = 0;
    hookMotorRunning = false;
  }
}

// Fungsi utama hook dengan mode string
void hook(String mode) {
  if (mode == "debug") {
    debugHookStatus();
    return;
  }
  
  if (mode == "test_on") {
    testHookRelay(true);
    return;
  }
  
  if (mode == "test_off") {
    testHookRelay(false);
    return;
  }
  
  if (mode == "test_5s") {
    testHookRelayTimed(5);
    return;
  }
  
  if (mode == "test_10s") {
    testHookRelayTimed(10);
    return;
  }
  
  if (mode == "check_pins") {
    checkPinConflicts();
    return;
  }
  
  // Apply hook inversion if enabled
  String actualMode = mode;
  if (invertHook) {
    if (mode == "naik") {
      actualMode = "turun";
    } else if (mode == "turun") {
      actualMode = "naik";
    }
  }

  // Reset ke mode manual jika ada perintah baru
  currentHookState = HOOK_IDLE;

  if (actualMode == "naik") {
    moveHookUp();
  } else if (actualMode == "turun") {
    moveHookDown();
  } else if (actualMode == "stop") {
    stopHook();
  } else {
    // Serial.println("Command hook tidak dikenal: " + mode);
    // Serial.println("Available commands: naik, turun, stop, debug, test_on, test_off, test_5s, test_10s, check_pins");
    // Serial.println("\nUNTUK TROUBLESHOOTING:");
    // Serial.println("1. hook(\"debug\") - Lihat status lengkap");
    // Serial.println("2. hook(\"test_5s\") - Test SSR 5 detik");
    // Serial.println("3. Periksa wiring SSR dan motor");
    // Serial.println("4. Pastikan power supply motor mencukupi");
  }
}

// Fungsi untuk memulai siklus otomatis hook
void startHookAutoCycle() {
  // Serial.println("Hook: Memulai siklus otomatis.");
  bool limitUpActive = (digitalRead(LIMIT_SWITCH_UP_PIN) == HIGH);
  bool limitDownActive = (digitalRead(LIMIT_SWITCH_DOWN_PIN) == HIGH);
  
  if (limitDownActive) {
    currentHookState = HOOK_AT_BOTTOM;
    hookStateChangeTime = millis();
  } else if (limitUpActive) {
    currentHookState = HOOK_AT_TOP;
    hookStateChangeTime = millis();
  } else {
    moveHookDown();
    currentHookState = HOOK_MOVING_DOWN;
  }
}

// Fungsi untuk update status hook (dipanggil di loop utama)
void updateHookStatus() {
  bool limitUpActive = (digitalRead(LIMIT_SWITCH_UP_PIN) == HIGH);   // Konsisten: HIGH = aktif
  bool limitDownActive = (digitalRead(LIMIT_SWITCH_DOWN_PIN) == HIGH); // Konsisten: HIGH = aktif

  // Safety check: stop motor jika limit switch aktif
  if (hookDirection == 1 && limitUpActive) {
    stopHook();
    if (currentHookState == HOOK_MOVING_UP) {
      currentHookState = HOOK_AT_TOP;
      hookStateChangeTime = millis();
    }
  } else if (hookDirection == -1 && limitDownActive) {
    stopHook();
    if (currentHookState == HOOK_MOVING_DOWN) {
      currentHookState = HOOK_AT_BOTTOM;
      hookStateChangeTime = millis();
    }
  }

  // Logika untuk mode otomatis
  switch (currentHookState) {
    case HOOK_IDLE:
      // Tidak melakukan apa-apa
      break;

    case HOOK_MOVING_UP:
    case HOOK_MOVING_DOWN:
      // Motor sedang bergerak, tunggu sampai limit switch aktif
      break;

    case HOOK_AT_TOP:
      if (millis() - hookStateChangeTime >= hookDelayAtLimit) {
        moveHookDown();
        currentHookState = HOOK_MOVING_DOWN;
      }
      break;

    case HOOK_AT_BOTTOM:
      if (millis() - hookStateChangeTime >= hookDelayAtLimit) {
        moveHookUp();
        currentHookState = HOOK_MOVING_UP;
      }
      break;
  }
}

// Fungsi untuk mendapatkan status posisi hook
String getHookPosition() {
  bool limitUpActive = (digitalRead(LIMIT_SWITCH_UP_PIN) == HIGH);
  bool limitDownActive = (digitalRead(LIMIT_SWITCH_DOWN_PIN) == HIGH);
  
  if (limitUpActive && limitDownActive) {
    return "ERROR: Kedua limit switch aktif!";
  } else if (limitUpActive) {
    return "ATAS";
  } else if (limitDownActive) {
    return "BAWAH";
  } else if (hookMotorRunning) {
    return hookDirection == 1 ? "NAIK" : "TURUN";
  } else {
    return "TENGAH";
  }
}

// Fungsi debug untuk troubleshooting hook
void debugHookStatus() {
  bool limitUpActive = (digitalRead(LIMIT_SWITCH_UP_PIN) == HIGH);
  bool limitDownActive = (digitalRead(LIMIT_SWITCH_DOWN_PIN) == HIGH);
  bool relayStatus = digitalRead(HOOK_RELAY_PIN);
  
  // Serial.println("=== HOOK DEBUG STATUS ===");
  // Serial.print("Limit Up Pin ("); Serial.print(LIMIT_SWITCH_UP_PIN); Serial.print("): ");
  // Serial.print(digitalRead(LIMIT_SWITCH_UP_PIN)); Serial.print(" -> ");
  // Serial.println(limitUpActive ? "AKTIF" : "TIDAK AKTIF");
  
  // Serial.print("Limit Down Pin ("); Serial.print(LIMIT_SWITCH_DOWN_PIN); Serial.print("): ");
  // Serial.print(digitalRead(LIMIT_SWITCH_DOWN_PIN)); Serial.print(" -> ");
  // Serial.println(limitDownActive ? "AKTIF" : "TIDAK AKTIF");
  
  // Serial.print("Relay Pin ("); Serial.print(HOOK_RELAY_PIN); Serial.print("): ");
  // Serial.println(relayStatus ? "ON" : "OFF");
  
  // Serial.print("Hook Motor Running: "); Serial.println(hookMotorRunning ? "YES" : "NO");
  // Serial.print("Hook Direction: ");
  // if (hookDirection == 1) Serial.println("NAIK");
  // else if (hookDirection == -1) Serial.println("TURUN");
  // else Serial.println("STOP");
  
  // Serial.print("Hook State: ");
  // switch(currentHookState) {
  //   case HOOK_IDLE: Serial.println("IDLE"); break;
  //   case HOOK_MOVING_UP: Serial.println("MOVING_UP"); break;
  //   case HOOK_AT_TOP: Serial.println("AT_TOP"); break;
  //   case HOOK_MOVING_DOWN: Serial.println("MOVING_DOWN"); break;
  //   case HOOK_AT_BOTTOM: Serial.println("AT_BOTTOM"); break;
  // }
  
  // Serial.println("\n=== TROUBLESHOOTING GUIDE ===");
  
  // Cek masalah limit switch
  if (limitUpActive && limitDownActive) {
    // Serial.println("ERROR: Kedua limit switch aktif! Periksa wiring.");
  } else if (limitUpActive || limitDownActive) {
    // Serial.println("INFO: Ada limit switch yang aktif, motor tidak akan bergerak.");
    // Serial.println("      Pindahkan hook secara manual jika perlu.");
  } else {
    // Serial.println("OK: Limit switch dalam kondisi normal.");
  }
  
  // Panduan troubleshooting SSR
  // Serial.println("\n--- PANDUAN TROUBLESHOOTING SSR ---");
  // Serial.println("1. LANGKAH PERTAMA - Test SSR:");
  // Serial.println("   a) Jalankan: hook(\"test_on\")");
  // Serial.println("   b) Periksa LED indikator SSR menyala");
  // Serial.println("   c) Dengar bunyi klik dari SSR");
  // Serial.println("   d) Jalankan: hook(\"test_off\")");
  // Serial.println("");
  // Serial.println("2. PERIKSA WIRING SSR-40 DA:");
  // Serial.println("   - Pin 3-4: Input control (3.3V dari ESP32 pin 21)");
  // Serial.println("   - Pin 1-2: Output AC (ke motor hook)");
  // Serial.println("   - Pastikan polaritas input control benar");
  // Serial.println("");
  // Serial.println("3. GUNAKAN MULTIMETER:");
  // Serial.println("   - Ukur tegangan pin 21 ESP32: harus 3.3V saat ON");
  // Serial.println("   - Ukur tegangan input SSR: harus 3.3V saat ON");
  // Serial.println("   - Ukur kontinuitas output SSR saat ON");
  // Serial.println("");
  // Serial.println("4. PERIKSA MOTOR DAN POWER:");
  // Serial.println("   - Pastikan motor hook terhubung ke output SSR");
  // Serial.println("   - Pastikan power supply motor mencukupi");
  // Serial.println("   - Test motor langsung tanpa SSR");
  // Serial.println("");
  // Serial.println("5. TEST COMMANDS:");
  // Serial.println("   - hook(\"test_5s\") untuk test 5 detik");
  // Serial.println("   - hook(\"test_10s\") untuk test 10 detik");
  // Serial.println("   - hook(\"debug\") untuk status lengkap");
  
  // Serial.println("\n--- MASALAH KONTROL ARAH ---");
  // Serial.println("PENTING: SSR-40 DA hanya ON/OFF, tidak mengontrol arah!");
  // Serial.println("Untuk kontrol arah naik/turun, diperlukan:");
  // Serial.println("- Relay DPDT tambahan, atau");
  // Serial.println("- Kontaktor dengan kontrol arah, atau");
  // Serial.println("- Motor driver H-bridge");
  
  // Serial.println("=========================");
}

// Fungsi untuk test manual relay (untuk troubleshooting)
void testHookRelay(bool state) {
  // Serial.print("Testing relay: ");
  // Serial.println(state ? "ON" : "OFF");
  
  // Set pin sebagai output jika belum
  pinMode(HOOK_RELAY_PIN, OUTPUT);
  
  // Tulis sinyal ke relay
  digitalWrite(HOOK_RELAY_PIN, state ? HIGH : LOW);
  
  // Baca kembali status pin untuk verifikasi
  bool actualState = digitalRead(HOOK_RELAY_PIN);
  // Serial.print("Pin "); Serial.print(HOOK_RELAY_PIN); 
  // Serial.print(" actual state: "); Serial.println(actualState ? "HIGH" : "LOW");
  
  // Cek apakah pin benar-benar berubah
  if (actualState != state) {
    // Serial.println("WARNING: Pin state tidak sesuai yang diharapkan!");
    // Serial.println("Kemungkinan masalah:");
    // Serial.println("- Pin conflict dengan komponen lain");
    // Serial.println("- Hardware problem pada ESP32");
    // Serial.println("- Wiring problem");
  } else {
    // Serial.println("OK: Pin state sesuai yang diharapkan");
  }
  
  // Delay untuk stabilitas dan observasi
  delay(500);
}

// Fungsi untuk mengecek konflik pin
void checkPinConflicts() {
  // Serial.println("=== CEK KONFLIK PIN ===");
  // Serial.print("Hook Relay Pin: "); Serial.println(HOOK_RELAY_PIN);
  // Serial.print("Limit Up Pin: "); Serial.println(LIMIT_SWITCH_UP_PIN);
  // Serial.print("Limit Down Pin: "); Serial.println(LIMIT_SWITCH_DOWN_PIN);
  
  // Serial.println("\nPin yang digunakan komponen lain:");
  // Serial.println("- Pin 7, 15, 16, 14: Music relay");
  // Serial.println("- Pin 12, 13: RFID");
  // Serial.println("- Pin 48, 45, 4, 5, 35, 6: Motor L298N");
  // Serial.println("- Pin 3, 8, 18, 17, 11, 10, 9, 46: Sensor");
  
  // Serial.println("\nKESIMPULAN: Pin 21, 20, 19 tidak konflik dengan komponen lain.");
  // Serial.println("==========================");
}

// Fungsi untuk test relay dengan durasi tertentu
void testHookRelayTimed(int seconds) {
  pinMode(HOOK_RELAY_PIN, OUTPUT);
  digitalWrite(HOOK_RELAY_PIN, LOW);  // LOW untuk mengaktifkan
  
  for(int i = seconds; i > 0; i--) {
    delay(1000);
  }
  
  digitalWrite(HOOK_RELAY_PIN, HIGH); // HIGH untuk mematikan
}