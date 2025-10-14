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
    // Gunakan floating point untuk perhitungan yang lebih akurat
    float rpm_kanan_raw = ((float)enc_kanan * 60000.0) / ((float)intervalrpm * (float)perRotasi);
    float rpm_kiri_raw = ((float)enc_kiri * 60000.0) / ((float)intervalrpm * (float)perRotasi);
    
    // Simple moving average filter untuk mengurangi noise
    static float rpm_kanan_filtered = 0;
    static float rpm_kiri_filtered = 0;
    const float alpha = 0.3; // Low-pass filter coefficient (0.3 = smooth, 1.0 = no filter)
    
    rpm_kanan_filtered = alpha * rpm_kanan_raw + (1.0 - alpha) * rpm_kanan_filtered;
    rpm_kiri_filtered = alpha * rpm_kiri_raw + (1.0 - alpha) * rpm_kiri_filtered;
    
    // Convert ke integer untuk compatibility
    rpm_depan_kanan = (int)round(rpm_kanan_filtered);
    rpm_depan_kiri = (int)round(rpm_kiri_filtered);

    Serial.print("Encoder - Kanan: "); Serial.print(enc_kanan);
    Serial.print(" pulses, Kiri: "); Serial.print(enc_kiri); Serial.print(" pulses");
    Serial.print(" | RPM Kanan: "); Serial.print(rpm_depan_kanan);
    Serial.print(", RPM Kiri: "); Serial.println(rpm_depan_kiri);
    
    // Debug raw vs filtered
    if (abs(enc_kanan) > 0 || abs(enc_kiri) > 0) {
      Serial.printf("RPM Raw - Kanan: %.1f, Kiri: %.1f | Filtered - Kanan: %.1f, Kiri: %.1f\n", 
                    rpm_kanan_raw, rpm_kiri_raw, rpm_kanan_filtered, rpm_kiri_filtered);
    }
    
    enc_kanan = 0;
    enc_kiri = 0;
  }
}
