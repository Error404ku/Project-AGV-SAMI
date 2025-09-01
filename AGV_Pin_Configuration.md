# AGV SAMI - Pin Configuration Summary

## System Overview

Sistem AGV SAMI menggunakan **2 ESP32** dalam konfigurasi Master-Slave:

- **ESP32 Master (AGV_SAMI)**: Kontrol utama, sensor, UI, komunikasi
- **ESP32 Slave (Motor Controller)**: Dedicated motor control dengan encoder

---

## ESP32 Master (AGV_SAMI) - Pin Configuration

### 🎮 Button Interface

| Pin | Function | Description                     |
| --- | -------- | ------------------------------- |
| 10  | UP       | Navigate up in menu             |
| 42  | LEFT     | Navigate left / decrease value  |
| 39  | RIGHT    | Navigate right / increase value |
| 40  | DOWN     | Navigate down in menu           |
| 9   | START    | Select/Enter/Confirm            |
| 41  | STOP     | Back/Cancel/Exit                |
| 0   | BOOT     | Boot button (system use)        |

### 🖥️ Display Interface

| Pin | Function | Description                       |
| --- | -------- | --------------------------------- |
| 3   | SDA      | I2C Data for LCD 20x4             |
| 8   | SCL      | I2C Clock for LCD 20x4            |
| -   | LCD      | Address: 0x27, 20 columns, 4 rows |

### 🚗 Motor Control Interface

| Pin | Function | Description             |
| --- | -------- | ----------------------- |
| 48  | IN1      | Motor Kanan Direction 1 |
| 38  | IN2      | Motor Kanan Direction 2 |
| 4   | IN3      | Motor Kiri Direction 1  |
| 5   | IN4      | Motor Kiri Direction 2  |
| 35  | ENA      | Motor Kanan Enable/PWM  |
| 6   | ENB      | Motor Kiri Enable/PWM   |

### 📡 RS485 Communication

| Pin | Function     | Description                     |
| --- | ------------ | ------------------------------- |
| 36  | MAX485_DE/RE | Direction/Receive Enable        |
| 18  | RS485_RX     | Serial1 RX (Magnet Sensors)     |
| 17  | RS485_TX     | Serial1 TX (Magnet Sensors)     |
| 11  | RS485_RX2    | Serial2 RX (Ultrasonic Sensors) |
| 46  | RS485_TX2    | Serial2 TX (Ultrasonic Sensors) |

### 🏷️ RFID Reader (Wiegand)

| Pin | Function | Description    |
| --- | -------- | -------------- |
| 12  | PIN_D0   | Wiegand Data 0 |
| 13  | PIN_D1   | Wiegand Data 1 |

### 🎵 Music/Buzzer Control

| Pin | Function  | Description              |
| --- | --------- | ------------------------ |
| 7   | pinMusic1 | Music Channel 1          |
| 15  | pinMusic2 | Music Channel 2          |
| 16  | pinMusic3 | Music Channel 3          |
| 14  | pinMusic4 | Music Channel 4          |
| 37  | pinMusic5 | Music Channel 5          |
| 2   | pinMusic6 | Music Channel 6 / Silent |

### 🪝 Hook Control System

| Pin | Function     | Description        |
| --- | ------------ | ------------------ |
| 20  | pinHook1     | Hook Relay 1       |
| 45  | pinHook2     | Hook Relay 2       |
| 21  | pinMotorHook | Hook Motor Control |

### 💡 Indicator Light

| Pin | Function | Description     |
| --- | -------- | --------------- |
| 47  | lampPin  | Status LED/Lamp |

### 🌐 WiFi Configuration

- **SSID**: "My Phone"
- **Password**: "kalolaparmakan"
- **Static IP**: 192.168.121.14
- **Gateway**: 192.168.121.99
- **Subnet**: 255.255.255.0
- **DNS**: 192.168.121.99

---

## ESP32 Slave (Motor Controller) - Pin Configuration

### 🚗 Motor 1 (Right Motor)

| Pin | Function   | Description         |
| --- | ---------- | ------------------- |
| 4   | MOTOR1_D1  | Direction Control 1 |
| 5   | MOTOR1_D2  | Direction Control 2 |
| 6   | MOTOR1_PWM | PWM Speed Control   |

### 🚗 Motor 2 (Left Motor)

| Pin | Function   | Description         |
| --- | ---------- | ------------------- |
| 7   | MOTOR2_D1  | Direction Control 1 |
| 15  | MOTOR2_D2  | Direction Control 2 |
| 16  | MOTOR2_PWM | PWM Speed Control   |

### 📏 Encoder Interface

| Pin | Function        | Description         |
| --- | --------------- | ------------------- |
| 39  | EncoderKananPin | Right Motor Encoder |
| 37  | EncoderKiriPin  | Left Motor Encoder  |

### 📡 Serial Communication with Master

| Pin | Function | Description               |
| --- | -------- | ------------------------- |
| 41  | RX_PIN   | Receive from Master ESP32 |
| 42  | TX_PIN   | Transmit to Master ESP32  |

### ⚙️ PWM Configuration

- **Frequency**: 5000 Hz
- **Resolution**: 12-bit (0-4095)
- **Channel 1**: Motor 1 PWM
- **Channel 2**: Motor 2 PWM

---

## Sensor Network (RS485 Slaves)

### 🧲 Magnet Sensors (Serial1)

| Slave ID | Location                | Function                 |
| -------- | ----------------------- | ------------------------ |
| 1        | SLAVEID_MAGNET_DEPAN    | Front magnet line sensor |
| 4        | SLAVEID_MAGNET_BELAKANG | Back magnet line sensor  |

### 📐 Ultrasonic Sensors (Serial2)

| Slave ID | Location                    | Function                 |
| -------- | --------------------------- | ------------------------ |
| 2        | SLAVEID_ULTRASONIK_DEPAN    | Front obstacle detection |
| 3        | SLAVEID_ULTRASONIK_BELAKANG | Back obstacle detection  |

---

## System Communication Architecture

```
ESP32 Master (AGV_SAMI)
├── LCD Display (I2C: Pin 3, 8)
├── Buttons (Pins: 9, 10, 39, 40, 41, 42)
├── RFID Reader (Wiegand: Pin 12, 13)
├── Music System (Pins: 2, 7, 14, 15, 16, 37)
├── Hook System (Pins: 20, 21, 45)
├── Status Lamp (Pin: 47)
├── RS485 Network 1 (Pins: 17, 18, 36) → Magnet Sensors
├── RS485 Network 2 (Pins: 11, 46) → Ultrasonic Sensors
├── Serial Communication → ESP32 Slave Motor Controller
└── WiFi → Web Server & HTTP API

ESP32 Slave (Motor Controller)
├── Motor 1 Control (Pins: 4, 5, 6)
├── Motor 2 Control (Pins: 7, 15, 16)
├── Encoder 1 (Pin: 39)
├── Encoder 2 (Pin: 37)
├── Serial Communication (Pins: 41, 42) → ESP32 Master
└── PID Control System
```

---

## Key Features

### 🎛️ Control Modes

- **Manual Control**: Button-based navigation
- **AGV Mode**: Autonomous line following
- **Motor Test**: PWM and RPM testing
- **Menu System**: Configuration interface

### 🔧 Configuration Options

- **PID Tuning**: Forward/Backward with/without mass
- **Motor Settings**: Speed, RPM, direction inversion
- **RFID Management**: Station, warehouse, terminal setup
- **Music Mapping**: Custom sound assignments
- **Network Setup**: WiFi and static IP configuration

### 📊 Monitoring

- **Real-time RPM**: From encoder feedback
- **Sensor Data**: Magnet line detection, obstacle distance
- **System Status**: Performance monitoring, error recovery
- **Web Interface**: Remote monitoring and control

---

## Safety Features

- **Obstacle Detection**: Ultrasonic sensors with configurable safe distances
- **Watchdog Timer**: System reliability monitoring
- **Error Recovery**: Automatic fault detection and recovery
- **Emergency Stop**: Immediate halt capability via STOP button

---

_Last Updated: August 29, 2025_  
_Project: AGV-SAMI_  
_Branch: PercobaanPWM_
