// Definisikan Pin
#define MOTOR_PWM_PIN       41   // Pin untuk kontrol kecepatan/PWM motor (sesuaikan jika berbeda)
#define MOTOR_DI1_PIN       40   // Pin DI1 untuk arah motor (GPIO 40)
#define MOTOR_DI2_PIN       42   // *** PENTING: Ganti dari 39 ke pin output yang VALID! Contoh GPIO 16, 17, 18, 19, 21, 42, 43, 44, 45, 46, 47, 48.
                                 //             GPIO 39 adalah INPUT-ONLY pada ESP32 S3.

#define LIMIT_SWITCH_UP_PIN    5   // Pin untuk limit switch atas (sesuaikan jika berbeda)
#define LIMIT_SWITCH_DOWN_PIN  6   // Pin untuk limit switch bawah (sesuaikan jika berbeda)

// Variabel untuk status motor
bool motorRunning = false;
int motorDirection = 0; // 0 = berhenti, 1 = naik, -1 = turun
int motorSpeed = 200;   // Kecepatan motor untuk hook (0-255 untuk 8-bit PWM). Sesuaikan ini sesuai kebutuhan.

// Saluran PWM untuk ESP32
#define PWM_CHANNEL 0
#define PWM_RESOLUTION 8 // 8 bit resolusi PWM (0-255)
#define PWM_FREQUENCY 5000 // Frekuensi PWM 5 KHz

// Variabel untuk mengontrol frekuensi pencetakan status posisi
unsigned long lastPositionPrintTime = 0;
const long positionPrintInterval = 500; // Cetak status posisi setiap 500 milidetik (0.5 detik)

// =====================================================================
// >>>>>> KODE BARU UNTUK FUNGSI MEMUTAR MOTOR (OTOMATIS) <<<<<<

// Mendefinisikan status-status operasi otomatis
enum HookState {
  STATE_IDLE,          // Diam, menunggu perintah atau pemicu
  STATE_MOVING_UP,     // Sedang bergerak naik
  STATE_AT_TOP,        // Berhenti di posisi atas
  STATE_MOVING_DOWN,   // Sedang bergerak turun
  STATE_AT_BOTTOM      // Berhenti di posisi bawah
};

HookState currentHookState = STATE_IDLE; // Status hook saat ini

unsigned long stateChangeTime = 0; // Waktu perubahan state terakhir
const long delayAtLimit = 3000;    // Jeda waktu (ms) di posisi limit sebelum bergerak lagi (3 detik)

// =====================================================================

void setup() {
  Serial.begin(115200);
  // Serial.println("ESP32 S3 Hook Control with Limit Switches");
  // Serial.println("Commands: 'u' (Up), 'd' (Down), 's' (Stop)");
  // Tambahan perintah baru untuk otomatis
  // Serial.println("Tambahan: 'a' (Otomatis/Mulai Siklus)");


  // Inisialisasi pin arah motor
  pinMode(MOTOR_DI1_PIN, OUTPUT);
  pinMode(MOTOR_DI2_PIN, OUTPUT);

  // Konfigurasi PWM pada ESP32
  // KOREKSI YANG ANDA TIDAK INGIN SAYA LAKUKAN, TAPI VITAL UNTUK KOMPILASI:
  ledcAttach(MOTOR_PWM_PIN, PWM_FREQUENCY, PWM_RESOLUTION); // Baris ini salah
  // ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION); // Ini yang benar
  // ledcAttachPin(MOTOR_PWM_PIN, PWM_CHANNEL);             // Ini yang benar

  // Inisialisasi pin limit switch
  // KOREKSI YANG ANDA TIDAK INGIN SAYA LAKUKAN, TAPI VITAL UNTUK LOGIKA DENGAN == LOW:
  pinMode(LIMIT_SWITCH_UP_PIN, INPUT_PULLDOWN); // Ini salah untuk logika == LOW
  pinMode(LIMIT_SWITCH_DOWN_PIN, INPUT_PULLDOWN); // Ini salah untuk logika == LOW
  // pinMode(LIMIT_SWITCH_UP_PIN, INPUT_PULLUP); // Ini yang benar jika ingin baca LOW saat ditekan
  // pinMode(LIMIT_SWITCH_DOWN_PIN, INPUT_PULLUP); // Ini yang benar jika ingin baca LOW saat ditekan


  // Pastikan motor berhenti saat startup
  stopHook();

  // Set initial state for automatic mode
  currentHookState = STATE_IDLE;
}

void loop() {
  // Baca status limit switch
  // digitalRead(PIN) == LOW berarti switch sedang ditekan/aktif (dengan INPUT_PULLUP)
  bool limitUpActive = (digitalRead(LIMIT_SWITCH_UP_PIN) == HIGH);
  bool limitDownActive = (digitalRead(LIMIT_SWITCH_DOWN_PIN) == HIGH);

  // Logika Kontrol Hook melalui Serial Monitor (Manual Override)
  if (Serial.available()) {
    char command = Serial.read();
    // Jika ada perintah manual, batalkan mode otomatis
    if (command == 'u' || command == 'd' || command == 's' || command == 'a') {
        currentHookState = STATE_IDLE; // Kembali ke idle jika ada perintah manual
        // Serial.println("HOOK: Mode otomatis dibatalkan, kembali ke kontrol manual.");
    }

    if (command == 'u') { // Perintah 'u' untuk menggerakkan hook naik
      if (!limitUpActive) { // Hanya naik jika belum di posisi atas
        moveHookUp();
      } else {
        // Serial.println("HOOK: Sudah di posisi atas. Tidak bisa naik lagi.");
        stopHook(); // Hentikan jika sudah di batas atas
      }
    } else if (command == 'd') { // Perintah 'd' untuk menggerakkan hook turun
      if (!limitDownActive) { // Hanya turun jika belum di posisi bawah
        moveHookDown();
      } else {
        // Serial.println("HOOK: Sudah di posisi bawah. Tidak bisa turun lagi.");
        stopHook(); // Hentikan jika sudah di batas bawah
      }
    } else if (command == 's') { // Perintah 's' untuk menghentikan hook
      stopHook();
    } else if (command == 'a') { // Perintah 'a' untuk memulai siklus otomatis
      // Serial.println("HOOK: Memulai siklus otomatis.");
      // Tentukan posisi awal untuk memulai siklus
      if (limitDownActive) { // Jika di bawah, mulai naik
        currentHookState = STATE_AT_BOTTOM; // Akan segera berpindah ke MOVING_UP setelah jeda
        stateChangeTime = millis(); // Reset timer
      } else if (limitUpActive) { // Jika di atas, mulai turun
        currentHookState = STATE_AT_TOP; // Akan segera berpindah ke MOVING_DOWN setelah jeda
        stateChangeTime = millis(); // Reset timer
      } else { // Di antara, mulai turun (atau naik, sesuaikan)
        moveHookDown(); // Mulai turun
        currentHookState = STATE_MOVING_DOWN;
      }
    }
  }

  // Otomatis berhenti jika mencapai limit switch saat bergerak (ini tetap logika keamanan dasar)
  if (motorDirection == 1 && limitUpActive) { // Jika sedang naik dan limit atas aktif
    // Serial.println("HOOK: Batas atas tercapai. Hook berhenti.");
    stopHook();
    if (currentHookState == STATE_MOVING_UP) { // Jika dalam mode otomatis naik
        currentHookState = STATE_AT_TOP;
        stateChangeTime = millis(); // Catat waktu tiba di atas
    }
  } else if (motorDirection == -1 && limitDownActive) { // Jika sedang turun dan limit bawah aktif
    // Serial.println("HOOK: Batas bawah tercapai. Hook berhenti.");
    stopHook();
    if (currentHookState == STATE_MOVING_DOWN) { // Jika dalam mode otomatis turun
        currentHookState = STATE_AT_BOTTOM;
        stateChangeTime = millis(); // Catat waktu tiba di bawah
    }
  }

  // =====================================================================
  // >>>>>> LOGIKA BARU UNTUK MEMUTAR MOTOR (OTOMATIS) <<<<<<

  switch (currentHookState) {
    case STATE_IDLE:
      // Tidak melakukan apa-apa, menunggu perintah manual atau 'a'
      break;

    case STATE_MOVING_UP:
      // Hanya biarkan motor bergerak sampai limit atas aktif
      // Logika penghentian ada di atas (Otomatis berhenti jika mencapai limit switch saat bergerak)
      break;

    case STATE_AT_TOP:
      // Jeda di posisi atas
      if (millis() - stateChangeTime >= delayAtLimit) {
        moveHookDown(); // Mulai bergerak turun
        currentHookState = STATE_MOVING_DOWN;
      }
      break;

    case STATE_MOVING_DOWN:
      // Hanya biarkan motor bergerak sampai limit bawah aktif
      // Logika penghentian ada di atas
      break;

    case STATE_AT_BOTTOM:
      // Jeda di posisi bawah
      if (millis() - stateChangeTime >= delayAtLimit) {
        moveHookUp(); // Mulai bergerak naik
        currentHookState = STATE_MOVING_UP;
      }
      break;
  }

  // =====================================================================

  // >>>>>> LOGIKA PRINT STATUS POSISI HOOK SECARA PERIODIK <<<<<<
  unsigned long currentTime = millis();
  if (currentTime - lastPositionPrintTime >= positionPrintInterval) {
    if (limitUpActive && limitDownActive) { // Kondisi yang tidak seharusnya terjadi (error)
      // Serial.println("HOOK: ERROR - Kedua limit switch aktif bersamaan! Periksa wiring/mekanis.");
      // Mungkin juga ingin menghentikan motor sebagai tindakan darurat
      // stopHook(); // Jika ini terjadi, disarankan stop dan keluar dari mode otomatis
      currentHookState = STATE_IDLE; // Keluar dari mode otomatis jika ada error fatal
    }
    else if (limitUpActive) { // Hanya limit atas yang aktif
      // Serial.println("HOOK: Posisi saat ini ADA DI ATAS.");
      // Tambahan: Pastikan motor berhenti sepenuhnya jika masih berjalan ke atas
      if (motorRunning && motorDirection == 1) {
          stopHook();
      }
    } else if (limitDownActive) { // Hanya limit bawah yang aktif
      // Serial.println("HOOK: Posisi saat ini ADA DI BAWAH.");
      // Tambahan: Pastikan motor berhenti sepenuhnya jika masih berjalan ke bawah
      if (motorRunning && motorDirection == -1) {
          stopHook();
      }
    } else { // Tidak ada limit switch yang aktif
      if (motorRunning) { // Jika motor sedang bergerak
         // Serial.print("HOOK: Sedang bergerak ");
         // if (motorDirection == 1) Serial.println("NAIK.");
         // else if (motorDirection == -1) Serial.println("TURUN.");
      } else { // Motor tidak bergerak dan tidak di limit switch
         // Serial.println("HOOK: Di antara posisi atas dan bawah (diam).");
      }
    }
    lastPositionPrintTime = currentTime; // Update waktu terakhir print
  }
  // >>>>>> AKHIR LOGIKA PRINT STATUS <<<<<<

  delay(50); // Delay kecil untuk stabilitas pembacaan sensor dan serial
}

// Fungsi untuk menggerakkan hook ke atas
void moveHookUp() {
  if (motorDirection != 1) { // Hanya ubah arah jika belum naik
    // Serial.println("Hook bergerak naik...");
    // Atur pin DI1 dan DI2 untuk arah naik
    // *** ANDA MUNGKIN PERLU MEMBALIKKAN HIGH/LOW INI TERGANTUNG PADA DRIVER DAN WIRING MOTOR ANDA ***
    digitalWrite(MOTOR_DI1_PIN, HIGH);
    digitalWrite(MOTOR_DI2_PIN, LOW);
    ledcWrite(MOTOR_PWM_PIN, motorSpeed); // Aplikasikan kecepatan
    motorDirection = 1;
    motorRunning = true;
  }
}

// Fungsi untuk menggerakkan hook ke bawah
void moveHookDown() {
  if (motorDirection != -1) { // Hanya ubah arah jika belum turun
    // Serial.println("Hook bergerak turun...");
    // Atur pin DI1 dan DI2 untuk arah turun
    // *** ANDA MUNGKIN PERLU MEMBALIKKAN HIGH/LOW INI TERGANTUNG PADA DRIVER DAN WIRING MOTOR ANDA ***
    digitalWrite(MOTOR_DI1_PIN, LOW);
    digitalWrite(MOTOR_DI2_PIN, HIGH);
    ledcWrite(MOTOR_PWM_PIN, motorSpeed); // Aplikasikan kecepatan
    motorDirection = -1;
    motorRunning = true;
  }
}

// Fungsi untuk menghentikan hook
void stopHook() {
  if (motorRunning) { // Hanya lakukan jika motor sedang berjalan
    // Serial.println("Hook berhenti.");
    // Matikan kedua pin arah untuk memastikan motor berhenti sepenuhnya
    digitalWrite(MOTOR_DI1_PIN, LOW);
    digitalWrite(MOTOR_DI2_PIN, LOW);
    ledcWrite(MOTOR_PWM_PIN, 0); // Matikan PWM (kecepatan 0)
    motorDirection = 0;
    motorRunning = false;
  }
}