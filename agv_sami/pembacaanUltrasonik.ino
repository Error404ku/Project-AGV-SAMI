
// Obstacle detection variables
bool obstacleDetected = false;
uint16_t minSafeDistance = 30; // cm - minimum safe distance
uint16_t ultrasonicDistances[5] = {0}; // Store distances from 5 probes
unsigned long lastObstacleCheck = 0;
const unsigned long obstacleCheckInterval = 100; // Check every 100ms

int currentUltrasonicSlaveId = SLAVEID_ULTRASONIK_DEPAN;
void setUltrasonicSlaveId(int slaveId) {
    currentUltrasonicSlaveId = slaveId;
}

void loopUltrasonik() {
  if (Serial1.available()) {
    byte incomingByte = Serial1.read();
    Serial.println("Seial 1 tersedia");
    // Logika untuk sinkronisasi paket data
    if (!inPacket) {
      // Mencari byte pertama dari header paket (Alamat Slave)
      if (incomingByte == currentUltrasonicSlaveId) {
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
        parsePacket();     // Kirim paket untuk diproses
        inPacket = false;  // Reset untuk mencari paket berikutnya
        byteCounter = 0;
      }
    }
  } else {
    // serial 1 tidak tersedia
    // Serial.println("Serial 1 tidak tersedia"); 
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

      // Store distances in array for obstacle detection
      ultrasonicDistances[0] = dist1;
      ultrasonicDistances[1] = dist2;
      ultrasonicDistances[2] = dist3;
      ultrasonicDistances[3] = dist4;
      ultrasonicDistances[4] = dist5;

      // Check for obstacles
      checkObstacles();

      // Tampilkan hasil
      Serial.printf("  Jarak Probe 1: %d cm\n", dist1);
      Serial.printf("  Jarak Probe 2: %d cm\n", dist2);
      Serial.printf("  Jarak Probe 3: %d cm\n", dist3);
      Serial.printf("  Jarak Probe 4: %d cm\n", dist4);
      Serial.printf("  Jarak Probe 5: %d cm\n\n", dist5);

    } else {
        error(ERROR_ULTRASONIC_COMMUNICATION, "CRC Checksum tidak cocok. Data korup.");
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



/**
 * Check for obstacles in front of AGV
 */
void checkObstacles() {
  bool previousObstacleState = obstacleDetected;
  obstacleDetected = false;
  
  // Check each probe for obstacles
  for (int i = 0; i < 5; i++) {
    if (ultrasonicDistances[i] > 0 && ultrasonicDistances[i] < minSafeDistance) {
      obstacleDetected = true;
      Serial.printf("OBSTACLE DETECTED! Probe %d: %d cm\n", i+1, ultrasonicDistances[i]);
      break;
    }
  }
  
  // If obstacle just detected, trigger buzzer
  if (obstacleDetected && !previousObstacleState) {
    Serial.println("EMERGENCY STOP - Obstacle detected!");
    // buzzerError();
    music("error");
  }
  
  // If obstacle cleared, notify
  if (!obstacleDetected && previousObstacleState) {
    Serial.println("Path clear - obstacle removed");
  }
}