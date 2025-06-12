#include <ModbusMaster.h>

// ==================== Konfigurasi RS485 ====================
#define MAX485_DE 12
#define MAX485_RE 12
#define RXD2 18
#define TXD2 17
#define BAUDRATE 19200  // Ganti jika sensor diubah di software sensor. Default pabrik adalah 9600.

ModbusMaster node;

// Variabel untuk timing millis()
unsigned long previousMillis = 0;
const unsigned long interval = 500;   // Interval 500ms

// ==================== Fungsi Kontrol Transceiver ====================
void preTransmission() {
  digitalWrite(MAX485_RE, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission() {
  digitalWrite(MAX485_RE, 0);
  digitalWrite(MAX485_DE, 0);
}

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=== MEMBACA SENSOR MAGNET CCF-NS16-4M (POLLING MODE) ==="));

  // Inisialisasi Serial2
  Serial2.begin(BAUDRATE, SERIAL_8N1, RXD2, TXD2);

  // Setup pin DE/RE
  pinMode(MAX485_RE, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE, 0);
  digitalWrite(MAX485_DE, 0);

  // Setup Modbus
  node.begin(1, Serial2);   // Slave ID = 1 (default pabrik adalah 1)
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  Serial.println(F("Inisialisasi selesai."));
}

// ==================== Loop ====================
void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    bacaSensor();
  }
}

// ==================== Fungsi Membaca Sensor ====================
void bacaSensor() {
  Serial.println(F("Mengirim permintaan pembacaan..."));
  uint8_t result = node.readHoldingRegisters(0x0000, 2);

  if (result == node.ku8MBSuccess) {
    uint16_t medianValue = node.getResponseBuffer(0);
    uint16_t positionValue = node.getResponseBuffer(1);

    Serial.println(F("\n=== Data Sensor Diterima ==="));
    Serial.print(F("Median Value  : "));
    Serial.print(medianValue);
    Serial.print(F(" (0x"));
    Serial.print(medianValue, HEX);
    Serial.println(F(")"));

    Serial.print(F("Position Value: "));
    Serial.print(positionValue);
    Serial.print(F(" (0x"));
    Serial.print(positionValue, HEX);
    Serial.println(F(")"));
    
    printActiveSegmentsFromBitmask(positionValue);
    
    Serial.println(F("----------------------------"));

    if (positionValue == 0xFFFF) {
        Serial.println(F("Status: Di Luar Jalur"));
    } else {
        int errorValue = hitungErrorPosisi(positionValue);
        Serial.print(F("Nilai Error Posisi: "));
        Serial.println(errorValue);
    }

    Serial.println(F("============================\n"));
    
  } else {
    Serial.print(F("Gagal membaca data sensor. Error Code: 0x"));
    Serial.println(result, HEX);
  }
}

// ==================== Fungsi untuk mencetak semua segmen aktif dari Bitmask (Active Low) ====================
void printActiveSegmentsFromBitmask(uint16_t positionValue) {
  Serial.print(F("Segmen Aktif  : "));
  if (positionValue == 0xFFFF) {
    Serial.println(F("Tidak ada magnet / Diluar Jalur."));
    return;
  }

  bool foundAny = false;
  for (int i = 0; i < 16; i++) {
    if (!((positionValue >> i) & 0x01)) { 
      Serial.print(i + 1);
      Serial.print(" ");
      foundAny = true;
    }
  }
  if (!foundAny) {
    Serial.print(F("Tidak ada segmen aktif (Error Logika)."));
  }
  Serial.println();
}

// ==================== FUNGSI FINAL (Logika Prioritas Benar) ====================
int hitungErrorPosisi(uint16_t bitmask) {
    int jumlahSegmenAktif = 0;
    int segmenTertinggi = 0;
    int segmenTerendah = 17; // Mulai dengan angka di luar jangkauan (1-16)

    // 1. Cari metrik penting: jumlah, segmen terendah, dan segmen tertinggi
    for (int i = 0; i < 16; i++) {
        if (!((bitmask >> i) & 0x01)) { // Jika segmen (i+1) aktif
            jumlahSegmenAktif++;
            int segmenSaatIni = i + 1;
            if (segmenSaatIni < segmenTerendah) {
                segmenTerendah = segmenSaatIni;
            }
            if (segmenSaatIni > segmenTertinggi) {
                segmenTertinggi = segmenSaatIni;
            }
        }
    }

    // 2. Jika tidak ada segmen aktif, keluar
    if (jumlahSegmenAktif == 0) {
        return 99; // Error penanda "hilang jalur"
    }

    // 3. Terapkan logika baru dengan urutan prioritas yang benar

    // PRIORITAS 1: Cek pergeseran ke KIRI terlebih dahulu.
    // Jika segmen terendah < 7, ini adalah prioritas utama.
    if (segmenTerendah < 7) {
        // Hitung error negatif berdasarkan segmen TERENDAH
        switch (segmenTerendah) {
            case 6: return -1;
            case 5: return -2;
            case 4: return -3;
            case 3: return -4;
            case 2: return -5;
            case 1: return -6;
            default: return 99;
        }
    }
    // PRIORITAS 2: Jika tidak di kiri, cek pergeseran ke KANAN.
    // Jika segmen tertinggi > 10.
    else if (segmenTertinggi > 10) {
        // Hitung error positif berdasarkan segmen TERTINGGI
        switch (segmenTertinggi) {
            case 11: return 1;
            case 12: return 2;
            case 13: return 3;
            case 14: return 4;
            case 15: return 5;
            case 16: return 6;
            default: return 99;
        }
    }
    // PRIORITAS 3: Jika tidak di kiri dan tidak di kanan, PASTI di TENGAH.
    // Kondisi ini hanya akan tercapai jika semua segmen aktif berada di antara 7 dan 10.
    else {
        return 0;
    }
}