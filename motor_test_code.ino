// Motor Communication Test
// Tambahkan kode ini di loop() agv_sami.ino untuk test komunikasi motor

bool motorTestRunning = false;
unsigned long lastMotorTest = 0;
int motorTestStep = 0;

void testMotorCommunication() {
  static unsigned long lastTest = 0;
  static int testStep = 0;
  
  // Test setiap 3 detik
  if (millis() - lastTest > 3000) {
    lastTest = millis();
    
    switch(testStep) {
      case 0:
        Serial.println("Motor Test: Forward");
        kirimPerintahMotor(1000, 1000);
        break;
      case 1:
        Serial.println("Motor Test: Stop");
        kirimPerintahMotor(0, 0);
        break;
      case 2:
        Serial.println("Motor Test: Left Turn");
        kirimPerintahMotor(-500, 500);
        break;
      case 3:
        Serial.println("Motor Test: Right Turn");
        kirimPerintahMotor(500, -500);
        break;
      case 4:
        Serial.println("Motor Test: Backward");
        kirimPerintahMotor(-1000, -1000);
        break;
      default:
        Serial.println("Motor Test: Stop");
        kirimPerintahMotor(0, 0);
        testStep = -1; // Reset akan menjadi 0 di increment
        break;
    }
    testStep++;
  }
}

// Cara Menggunakan:
// 1. Tambahkan panggilan testMotorCommunication(); di dalam loop() agv_sami.ino
// 2. Comment out bagian AGV logic sementara untuk test
// 3. Upload dan lihat Serial Monitor untuk melihat perintah yang dikirim
// 4. Monitor Serial ESP32 Motor Controller untuk melihat perintah yang diterima
// 5. Perhatikan apakah motor bergerak sesuai perintah
