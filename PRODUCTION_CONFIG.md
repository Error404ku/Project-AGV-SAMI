# Production Configuration - Serial0 untuk Motor Only

## Perubahan untuk Production Mode

### 1. Disable Debug Output
Semua debug Serial.print() telah dinonaktifkan untuk production.

### 2. Serial0 Configuration
```cpp
// Di motor_serial.ino
void setupMotorSerial() {
  Serial.begin(115200);  // Serial0 hanya untuk motor commands
  delay(1000);
}

void kirimPerintahMotor(int speedKiri, int speedKanan) {
  speedKiri = constrain(speedKiri, -4095, 4095);
  speedKanan = constrain(speedKanan, -4095, 4095);
  
  String perintah = "L" + String(speedKiri) + "R" + String(speedKanan) + "\n";
  Serial.print(perintah);  // Hanya kirim perintah, no debug
}
```

### 3. Hardware Connection
```
ESP32 Master          ESP32 Motor Controller
Pin 1 (TX) ----------> Pin 41 (RX)
Pin 3 (RX) ----------> Pin 42 (TX)  
GND -----------------> GND
```

### 4. Debug Flags Disabled
```cpp
// Di config.h
#define DEBUG_PID 0
#define DEBUG_SENSOR 0  
#define DEBUG_MOTOR 0
#define DEBUG_SETUP 0
#define DEBUG_WIFI 0
#define DEBUG_PERFORMANCE 0
```

### 5. Verification Steps
1. Upload ESP32_Motor_Controller_Slave.ino ke ESP32 kedua
2. Upload project AGV (dengan debug disabled) ke ESP32 pertama
3. Cek Serial Monitor ESP32 Motor Controller - harus menerima perintah L[speed]R[speed]
4. Test motor movement di AGV mode atau PID mode

### 6. Expected Serial Output (Motor Controller)
```
ESP32 Motor Controller Ready
Waiting for commands from Master ESP32...
Received command: L0R0
All motors stopped
Received command: L2000R2000  
Motor speeds set - Left: 2000, Right: 2000
```

## Files Modified:
- ✅ agv_sami.ino (debug disabled)
- ✅ motor_serial.ino (clean Serial0 output)
- ✅ config.h (debug flags disabled)
- ✅ setup.ino (clean setup functions)

Serial0 sekarang HANYA digunakan untuk mengirim perintah motor, tidak ada debug output lagi.
