// Interrupt Service Routines untuk encoder
void IRAM_ATTR EncoderKanan() {
  enc_kanan++;
}

void IRAM_ATTR EncoderKiri() {
  enc_kiri++;
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
    // Disable interrupts untuk baca nilai encoder secara atomic
    noInterrupts();
    long tempEncKanan = enc_kanan;
    long tempEncKiri = enc_kiri;
    enc_kanan = 0;  // Reset counter
    enc_kiri = 0;   // Reset counter
    interrupts();
    
    // Perhitungan RPM: (pulses * 60000) / (interval_ms * pulses_per_rotation)
    rpm_depan_kanan = (float)(tempEncKanan * 60000) / (intervalrpm * perRotasi);
    rpm_depan_kiri = (float)(tempEncKiri * 60000) / (intervalrpm * perRotasi);
    
    // Filter noise - set nilai sangat kecil ke 0
    if (fabs(rpm_depan_kanan) < 0.1) rpm_depan_kanan = 0.0;
    if (fabs(rpm_depan_kiri) < 0.1) rpm_depan_kiri = 0.0;

    // Debug output yang muncul setiap 1 detik
    Serial.print("Encoder - Kanan: "); Serial.print(tempEncKanan);
    Serial.print(" pulses, Kiri: "); Serial.print(tempEncKiri); Serial.print(" pulses");
    Serial.print(" | RPM Kanan: "); Serial.print(rpm_depan_kanan, 2);
    Serial.print(", RPM Kiri: "); Serial.println(rpm_depan_kiri, 2);
  }
}
