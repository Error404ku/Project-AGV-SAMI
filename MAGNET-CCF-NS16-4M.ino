#include <ModbusMaster.h>

// ==================== Konfigurasi RS485 ====================
#define MAX485_DE 12
#define MAX485_RE 12
#define RXD2 18
#define TXD2 17
#define BAUDRATE 19200  // Ganti jika sensor diubah di software sensor

ModbusMaster node;

// Variabel untuk timing millis()
unsigned long previousMillis = 0;
const unsigned long interval = 500;  // Interval 500ms

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
  node.begin(1, Serial2);  // Slave ID = 1
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  Serial.println(F("Inisialisasi selesai."));
}

// ==================== Loop ====================
void loop() {
  unsigned long currentMillis = millis();
  
  // Baca sensor setiap 500ms
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    bacaSensor();
  }

  // Jika ingin, di sini bisa ditambahkan tugas lain tanpa blocking
}

// ==================== Fungsi Membaca Sensor ====================
void bacaSensor() {
  Serial.println(F("Mengirim permintaan pembacaan Median dan Position Value..."));
  uint8_t result = node.readHoldingRegisters(0x0000, 2);  // Median dan Position

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

    // Mapping ke Segment 1-16
    uint8_t segment = mapPositionToSegment(medianValue);
    Serial.print(F("Segment Aktif : "));
    Serial.println(segment);

    // Deteksi Pojok
    if (segment == 1) {
      Serial.println(F("Posisi: Pojok Kiri (Segment 1)"));
    } else if (segment == 16) {
      Serial.println(F("Posisi: Pojok Kanan (Segment 16)"));
    } else {
      Serial.println(F("Posisi: Tengah Jalur"));
    }

    Serial.println(F("============================\n"));
  } 
  else {
    Serial.print(F("Gagal membaca data sensor. Error Code: 0x"));
    Serial.println(result, HEX);
    if (result == node.ku8MBResponseTimedOut) {
      Serial.println(F("Deskripsi: Waktu tunggu habis, sensor tidak merespon."));
    } else if (result == node.ku8MBInvalidCRC) {
      Serial.println(F("Deskripsi: CRC tidak valid."));
    } else if (result == node.ku8MBIllegalDataAddress) {
      Serial.println(F("Deskripsi: Alamat register salah."));
    } else if (result == node.ku8MBIllegalDataValue) {
      Serial.println(F("Deskripsi: Nilai data salah."));
    } else {
      Serial.println(F("Deskripsi: Kesalahan Modbus lainnya."));
    }
  }
}

// ==================== Fungsi Mapping Position ke Segment ====================
uint8_t mapPositionToSegment(uint16_t positionValue) {
  uint16_t segmentSize = 4096;  // 65536 / 16 = 4096
  uint8_t segment = segmentSize/4096 + 1;
  if (segment > 16) segment = 16;
  return segment;
}