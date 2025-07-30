void displayPrint() {
  // lcd.clear();
  displayLogicAgv();
  Serial.println("Display Print");
  // displayEncoderValue();
  // displayRpm();
}


void displaySensorData() {
  // Display sensor data on LCD (16 sensors in 2 rows)
  lcd.setCursor(0, 0);
  lcd.print("Sensor Magnet ");
  if (getCurrentMagnetSlaveId() == SLAVEID_MAGNET_DEPAN) {
    lcd.print("F");
  } else {
    lcd.print("B");
  }

  // Display magnet sensor status: 1 = detected, 0 = not detected
  lcd.setCursor(0, 1);
  // Menampilkan dari kanan ke kiri (sensor 15, 14, 13, ... 0)
  for (int i = 15; i >= 0; i--) {
    if (jumlahMagnet[i] == 1) {
      lcd.print("1");
    } else {
      lcd.print("0");
    }
  }

  // Show error value
  lcd.setCursor(0, 2);
  lcd.print("Error: ");
  lcd.print(errorValue);
}
