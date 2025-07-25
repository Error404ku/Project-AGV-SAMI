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
  Serial2.begin(BAUDRATE, SERIAL_8N1, RXD2, TXD2); // Komunikasi format: 8 data bits, 1 stop bit, no parity.

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
  // Membaca 2 Holding Registers dari alamat 0x0000.
  uint8_t result = node.readHoldingRegisters(0x0000, 2);

  if (result == node.ku8MBSuccess) {
    uint16_t medianValue = node.getResponseBuffer(0); // Intermediate value
    uint16_t positionValue = node.getResponseBuffer(1); // Location value (bitmask)

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

    // === BAGIAN BARU: MENGHITUNG DAN MENAMPILKAN NILAI ERROR ===
    if (positionValue == 0xFFFF) {
        Serial.println(F("Status: Di Luar Jalur"));
    } else {
        int errorValue = hitungErrorPosisi(positionValue);
        Serial.print(F("Nilai Error Posisi: "));
        Serial.println(errorValue);
    }
    // === AKHIR BAGIAN BARU ===

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
    // Periksa apakah bit ke-i adalah 0 (Active Low)
    if (!((positionValue >> i) & 0x01)) { 
      Serial.print(i + 1); // Segmen adalah indeks bit + 1
      Serial.print(" ");
      foundAny = true;
    }
  }
  if (!foundAny) {
    Serial.print(F("Tidak ada segmen aktif (Error Logika)."));
  }
  Serial.println();
}

// ==================== FUNGSI BARU: Menghitung Nilai Error dari Bitmask ====================
// Fungsi ini mengembalikan nilai error berdasarkan kombinasi segmen yang aktif (active low).
// Didesain untuk sensor 16 segmen.
int hitungErrorPosisi(uint16_t bitmask) {
    // Definisi bit untuk setiap segmen (segmen 1 = bit 0, dst.)
    // Active low, jadi kita cek apakah bitnya 0
    bool s[16];
    for(int i=0; i<16; i++){
        s[i] = !((bitmask >> i) & 0x01);
    }

    // Kondisi Ideal (Center)
    // Error 0: segmen 7,8,9,10 aktif
    if (s[6] && s[7] && s[8] && s[9]) return 0;

    // Geser ke Kanan (Error Positif)
    // Error 1: segmen 8,9,10 aktif
    if (s[7] && s[8] && s[9]) return 1;
    // Error 2: segmen 8,9,10,11 aktif atau 9,10,11 aktif
    if ((s[7] && s[8] && s[9] && s[10]) || (s[8] && s[9] && s[10])) return 2;
    // Error 3: segmen 9,10,11,12 aktif atau 10,11,12 aktif
    if ((s[8] && s[9] && s[10] && s[11]) || (s[9] && s[10] && s[11])) return 3;
    // Error 4: segmen 10,11,12,13 aktif atau 11,12,13 aktif
    if ((s[9] && s[10] && s[11] && s[12]) || (s[10] && s[11] && s[12])) return 4;
    // Error 5: segmen 11,12,13,14 aktif atau 12,13,14 aktif
    if ((s[10] && s[11] && s[12] && s[13]) || (s[11] && s[12] && s[13])) return 5;
     // Error 6: segmen 12,13,14,15 aktif atau 13,14,15 aktif
    if ((s[11] && s[12] && s[13] && s[14]) || (s[12] && s[13] && s[14])) return 6;
     // Error 7: segmen 13,14,15,16 aktif atau 14,15,16 aktif
    if ((s[12] && s[13] && s[14] && s[15]) || (s[13] && s[14] && s[15])) return 7;
     // Error 8 (Paling Kanan): segmen 15,16 aktif atau 16 aktif
    if ((s[14] && s[15]) || s[15]) return 8;


    // Geser ke Kiri (Error Negatif)
    // Error -1: segmen 7,8,9 aktif
    if (s[6] && s[7] && s[8]) return -1;
    // Error -2: segmen 6,7,8,9 aktif atau 6,7,8 aktif
    if ((s[5] && s[6] && s[7] && s[8]) || (s[5] && s[6] && s[7])) return -2;
    // Error -3: segmen 5,6,7,8 aktif atau 5,6,7 aktif
    if ((s[4] && s[5] && s[6] && s[7]) || (s[4] && s[5] && s[6])) return -3;
    // Error -4: segmen 4,5,6,7 aktif atau 4,5,6 aktif
    if ((s[3] && s[4] && s[5] && s[6]) || (s[3] && s[4] && s[5])) return -4;
    // Error -5: segmen 3,4,5,6 aktif atau 3,4,5 aktif
    if ((s[2] && s[3] && s[4] && s[5]) || (s[2] && s[3] && s[4])) return -5;
    // Error -6: segmen 2,3,4,5 aktif atau 2,3,4 aktif
    if ((s[1] && s[2] && s[3] && s[4]) || (s[1] && s[2] && s[3])) return -6;
    // Error -7: segmen 1,2,3,4 aktif atau 1,2,3 aktif
    if ((s[0] && s[1] && s[2] && s[3]) || (s[0] && s[1] && s[2])) return -7;
    // Error -8 (Paling Kiri): segmen 1,2 aktif atau 1 aktif
    if ((s[0] && s[1]) || s[0]) return -8;


    // Jika tidak ada kombinasi di atas yang cocok, kembalikan nilai default
    // yang menandakan kondisi tidak terdefinisi.
    return 99; // Atau nilai lain yang Anda anggap sebagai error/undefined
}