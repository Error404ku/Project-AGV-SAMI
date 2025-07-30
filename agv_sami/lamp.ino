void lamp_flip_flop() {
    // Variabel statis untuk menyimpan waktu dan state terakhir
    static unsigned long lastToggleTime = 0;
    static bool outputState = LOW; // State awal, merepresentasikan output Q

    // Interval waktu (berperan sebagai clock trigger)
    const int intervalLamp = 1000;
    unsigned long currentTime = millis();

    // Cek apakah sudah waktunya untuk "trigger" (toggle)
    if (currentTime - lastToggleTime >= intervalLamp) {

        lastToggleTime = currentTime; // Simpan waktu trigger saat ini

        // Logika Flip-Flop: membalik state output
        if (outputState == LOW) {
            outputState = HIGH;
        } else {
            outputState = LOW;
        }

        // Tulis state baru ke pin lampu
        digitalWrite(lampPin, outputState);
    }
}