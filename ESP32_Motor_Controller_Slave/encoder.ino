// Interrupt Service Routines untuk encoder
void IRAM_ATTR EncoderKanan() {
  encKananA = digitalRead(EncoderKananPinA); 
  encKananB = digitalRead(EncoderKananPinB);
  if ((encKananA == HIGH) != (encKananB == LOW)) {
    enc_kanan--;
  } else {
    enc_kanan++;
  }
}

void IRAM_ATTR EncoderKiri() {
  encKiriA = digitalRead(EncoderKiriPinA); 
  encKiriB = digitalRead(EncoderKiriPinB);
  if ((encKiriA == HIGH) != (encKiriB == LOW)) {
    enc_kiri--;
  } else {
    enc_kiri++;
  }
}

// Fungsi untuk checking interval timing
bool checkInterval(unsigned long interval, unsigned long &lastTime) {
  if (millis() - lastTime >= interval) {
    lastTime = millis();
    return true;
  }
  return false;
}

void pembacaan_RPM() {
  if (checkInterval(intervalrpm, milisrpm)) {
    // Formula RPM yang benar: (pulses * 60000) / (intervalrpm * perRotasi)
    // Gunakan floating point untuk akurasi, lalu convert ke int
    rpm_depan_kanan = (int)((long)enc_kanan * 60000L) / ((long)intervalrpm * perRotasi);
    rpm_depan_kiri = (int)((long)enc_kiri * 60000L) / ((long)intervalrpm * perRotasi);

    Serial.print("Encoder - Kanan: "); Serial.print(enc_kanan);
    Serial.print(" pulses, Kiri: "); Serial.print(enc_kiri); Serial.print(" pulses");
    Serial.print(" | RPM Kanan: "); Serial.print(rpm_depan_kanan);
    Serial.print(", RPM Kiri: "); Serial.println(rpm_depan_kiri);
    
    enc_kanan = 0;
    enc_kiri = 0;
  }
}
