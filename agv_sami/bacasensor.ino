

void preTransmission()
{
    digitalWrite(MAX485_RE, 1);
    digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
    digitalWrite(MAX485_RE, 0);
    digitalWrite(MAX485_DE, 0);
}


void bacaSensorGaris()
{
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        bacaSensor();
    }
}

// ==================== Fungsi Membaca Sensor ====================
void bacaSensor()
{
    // Serial.println(F("Mengirim permintaan pembacaan..."));
    uint8_t result = node.readHoldingRegisters(0x0000, 2);

    if (result == node.ku8MBSuccess)
    {
        uint16_t medianValue = node.getResponseBuffer(0);
        uint16_t positionValue = node.getResponseBuffer(1);

        // Serial.println(F("\n=== Data Sensor Diterima ==="));
        // Serial.print(F("Median Value  : "));
        // Serial.print(medianValue);
        // Serial.print(F(" (0x"));
        // Serial.print(medianValue, HEX);
        // Serial.println(F(")"));

        // Serial.print(F("Position Value: "));
        // Serial.print(positionValue);
        // Serial.print(F(" (0x"));
        // Serial.print(positionValue, HEX);
        // Serial.println(F(")"));

        printActiveSegmentsFromBitmask(positionValue);

        // ⬇️ Tambahkan ini di sini:
        updateJumlahMagnet(positionValue);

        // Serial.println(F("----------------------------"));

        if (positionValue == 0xFFFF)
        {
            // Serial.println(F("Status: Di Luar Jalur"));
            // pwmMotor(0,0);
            totalSensorAktif = 0; 
        }
        else
        {
            totalSensorAktif = 0;
                for (int i = 0; i < 16; i++)
                {
                    if (jumlahMagnet[i]) totalSensorAktif++;
                }
            errorValue = hitungErrorPosisi(positionValue);
            
            // Serial.print(F("Nilai Error Posisi: "));
            // Serial.println(errorValue);
        }

        // Serial.println(F("============================\n"));
    }
    else
    {
        Serial.print(F("Gagal membaca data sensor. Error Code: 0x"));
        Serial.println(result, HEX);
    }
}

// ==================== Fungsi untuk mencetak semua segmen aktif dari Bitmask (Active Low) ====================
void printActiveSegmentsFromBitmask(uint16_t positionValue)
{
    // Serial.print(F("Segmen Aktif  : "));
    if (positionValue == 0xFFFF)
    {
        // Serial.println(F("Tidak ada magnet / Diluar Jalur."));
        return;
    }

    bool foundAny = false;
    for (int i = 0; i < 16; i++)
    {
        if (!((positionValue >> i) & 0x01))
        {
            Serial.print(i + 1);
            Serial.print(" ");
            foundAny = true;
        }
    }
    if (!foundAny)
    {
        Serial.print(F("Tidak ada segmen aktif (Error Logika)."));
    }
    Serial.println();
}
int hitungErrorPosisi(uint16_t bitmask)
{
    int jumlahSegmenAktif = 0;
    int segmenTertinggi = 0;
    int segmenTerendah = 17;

    for (int i = 0; i < 16; i++)
    {
        if (!((bitmask >> i) & 0x01))
        {
            jumlahSegmenAktif++;
            int segmenSaatIni = i + 1;
            if (segmenSaatIni < segmenTerendah)
                segmenTerendah = segmenSaatIni;
            if (segmenSaatIni > segmenTertinggi)
                segmenTertinggi = segmenSaatIni;
        }
    }

    if (jumlahSegmenAktif == 0)
        return 99;

    // Logika baru: cek dua-duanya lalu ambil dominasi
    int errorKiri = 0, errorKanan = 0;

    if (segmenTerendah < 7)
    {
        switch (segmenTerendah)
        {
        case 6: errorKiri = -1; break;
        case 5: errorKiri = -2; break;
        case 4: errorKiri = -3; break;
        case 3: errorKiri = -4; break;
        case 2: errorKiri = -5; break;
        case 1: errorKiri = -6; break;
        }
    }

    if (segmenTertinggi > 10)
    {
        switch (segmenTertinggi)
        {
        case 11: errorKanan = 1; break;
        case 12: errorKanan = 2; break;
        case 13: errorKanan = 3; break;
        case 14: errorKanan = 4; break;
        case 15: errorKanan = 5; break;
        case 16: errorKanan = 6; break;
        }
    }

    // Bandingkan dominasi sisi kiri vs kanan
    if (abs(errorKiri) > abs(errorKanan))
        return errorKiri;
    else if (abs(errorKanan) > abs(errorKiri))
        return errorKanan;
    else
        return 0; // tengah atau seimbang
}

void updateJumlahMagnet(uint16_t bitmask)
{
    for (int i = 0; i < 16; i++)
    {
        jumlahMagnet[i] = !((bitmask >> i) & 0x01) ? 1 : 0;
    }

    // Opsional: tampilkan ke Serial Monitor
    // Serial.print(F("jumlahMagnet[] : "));
    // for (int i = 0; i < 16; i++)
    // {
    //     Serial.print(jumlahMagnet[i]);
    //     Serial.print(" ");
    // }
    // Serial.println();
}

