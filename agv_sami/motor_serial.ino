// Motor Serial Communication - ESP32 Master
// Mengirim perintah motor ke ESP32 kedua via Serial0

void setupMotorSerial() {
  // Inisialisasi Serial0 untuk komunikasi dengan ESP32 kedua
  Serial.begin(115200);
  delay(1000); // Tunggu ESP32 motor siap
}

void kirimPerintahMotor(int speedKiri, int speedKanan) {
  // Format perintah: "L[speed]R[speed]\n"
  // Contoh: "L150R-100\n" (kiri maju 150, kanan mundur 100)
  
  // Batasi kecepatan dalam range 12-bit PWM
  speedKiri = constrain(speedKiri, -4095, 4095);
  speedKanan = constrain(speedKanan, -4095, 4095);
  
  String perintah = "L" + String(speedKiri) + "R" + String(speedKanan) + "\n";
  Serial.print(perintah);
}

void motorMaju(int speed) {
  // Pastikan speed dalam range 12-bit PWM
  speed = constrain(speed, 0, 4095);
  kirimPerintahMotor(speed, speed);
}

void motorMundur(int speed) {
  // Pastikan speed dalam range 12-bit PWM
  speed = constrain(speed, 0, 4095);
  kirimPerintahMotor(-speed, -speed);
}

void motorKiri(int speed) {
  // Pastikan speed dalam range 12-bit PWM
  speed = constrain(speed, 0, 4095);
  kirimPerintahMotor(-speed, speed);
}

void motorKanan(int speed) {
  // Pastikan speed dalam range 12-bit PWM
  speed = constrain(speed, 0, 4095);
  kirimPerintahMotor(speed, -speed);
}

void motorBerhenti() {
  kirimPerintahMotor(0, 0);
}

void motorManual(int speedKiri, int speedKanan) {
  kirimPerintahMotor(speedKiri, speedKanan);
}

void aturKecepatanMotor(int kecepatanKiri, int kecepatanKanan) {
  kirimPerintahMotor(kecepatanKiri, kecepatanKanan);
}

// Fungsi dengan range persentase (0-100%) yang akan dikonversi ke 12-bit PWM
void motorPersentase(int persenKiri, int persenKanan) {
  // Konversi persentase (-100% to 100%) ke 12-bit PWM (-4095 to 4095)
  int speedKiri = (persenKiri * 4095) / 100;
  int speedKanan = (persenKanan * 4095) / 100;
  
  kirimPerintahMotor(speedKiri, speedKanan);
}

// Fungsi untuk kompatibilitas dengan kode lama
void setMotorSpeeds(int leftSpeed, int rightSpeed) {
  kirimPerintahMotor(leftSpeed, rightSpeed);
}

// Fungsi tambahan untuk kompatibilitas
void motorStop() {
  kirimPerintahMotor(0, 0);
}

void motorForward(int speed) {
  motorMaju(speed);
}

void motorBackward(int speed) {
  motorMundur(speed);
}

void motorTurnLeft(int speed) {
  motorKiri(speed);
}

void motorTurnRight(int speed) {
  motorKanan(speed);
}
