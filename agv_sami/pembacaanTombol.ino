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
  while (millis() - currentMillis <= abs(5000)) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.print("SETUP TOMBOL");
    display.setCursor(0, 10);
    display.display();
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
  display.clearDisplay();
  if (!semuaSudahDiset) {
    tombolBoot = true;
    aturNilaiTombol();
  }
  Serial.println("SETUP TOMBOL SELESAI");
}

void aturNilaiTombol() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("PENCET TOMBOL BOOT");
  display.display();

  while (tombolBoot) {
    if (digitalRead(BOOT_PIN) == LOW && millis() - lastPressed > 1000) {
      lastPressed = millis();
      settingIndex++;

      if (settingIndex < 8) {
        tampilkanKalibrasiTombol(settingIndex);
      } else {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Semua tombol");
        display.setCursor(0, 15);
        display.print("berhasil diset!");
        display.display();
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

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Keluar kalibrasi");
  display.display();
  preferences.end();
  delay(1000);
}
void tampilkanKalibrasiTombol(int indexAktif) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  struct Tombol {
    const char* label;
    int x, y;
  };

  Tombol tombolList[8] = {
    {"UP", 5, 0}, {"LEFT", 25, 10}, {"RIGHT", 45, 10}, {"DOWN", 5, 20},
    {"X", 70, 0}, {"Y", 90, 0}, {"A", 70, 30}, {"B", 90, 30}
  };

  for (int i = 0; i < 8; i++) {
    int x = tombolList[i].x;
    int y = tombolList[i].y;

    // Highlight aktif
    if (i == indexAktif) {
      display.fillRect(x - 2, y - 2, 20, 20, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK); // Tulisan hitam di atas highlight putih
    } else {
      display.setTextColor(SSD1306_WHITE);
    }

    // Gambar ikon/tulisan
    if (strcmp(tombolList[i].label, "UP") == 0) {
      display.fillTriangle(x + 8, y + 2, x + 2, y + 12, x + 14, y + 12, SSD1306_WHITE);
    } else if (strcmp(tombolList[i].label, "DOWN") == 0) {
      display.fillTriangle(x + 8, y + 12, x + 2, y + 2, x + 14, y + 2, SSD1306_WHITE);
    } else if (strcmp(tombolList[i].label, "LEFT") == 0) {
      display.fillTriangle(x + 2, y + 8, x + 12, y + 2, x + 12, y + 14, SSD1306_WHITE);
    } else if (strcmp(tombolList[i].label, "RIGHT") == 0) {
      display.fillTriangle(x + 12, y + 8, x + 2, y + 2, x + 2, y + 14, SSD1306_WHITE);
    } else {
      display.setCursor(x + 5, y + 5);
      display.setTextSize(2);
      display.print(tombolList[i].label);
      display.setTextSize(1);
    }

    // Tampilkan nilai
    display.setCursor(x, y + 18);
    display.print(nilaiTombol[i]);
  }

  display.display();
}

bool tombolDitekan(int index) {
  nilai_tombol = analogRead(tombol);
  return abs(nilai_tombol - nilaiTombol[index]) < 100;  // toleransi 100
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
  if (UP()) {
    display.setCursor(100, 40);  // Koordinat awal tulisan (x,y) dimulai dari atas-kiri
    display.print("UP");
  }

  if (LEFT()) {
    display.setCursor(100, 40);  // Koordinat awal tulisan (x,y) dimulai dari atas-kiri
    display.print("LEFT");
  }

  if (RIGHT()) {
    display.setCursor(100, 40);  // Koordinat awal tulisan (x,y) dimulai dari atas-kiri
    display.print("RIGHT");
  }

  if (DOWN()) {
    display.setCursor(100, 40);  // Koordinat awal tulisan (x,y) dimulai dari atas-kiri
    display.print("DOWN");
  }

  if (X()) {
    display.setCursor(100, 40);  // Koordinat awal tulisan (x,y) dimulai dari atas-kiri
    display.print("X");
  }

  if (Y()) {
    display.setCursor(100, 40);  // Koordinat awal tulisan (x,y) dimulai dari atas-kiri
    display.print("Y");
  }

  if (A()) {
    display.setCursor(100, 40);  // Koordinat awal tulisan (x,y) dimulai dari atas-kiri
    display.print("A");
  }

  if (B()) {
    display.setCursor(100, 40);  // Koordinat awal tulisan (x,y) dimulai dari atas-kiri
    display.print("B");
  }
}