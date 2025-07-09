int nilai_tombol;
bool tombolBoot = false;
unsigned long bootHoldStart = 0;
int lastPressed;
int settingIndex = -1;
String labelTombol[] = { "UP", "LEFT", "RIGHT", "DOWN", "X", "Y", "A", "B" };
int nilaiTombol[8];  // Menyimpan nilai yang dibaca dari Preferences
void setupTombol() {
  pinMode(BOOT_PIN, INPUT_PULLUP);
  pinMode(tombol, INPUT);
  preferences.begin("tombol", false);
  for (int i = 0; i < 8; i++) {
    nilaiTombol[i] = preferences.getInt(labelTombol[i].c_str(), 0);
    Serial.print(labelTombol[i]);
    Serial.print(" = ");
    Serial.println(nilaiTombol[i]);
  }
  // Cek apakah semua data sudah tersimpan
  bool semuaSudahDiset = true;
  for (int i = 0; i < 8; i++) {
    if (nilaiTombol[i] <= 0) {
      semuaSudahDiset = false;
      break;
    }
  }
  // preferences.end();
  int currentMillis = millis();
  while (millis() - currentMillis <= abs(3000)) {
    // lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SETUP TOMBOL");
    if (digitalRead(BOOT_PIN) == LOW) {
      if (bootHoldStart == 0) bootHoldStart = millis();
      if (millis() - bootHoldStart >= 2000 && !tombolBoot) {
        tombolBoot = true;


        Serial.println("Masuk mode setup tombol...");
        aturNilaiTombol();
        tombolBoot = false;
        bootHoldStart = 0;
      }
    } else {
      bootHoldStart = 0;
    }
  }
  if (!semuaSudahDiset) {
    tombolBoot = true;
    aturNilaiTombol();
  }
  Serial.println("SETUP TOMBOL SELESAI");
  lcd.clear();
}

void aturNilaiTombol() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PENCET TOMBOL BOOT");

  while (tombolBoot) {
    if (digitalRead(BOOT_PIN) == LOW && millis() - lastPressed > 1000) {
      lastPressed = millis();
      settingIndex++;

      if (settingIndex < 8) {
        tampilkanKalibrasiTombol(settingIndex);
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Semua tombol");
        lcd.setCursor(0, 1);
        lcd.print("berhasil diset!");
        delay(2000);
        settingIndex = -1;
        break;
      }
    }

    if (settingIndex >= 0 && settingIndex < 8) {
      nilai_tombol = analogRead(tombol);
      if (nilai_tombol > 50) {
        nilaiTombol[settingIndex] = nilai_tombol;
        preferences.putInt(labelTombol[settingIndex].c_str(), nilai_tombol);
        tampilkanKalibrasiTombol(settingIndex);
        delay(500);
      }
    }
  }

  lcd.setCursor(0, 0);
  lcd.print("Keluar kalibrasi");
  preferences.end();
  delay(1000);
  lcd.clear();
}
void tampilkanKalibrasiTombol(int indexAktif) {
  lcd.clear();
  
  String labelTombolList[8] = {"UP", "LEFT", "RIGHT", "DOWN", "X", "Y", "A", "B"};
  
  // Show current button being calibrated
  lcd.setCursor(0, 0);
  lcd.print("Kalibrasi Tombol:");
  lcd.setCursor(0, 1);
  lcd.print(">");
  lcd.print(labelTombolList[indexAktif]);
  lcd.print(" (");
  lcd.print(indexAktif + 1);
  lcd.print("/8)");
  
  // Show current value
  lcd.setCursor(0, 2);
  lcd.print("Nilai: ");
  lcd.print(nilaiTombol[indexAktif]);
  
  // Show instruction
  lcd.setCursor(0, 3);
  lcd.print("Tekan tombol ini");
}

bool tombolDitekan(int index) {
  nilai_tombol = analogRead(tombol);
  return abs(nilai_tombol - nilaiTombol[index]) < 300;  // toleransi 100
}

bool UP() {
  return tombolDitekan(0);
}
bool LEFT() {
  return tombolDitekan(1);
}
bool RIGHT() {
  return tombolDitekan(2);
}
bool DOWN() {
  return tombolDitekan(3);
}
bool X() {
  return tombolDitekan(4);
}
bool Y() {
  return tombolDitekan(5);
}
bool A() {
  return tombolDitekan(6);
}
bool B() {
  return tombolDitekan(7);
}


void uji_tombol() {
  nilai_tombol = analogRead(tombol);
  // Show button press on LCD at position (13,2) - right side
  lcd.setCursor(13, 2);
  if (UP()) {
    lcd.print("UP ");
  }
  else if (LEFT()) {
    lcd.print("LF ");
  }
  else if (RIGHT()) {
    lcd.print("RT ");
  }
  else if (DOWN()) {
    lcd.print("DN ");
  }
  else if (X()) {
    lcd.print("X  ");
  }
  else if (Y()) {
    lcd.print("Y  ");
  }
  else if (A()) {
    lcd.print("A  ");
  }
  else if (B()) {
    lcd.print("B  ");
  }
  else {
    lcd.print("   "); // Clear if no button pressed
  }
}