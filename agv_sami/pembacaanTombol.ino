// Variables for button handling
// ===================================================================
// BUTTON VARIABLES SUDAH DIPINDAHKAN KE config.h
// ===================================================================

// Konstanta debounceDelay sudah didefinisikan di config.h


// Optimized button functions using efficient debounce system
bool UP() {
  if (digitalRead(upPin) == HIGH) {
    unsigned long currentTime = millis();
    if (currentTime - lastUpPress >= debounceDelay) {
      lastUpPress = currentTime;
      return true;
    }
  }
  return false;
}

bool LEFT() {
  if (digitalRead(leftPin) == HIGH) {
    unsigned long currentTime = millis();
    if (currentTime - lastLeftPress >= debounceDelay) {
      lastLeftPress = currentTime;
      return true;
    }
  }
  return false;
}

bool RIGHT() {
  if (digitalRead(rightPin) == HIGH) {
    unsigned long currentTime = millis();
    if (currentTime - lastRightPress >= debounceDelay) {
      lastRightPress = currentTime;
      return true;
    }
  }
  return false;
}

bool DOWN() {
  if (digitalRead(downPin) == HIGH) {
    unsigned long currentTime = millis();
    if (currentTime - lastDownPress >= debounceDelay) {
      lastDownPress = currentTime;
      return true;
    }
  }
  return false;
}

bool START() {
  if (digitalRead(startPin) == HIGH) {
    unsigned long currentTime = millis();
    if (currentTime - lastStartPress >= debounceDelay) {
      lastStartPress = currentTime;
      return true;
    }
  }
  return false;
}

bool STOP() {
  if (digitalRead(stopPin) == HIGH) {
    unsigned long currentTime = millis();
    if (currentTime - lastStopPress >= debounceDelay) {
      lastStopPress = currentTime;
      return true;
    }
  }
  return false;
}

void uji_tombol() {
  // Show button press on LCD at position (13,2) - right side
  // NOTE: No debounce in test function for real-time feedback
  lcd.setCursor(13, 2);

  if (digitalRead(upPin) == HIGH) {
    lcd.print("UP ");
    Serial.println("Tombol UP ditekan");
  } else if (digitalRead(leftPin) == HIGH) {
    lcd.print("LF ");
    Serial.println("Tombol LEFT ditekan");
  } else if (digitalRead(rightPin) == HIGH) {
    lcd.print("RT ");
    Serial.println("Tombol RIGHT ditekan");
  } else if (digitalRead(downPin) == HIGH) {
    lcd.print("DN ");
    Serial.println("Tombol DOWN ditekan");
  } else if (digitalRead(startPin) == HIGH) {
    lcd.print("Start  ");
    Serial.println("Tombol START ditekan");
  } else if (digitalRead(stopPin) == HIGH) {
    lcd.print("Stop  ");
    Serial.println("Tombol STOP ditekan");
  } else {
    lcd.print("       ");  // Clear if no button pressed
  }
}