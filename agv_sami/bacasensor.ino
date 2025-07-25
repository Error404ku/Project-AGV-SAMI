extern bool modeMaju;
extern bool modeMundur;

// Mode switching now handled by unified RS485 system
// All sensors are accessible simultaneously through address switching

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
        // Use unified RS485 communication instead of individual sensor reading
        loopUnifiedRS485();
        
        // Update total active sensors based on current magnet data
        updateTotalSensorAktif();
    }
}

// ==================== Update Total Sensor Aktif ====================
void updateTotalSensorAktif()
{
    // Get current magnet data (front magnet for line following)
    int* currentMagnetData = getCurrentMagnetData();
    
    totalSensorAktif = 0;
    for (int i = 0; i < 16; i++)
    {
        if (currentMagnetData[i]) totalSensorAktif++;
    }
}

// ==================== Fungsi untuk mencetak semua segmen aktif dari Bitmask (Active Low) ====================
void printActiveSegmentsFromBitmask(uint16_t positionValue)
{
    if (positionValue == 0xFFFF)
    {
        return;
    }

    bool foundAny = false;
    for (int i = 0; i < 16; i++)
    {
        if (!((positionValue >> i) & 0x01))
        {
            foundAny = true;
        }
    }
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

// Function moved to unified_rs485.ino as updateMagnetData()
// This function is kept for compatibility but redirects to new system
void updateJumlahMagnet(uint16_t bitmask)
{
    updateMagnetData(bitmask, getCurrentMagnetData());
}

