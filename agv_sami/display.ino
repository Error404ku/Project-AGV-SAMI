void displayPrint() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  displaySensorData();
  uji_tombol();
  display.setCursor(0, 10); 
  displayLogicAgv();
  display.display();
  // displayEncoderValue();
  // displayRpm();
}


void displaySensorData() {
  // Draw line sensor boxes
  int boxWidth = 6;
  int boxHeight = 6;
  int spacing = 1;
  
  // Calculate total width and starting position to center the boxes
  int totalWidth = (boxWidth * 16) + (spacing * 15);
  int startX = (SCREEN_WIDTH - totalWidth) / 2;
  int startY = 0;
  for (int i = 0; i < 16; i++) {
    int x = startX + i * (boxWidth + spacing);
    if (jumlahMagnet[i] == 1) {
      display.fillRect(x, startY, boxWidth, boxHeight, SSD1306_WHITE);
    } else {
      display.drawRect(x, startY, boxWidth, boxHeight, SSD1306_WHITE);
    }
  }
  // display.print("errorValue : ");
  // display.println(errorValue);
  display.display();
}

void displayRpm(){
  display.print("Rpm: ");
  display.print(rpmKanan);
  display.print(" ");
  display.println(rpmKiri);
  display.display();
}

void displayEncoderValue(){
  display.print("Encoder ++ : ");
  display.print(encKananAVal);
  display.print(" ");
  display.println(encKiriAVal);
  display.display();
}