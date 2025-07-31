void displayPrint() {
  lcd.setCursor(0, 0);
  lcd.print("AGV Mode: ");
  lcd.print(agvStateToString(currentStateAgv));
  displaySensorData();
}

void scrollText(int row, String message, int delayTime) {
  unsigned long lastScrollTime = 0;
  int scrollPos = 0;
  // Inisialisasi ulang posisi scroll dan waktu jika pesan berubah atau fungsi dipanggil pertama kali
  static String currentMessage = "";
  if (message != currentMessage) {
    currentMessage = message;
    scrollPos = 0;
    lastScrollTime = millis();
  }

  // Tambahkan spasi di awal dan akhir pesan untuk efek scrolling yang mulus
  String paddedMessage = message;
  for (int i = 0; i < 16; i++) {
    paddedMessage = " " + paddedMessage;  
  } 
  paddedMessage = paddedMessage + " "; 

  // Lakukan scrolling jika waktu yang ditentukan telah berlalu
  if (millis() - lastScrollTime > delayTime) {
    lastScrollTime = millis();
    lcd.setCursor(0, row);
    lcd.print(paddedMessage.substring(scrollPos, scrollPos + 16));
    scrollPos++;
    if (scrollPos > paddedMessage.length() - 16) {
      scrollPos = 0;
    }
  }
}

void modeDisplayWarehouse(){
  lcd.setCursor(0,0);
  lcd.print("Mode: Warehouse");
  scrollText(1, "Tekan Start untuk jalan", 500);
}

void modeDisplayMoveForward(){
  scrollText(0, "Mode : Move Forward", 500);
  displaySensorData;
}

void modeDisplayMoveBackward(){
  scrollText(0, "Mode : Move Backward", 500);
  displaySensorData;
}

void modeDisplayTerminalPickup(){
  lcd.setCursor(0,0);
  scrollText(0, "Mode: Terminal Pickup", 500);
  scrollText(1, "Tekan Start untuk jalan", 500);
}

void modeDisplayTerminalDrop(){
  lcd.setCursor(0,0);
  scrollText(0, "Mode: Terminal Drop", 500);
  scrollText(1, "Tekan Start untuk jalan", 500);
}

void modeDisplayStation(){
  lcd.setCursor(0,0);
  scrollText(0, "Mode: Station", 500);
  scrollText(1, "Tekan Start untuk jalan", 500);
}

void displaySensorData() {
  // Display sensor data on LCD (16 sensors in 2 rows)
  lcd.setCursor(0, 1);
  lcd.print("Sensor Magnet ");
  if (getCurrentMagnetSlaveId() == SLAVEID_MAGNET_DEPAN) {
    lcd.print("F");
  } else {
    lcd.print("B");
  }

  // Display magnet sensor status: 1 = detected, 0 = not detected
  lcd.setCursor(0, 2);
  // Menampilkan dari kanan ke kiri (sensor 15, 14, 13, ... 0)
  for (int i = 15; i >= 0; i--) {
    if (jumlahMagnet[i] == 1) {
      lcd.print("1");
    } else {
      lcd.print("0");
    }
  }

  // Show error value
  lcd.setCursor(0, 3);
  lcd.print("Error: ");
  lcd.print(errorValue);
}
