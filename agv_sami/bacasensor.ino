extern bool modeMaju;
extern bool modeMundur;

void changeStateMode(String mode){
    if (mode == "maju" && !modeMundur){
        setupSensorMagnet(1, RX_MAGNET_FRONT, TX_MAGNET_FRONT, BAUDRATE);
        setupUltrasonikWithParams(RX_ULTRASONIK_FRONT, TX_ULTRASONIK_FRONT, BAUDRATE);
        modeMaju = true;
        modeMundur = false;
    }else if (mode == "mundur" && !modeMaju){
        setupSensorMagnet(1, RX_MAGNET_BACK, TX_MAGNET_BACK, BAUDRATE);
        setupUltrasonikWithParams(RX_ULTRASONIK_BACK, TX_ULTRASONIK_BACK, BAUDRATE);
        modeMaju = false;
        modeMundur = true;
    }
}

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
    Serial.println(F("Mengirim permintaan pembacaan..."));
    
    static int consecutiveFailures = 0; // Track consecutive communication failures
    
    uint8_t result = node.readHoldingRegisters(0x0000, 2);

    if (result == node.ku8MBSuccess)
    {
        // Reset consecutive failures counter on successful communication
        consecutiveFailures = 0;
        
        uint16_t medianValue = node.getResponseBuffer(0);
        uint16_t positionValue = node.getResponseBuffer(1);

        printActiveSegmentsFromBitmask(positionValue);
        updateJumlahMagnet(positionValue);

        if (positionValue == 0xFFFF)
        {
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
        }
    }
    else
    {
        Serial.print(F("Gagal membaca data sensor. Error Code: 0x"));
        Serial.println(result, HEX);
        
        // Count consecutive failures
        consecutiveFailures++;
        Serial.println(consecutiveFailures);
        // If too many consecutive failures, trigger system error
        // if (consecutiveFailures >= 30) {
        //     error(ERROR_SENSOR_COMMUNICATION, "Sensor Modbus Gagal 30x berturut-turut");
        // }
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
}

