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

// ==================== FUNGSI FINAL (Logika Berdasarkan Segmen Tertinggi) ====================
// Metode ini sederhana, kuat, dan memenuhi semua permintaan terakhir Anda.
int hitungErrorPosisi(uint16_t bitmask) {
    int jumlahSegmenAktif = 0;
    int segmenTertinggi = 0; // Untuk menyimpan nomor segmen terbesar yang aktif

    // 1. Cari nomor segmen tertinggi yang aktif
    // Loop dari segmen tertinggi (16) ke terendah (1)
    for (int i = 15; i >= 0; i--) {
        // Cek apakah bit ke-i adalah 0 (aktif)
        if (!((bitmask >> i) & 0x01)) {
            // Jika ini segmen aktif pertama yang kita temukan (dari kanan),
            // simpan nomornya dan hentikan loop.
            if (jumlahSegmenAktif == 0) {
                segmenTertinggi = i + 1; // i+1 adalah nomor segmen
            }
            jumlahSegmenAktif++;
        }
    }

    // 2. Jika tidak ada segmen aktif sama sekali, keluar
    if (jumlahSegmenAktif == 0) {
        return 99; // Error penanda "hilang jalur"
    }

    // 3. Tentukan nilai error berdasarkan segmen tertinggi
    // Aturan untuk Error 0 (range 7-10)
    if (segmenTertinggi >= 7 && segmenTertinggi <= 10) {
        return 0;
    }
    // Aturan untuk Error Positif (ke kanan)
    else if (segmenTertinggi == 11) { return 1;  }
    else if (segmenTertinggi == 12) { return 2;  }
    else if (segmenTertinggi == 13) { return 3;  }
    else if (segmenTertinggi == 14) { return 4;  }
    else if (segmenTertinggi == 15) { return 5;  }
    else if (segmenTertinggi == 16) { return 6;  } // Maksimum
    // Aturan untuk Error Negatif (ke kiri)
    else if (segmenTertinggi == 6)  { return -1; }
    else if (segmenTertinggi == 5)  { return -2; }
    else if (segmenTertinggi == 4)  { return -3; }
    else if (segmenTertinggi == 3)  { return -4; }
    else if (segmenTertinggi == 2)  { return -5; }
    else if (segmenTertinggi == 1)  { return -6; } // Minimum

    // Fallback jika ada kondisi aneh
    return 99;
}

// // ==================== FUNGSI FINAL (Logika & Urutan Benar) ====================
// int hitungErrorPosisi(uint16_t bitmask) {
//     // Array boolean untuk mempermudah pengecekan. s[0] untuk segmen 1, dst.
//     bool s[16];
//     for (int i = 0; i < 16; i++) {
//         s[i] = !((bitmask >> i) & 0x01); // Active low, jadi bit 0 artinya true
//     }

//     // --- URUTAN PENGECEKAN DARI UJUNG KE TENGAH ---

//     // -- GESER JAUH KE KANAN (Error Positif Besar) --
//     if ((s[14] && s[15]) || s[15]) return 7;
//     if ((s[12] && s[13] && s[14] && s[15]) || (s[13] && s[14] && s[15])) return 6;
//     if ((s[11] && s[12] && s[13] && s[14]) || (s[12] && s[13] && s[14])) return 5;
//     if ((s[10] && s[11] && s[12] && s[13]) || (s[11] && s[12] && s[13])) return 4;
//     if ((s[9] && s[10] && s[11] && s[12]) || (s[10] && s[11] && s[12])) return 3;

//     // Error +2: Sesuai permintaan Anda untuk 9,10,11,12 atau 10,11,12
//     if ((s[8] && s[9] && s[10] && s[11]) || (s[9] && s[10] && s[11])) return 2;

//     // Error +1: Sesuai permintaan Anda untuk 8,9,10,11 atau 9,10,11
//     if ((s[7] && s[8] && s[9] && s[10]) || (s[8] && s[9] && s[10])) return 1;


//     // -- GESER JAUH KE KIRI (Error Negatif Besar) --
//     if ((s[0] && s[1]) || s[0]) return -7;
//     if ((s[0] && s[1] && s[2] && s[3]) || (s[0] && s[1] && s[2])) return -6;
//     if ((s[1] && s[2] && s[3] && s[4]) || (s[1] && s[2] && s[3])) return -5;
//     if ((s[2] && s[3] && s[4] && s[5]) || (s[2] && s[3] && s[4])) return -4;
//     if ((s[3] && s[4] && s[5] && s[6]) || (s[3] && s[4] && s[5])) return -3;
    
//     // Error -2: Pola simetris dari +2
//     if ((s[4] && s[5] && s[6] && s[7]) || (s[4] && s[5] && s[6])) return -2;

//     // Error -1: Pola simetris dari +1
//     if ((s[5] && s[6] && s[7] && s[8]) || (s[5] && s[6] && s[7])) return -1;


//     // -- KONDISI TENGAH (ERROR 0) - DIPERIKSA PALING AKHIR --
//     // Error 0: Sesuai permintaan Anda untuk 7,8,9,10 atau 7,8,9 atau 8,9,10
//     if ((s[6] && s[7] && s[8] && s[9]) || // 7,8,9,10
//         (s[6] && s[7] && s[8])       || // 7,8,9
//         (s[7] && s[8] && s[9])) {      // 8,9,10
//         return 0;
//     }

//     // Jika tidak ada kombinasi di atas yang cocok sama sekali
//     return 99; // Mengindikasikan pembacaan aneh atau tidak terdefinisi
// }

// // ==================== FUNGSI FINAL (Metode Perhitungan Titik Tengah) ====================
// // Metode ini jauh lebih akurat dan sederhana daripada rantai if-else.
// int hitungErrorPosisi(uint16_t bitmask) {
//     float totalPosisi = 0;
//     int jumlahSegmenAktif = 0;

//     // Iterasi melalui semua 16 segmen
//     for (int i = 0; i < 16; i++) {
//         // Periksa apakah bit ke-i adalah 0 (artinya segmen aktif)
//         if (!((bitmask >> i) & 0x01)) {
//             jumlahSegmenAktif++;
//             totalPosisi += (i + 1); // i+1 adalah nomor segmen (1-16)
//         }
//     }

//     // Jika tidak ada segmen yang aktif, maka di luar jalur.
//     // Kembalikan nilai error yang besar untuk menandakan kondisi ini.
//     if (jumlahSegmenAktif == 0) {
//         return 99; // Atau nilai lain penanda "hilang jalur"
//     }

//     // Hitung posisi rata-rata dari segmen yang aktif
//     float posisiRataRata = totalPosisi / jumlahSegmenAktif;

//     // Titik tengah ideal kita adalah 8.5 (rata-rata dari 7, 8, 9, 10)
//     const float titikTengahIdeal = 8.5;

//     // Hitung error sebagai selisih dari posisi rata-rata dengan titik tengah ideal,
//     // lalu bulatkan ke integer terdekat.
//     int nilaiError = round(posisiRataRata - titikTengahIdeal);

//     return nilaiError;
// }