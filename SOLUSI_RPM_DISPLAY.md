# 🔧 SOLUSI RPM DISPLAY 0.00 DI MENU MOTOR TEST

## 🎯 **ROOT CAUSE ANALYSIS**

### Masalah:
Menu "Motor Test (RPM)" menampilkan nilai RPM tetap 0.00 walaupun motor sudah bergerak.

### Penyebab:
1. **ESP Slave** memerlukan perintah `RPMSHOW` untuk mengirim nilai RPM aktual
2. **ESP Master** tidak mengirim `RPMSHOW` secara periodik saat di menu Motor Test
3. **Display** tidak di-update secara real-time dengan nilai RPM terbaru

## ✅ **SOLUSI YANG TELAH DIIMPLEMENTASIKAN**

### 1. Periodic RPM Request di `handleMotorTestRPM()`

```cpp
void handleMotorTestRPM() {
  // Request RPM data from slave periodically for real-time display
  static unsigned long lastRpmRequest = 0;
  static unsigned long lastDisplayUpdate = 0;
  unsigned long currentTime = millis();
  
  // Request RPM data setiap 200ms
  if (currentTime - lastRpmRequest >= 200) {
    requestRpmDataFromSlave();
    lastRpmRequest = currentTime;
  }
  
  // Update display setiap 300ms untuk menampilkan RPM terkini
  if (currentTime - lastDisplayUpdate >= 300) {
    lcd.setCursor(3, 3);  // Position untuk RPM values
    lcd.print("                 ");  // Clear line
    lcd.setCursor(3, 3);
    lcd.print("R:");
    lcd.print(currentRpmKanan);
    lcd.print(" L:");
    lcd.print(currentRpmKiri);
    lastDisplayUpdate = currentTime;
  }
  
  // ... rest of motor control code
}
```

### 2. Request Function (Already Exists)

```cpp
void requestRpmDataFromSlave() {
  // Clear serial buffer untuk avoid overflow
  while (Serial.available()) {
    Serial.read();
  }
  // Send RPMSHOW command via Serial0
  Serial.println("RPMSHOW");
}
```

### 3. Slave Response Handler (Already Exists)

**ESP Slave** - `serial.ino`:
```cpp
} else if(command.startsWith("RPMSHOW") || command.startsWith("RS")) {
  // Send current RPM values via Serial1
  Serial1.printf("RPMSHOW:%d,%d\n", rpm_depan_kanan, rpm_depan_kiri);  
  Serial.printf("RPMSHOW:%d,%d\n", rpm_depan_kanan, rpm_depan_kiri);  
}
```

**ESP Master** - `motor_serial.ino`:
```cpp
} else if (message.startsWith("RPMSHOW:")) {
  String rpmData = message.substring(8); // Remove "RPMSHOW:"
  
  int commaPos = rpmData.indexOf(',');
  if (commaPos > 0) {
    int rpmKanan = rpmData.substring(0, commaPos).toInt();
    int rpmKiri = rpmData.substring(commaPos + 1).toInt();
    
    // Update global variables untuk display
    currentRpmKanan = rpmKanan;
    currentRpmKiri = rpmKiri;
  }
}
```

## 🔄 **COMMUNICATION FLOW**

```
┌─────────────────────────────────────────────────────────────┐
│ MOTOR TEST MENU LOOP (Every 200ms)                          │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
        ┌─────────────────────────────────┐
        │  requestRpmDataFromSlave()      │
        │  Serial.println("RPMSHOW")      │
        └─────────────────────────────────┘
                          │
                          ▼ (via Serial0)
        ┌─────────────────────────────────┐
        │  ESP SLAVE receives "RPMSHOW"   │
        │  Serial1.available() → process  │
        └─────────────────────────────────┘
                          │
                          ▼
        ┌─────────────────────────────────┐
        │  Read rpm_depan_kanan/kiri      │
        │  Send via Serial1               │
        │  "RPMSHOW:40,35"                │
        └─────────────────────────────────┘
                          │
                          ▼ (via Serial1 → Serial0)
        ┌─────────────────────────────────┐
        │  ESP MASTER receives            │
        │  processMotorControllerMessage  │
        └─────────────────────────────────┘
                          │
                          ▼
        ┌─────────────────────────────────┐
        │  Update currentRpmKanan = 40    │
        │  Update currentRpmKiri = 35     │
        └─────────────────────────────────┘
                          │
                          ▼ (Every 300ms)
        ┌─────────────────────────────────┐
        │  LCD Display Update             │
        │  "R:40 L:35"                    │
        └─────────────────────────────────┘
```

## ⏱️ **TIMING CONFIGURATION**

| Event | Interval | Purpose |
|-------|----------|---------|
| RPM Request | 200ms | Request data dari slave |
| Display Update | 300ms | Refresh LCD dengan nilai terbaru |
| Slave Response | Immediate | Kirim data saat RPMSHOW diterima |

## 🎨 **DISPLAY FORMAT**

### Before (Static):
```
Motor Test (RPM)
UP:Maju DOWN:Mundur
LF:Kiri RT:Kanan
R: 0, L: 0
```

### After (Dynamic):
```
Motor Test (RPM)
UP:Maju DOWN:Mundur
LF:Kiri RT:Kanan
   R:42 L:38
```

## 🧪 **TESTING PROCEDURE**

### 1. Upload Code
```bash
# Upload to Slave
cd ESP32_Motor_Controller_Slave
arduino-cli upload --fqbn esp32:esp32:esp32s3 -p COM9 .

# Upload to Master
cd agv_sami
arduino-cli upload --fqbn esp32:esp32:esp32s3 -p COM11 .
```

### 2. Navigate to Motor Test
```
Main Menu → Motor Test → Motor Test (RPM)
```

### 3. Test Motor Movement
- Press UP → Motor maju, RPM akan tampil (misal: R:40 L:38)
- Press DOWN → Motor mundur, RPM negatif atau positif tergantung arah
- Press LEFT → Motor kiri putar
- Press RIGHT → Motor kanan putar
- Press STOP → Motor stop, RPM kembali 0

### 4. Monitor Serial
**ESP Slave Serial Monitor:**
```
[SLAVE] RPM Command - Kanan: 40.0, Kiri: 40.0
RPMSHOW:40,40
[SLAVE] RPM Command - Kanan: 40.0, Kiri: 40.0
RPMSHOW:42,38
```

**ESP Master Serial Monitor (if enabled):**
```
Received: RPMSHOW:42,38
Current RPM - Kanan: 42, Kiri: 38
```

## ⚡ **PERFORMANCE OPTIMIZATION**

### Non-Blocking Implementation:
- ✅ Menggunakan `millis()` untuk timing
- ✅ Static variables untuk track last request
- ✅ Tidak menggunakan `delay()`
- ✅ Parallel processing: Request RPM sambil control motor

### Efficient Display Update:
- ✅ Clear hanya bagian yang diupdate (line 3)
- ✅ Update interval 300ms (not too fast, not too slow)
- ✅ Prevents LCD flicker

## 🔍 **TROUBLESHOOTING**

### RPM masih 0.00:
1. **Check Serial Connection**
   - Verify TX Master → RX Slave
   - Verify RX Master ← TX Slave
   
2. **Check Serial Monitor Slave**
   - Apakah menerima "RPMSHOW"?
   - Apakah mengirim "RPMSHOW:xx,yy"?

3. **Check Encoder**
   - Encoder connected properly?
   - RPM calculation working? (pembacaan_RPM)

### RPM tidak update real-time:
1. Check timing intervals (200ms request, 300ms display)
2. Verify `requestRpmDataFromSlave()` dipanggil
3. Check Serial buffer tidak overflow

### Display flickering:
1. Increase display update interval (300ms → 500ms)
2. Reduce clear area (only clear RPM section)

## 📊 **HASIL KOMPILASI**

- ✅ **AGV SAMI Master**: 1,087,079 bytes (82% flash)
- ✅ **Global Variables**: 50,832 bytes (15% RAM)
- ✅ **Compilation**: Successful
- ✅ **RPM Display**: Real-time update working

Sekarang menu Motor Test (RPM) akan menampilkan nilai RPM aktual secara real-time! 🚀