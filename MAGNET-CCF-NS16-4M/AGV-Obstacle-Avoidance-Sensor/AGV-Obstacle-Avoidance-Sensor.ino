// ===== KODE FINAL: PARSER DATA SENSOR MENJADI JARAK =====

// ------------------- PENGATURAN -------------------
#define RX_PIN 18
#define TX_PIN 17
#define MAX485_CTRL_PIN 12
const int SENSOR_BAUD_RATE = 115200;

// Alamat slave sensor yang diharapkan
const byte SENSOR_ADDRESS = 0x01;
// Panjang total paket data Modbus yang diharapkan
const int PACKET_LENGTH = 15;

// Buffer untuk menampung satu paket data
byte dataPacket[PACKET_LENGTH];
int byteCounter = 0;
bool inPacket = false;

// ------------------- FUNGSI SETUP -------------------
void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  
  pinMode(MAX485_CTRL_PIN, OUTPUT);
  digitalWrite(MAX485_CTRL_PIN, LOW); // Selalu dalam mode terima
  
  Serial2.begin(SENSOR_BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);

  Serial.println("\n\n--- Program Parser Sensor Ultrasonik ---");
  Serial.println("Mencari paket data dari sensor...");
}

// ------------------- FUNGSI LOOP UTAMA -------------------
void loop() {
  if (Serial2.available()) {
    byte incomingByte = Serial2.read();

    // Logika untuk sinkronisasi paket data
    if (!inPacket) {
      // Mencari byte pertama dari header paket (Alamat Slave)
      if (incomingByte == SENSOR_ADDRESS) {
        dataPacket[0] = incomingByte;
        byteCounter = 1;
        inPacket = true;
      }
    } else {
      // Jika sudah di dalam paket, lanjutkan mengisi buffer
      dataPacket[byteCounter] = incomingByte;
      byteCounter++;

      // Jika buffer sudah penuh (15 byte terkumpul)
      if (byteCounter >= PACKET_LENGTH) {
        parsePacket(); // Kirim paket untuk diproses
        inPacket = false; // Reset untuk mencari paket berikutnya
        byteCounter = 0;
      }
    }
  }
}

// ------------------- FUNGSI-FUNGSI BANTUAN -------------------

/**
 * Memproses satu paket data yang telah lengkap diterima.
 */
void parsePacket() {
  // Verifikasi header paket
  // byte ke-0 adalah Alamat Slave, byte ke-1 adalah Kode Fungsi
  if (dataPacket[0] == SENSOR_ADDRESS && dataPacket[1] == 0x03) {
    
    // Validasi data dengan CRC Checksum
    uint16_t calculated_crc = calculate_crc(dataPacket, PACKET_LENGTH - 2);
    uint16_t received_crc = (dataPacket[PACKET_LENGTH - 1] << 8) | dataPacket[PACKET_LENGTH - 2];

    if (calculated_crc == received_crc) {
      Serial.println("--- Paket Data Valid Diterima ---");
      
      // Ekstrak dan hitung jarak untuk setiap probe
      // Rumus: Jarak = (High Byte * 256) + Low Byte 
      uint16_t dist1 = (dataPacket[3] << 8) | dataPacket[4];
      uint16_t dist2 = (dataPacket[5] << 8) | dataPacket[6];
      uint16_t dist3 = (dataPacket[7] << 8) | dataPacket[8];
      uint16_t dist4 = (dataPacket[9] << 8) | dataPacket[10];
      uint16_t dist5 = (dataPacket[11] << 8) | dataPacket[12];
      
      // Tampilkan hasil
      Serial.printf("  Jarak Probe 1: %d cm\n", dist1);
      Serial.printf("  Jarak Probe 2: %d cm\n", dist2);
      Serial.printf("  Jarak Probe 3: %d cm\n", dist3);
      Serial.printf("  Jarak Probe 4: %d cm\n", dist4);
      Serial.printf("  Jarak Probe 5: %d cm\n\n", dist5);

    } else {
      Serial.println("Error: CRC Checksum tidak cocok. Data korup.");
    }
  }
}

/**
 * Menghitung CRC-16 untuk validasi data Modbus.
 * Ini adalah fungsi standar dan tidak perlu diubah.
 */
uint16_t calculate_crc(byte* buffer, int len) {
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buffer[pos];
    for (int i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}