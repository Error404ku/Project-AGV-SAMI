void displayPrint() {
  // lcd.clear();
  displaySensorData();
  // uji_tombol();
  displayLogicAgv();
  Serial.println("Display Print");
  // displayEncoderValue();
  // displayRpm();
}


void displaySensorData() {
  // Display sensor data on LCD (16 sensors in 2 rows)
  lcd.setCursor(0, 0);
  lcd.print("Sensor:");
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print(jumlahMagnet[i] ? "1" : "0");
  }
  
  // Show error value
  lcd.setCursor(0, 2);
  lcd.print("Error: ");
  lcd.print(errorValue);
}

// void displayRpm(){
//   lcd.setCursor(0, 3);
//   lcd.print("RPM R:");
//   lcd.print(rpmKanan);
//   lcd.print(" L:");
//   lcd.print(rpmKiri);
// }

// void displayEncoderValue(){
//   lcd.setCursor(0, 3);
//   lcd.print("Enc R:");
//   lcd.print(encKananAVal);
//   lcd.print(" L:");
//   lcd.print(encKiriAVal);
// }