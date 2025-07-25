# Unified RS485 Communication System

## Overview
Sistem komunikasi RS485 terpadu untuk AGV SAMI yang menggunakan 1 jalur RS485 untuk berkomunikasi dengan 4 device sensor:
- **Address 1**: Magnet Sensor Depan
- **Address 2**: Ultrasonic Sensor Depan  
- **Address 3**: Ultrasonic Sensor Belakang
- **Address 4**: Magnet Sensor Belakang

## Hardware Configuration
- **RX Pin**: 18
- **TX Pin**: 17
- **Baudrate**: 9600
- **RS485 Control Pins**: DE=36, RE=36

## Key Changes

### 1. Configuration (config.h)
- Unified pin configuration: `RX_RS485 = 18`, `TX_RS485 = 17`
- Device addresses: `ADDR_MAGNET_FRONT = 1`, `ADDR_ULTRASONIC_FRONT = 2`, etc.
- Separate data arrays: `jumlahMagnetFront[16]`, `jumlahMagnetBack[16]`, `ultrasonicDistancesFront[5]`, `ultrasonicDistancesBack[5]`

### 2. New File: unified_rs485.ino
- **setupUnifiedRS485()**: Initialize communication system
- **loopUnifiedRS485()**: Main communication loop with device switching
- **communicateWithMagnetFront/Back()**: Modbus communication for magnet sensors
- **communicateWithUltrasonicFront/Back()**: Address switching for ultrasonic sensors
- **processUltrasonicData()**: Parse incoming ultrasonic packets
- **checkDeviceTimeouts()**: Monitor device connectivity

### 3. Updated Files

#### bacasensor.ino
- `bacaSensorGaris()` now calls `loopUnifiedRS485()`
- `updateTotalSensorAktif()` uses `getCurrentMagnetData()`
- Legacy `bacaSensor()` functionality moved to unified system

#### pembacaanUltrasonik.ino
- `loopUltrasonik()` and `parsePacket()` are now compatibility stubs
- New functions: `getUltrasonicDistancesFront/Back()`, `hasObstacle()`
- Obstacle detection supports both front and back sensors

#### setup.ino
- `setupAll()` now calls `setupUnifiedRS485()`
- Individual sensor setup functions are compatibility stubs

#### pembacaanPos.ino
- `logicAgv()` uses `getCurrentMagnetData()` instead of global `jumlahMagnet[]`

#### menu.ino
- Magnet check uses `getCurrentMagnetData()`
- Ultrasonic check uses `getUltrasonicData(true)` and `hasObstacle()`

#### display.ino
- `displaySensorData()` uses `getCurrentMagnetData()`

## Communication Flow

1. **Device Switching**: System cycles through devices every 100ms
2. **Magnet Sensors**: Use Modbus protocol with `readHoldingRegisters()`
3. **Ultrasonic Sensors**: Send data automatically, system parses by address
4. **Error Handling**: Device timeout detection and error reporting
5. **Data Storage**: Separate arrays for front/back sensors

## API Functions

### Data Access
- `getCurrentMagnetData()`: Get current magnet data (front by default)
- `getMagnetData(bool useFront)`: Get specific magnet data
- `getUltrasonicData(bool useFront)`: Get specific ultrasonic data
- `hasObstacle(bool checkFront)`: Check for obstacles

### Status Monitoring
- `isDeviceOnline(int deviceIndex)`: Check device connectivity
- `getDeviceStatusString()`: Get formatted status string

### Communication Control
- `setupUnifiedRS485()`: Initialize system
- `loopUnifiedRS485()`: Main communication loop

## Device Status Monitoring

The system tracks each device's online status and last successful communication time:
- **Device 0**: Magnet Front
- **Device 1**: Ultrasonic Front
- **Device 2**: Ultrasonic Back
- **Device 3**: Magnet Back

Timeout period: 5 seconds

## Backward Compatibility

Legacy functions are maintained as compatibility stubs to ensure existing code continues to work:
- `setupSensorMagnet()`: Now handled by unified system
- `setupUltrasonikWithParams()`: Now handled by unified system
- `loopUltrasonik()`: Redirects to unified system
- `updateJumlahMagnet()`: Redirects to `updateMagnetData()`

## Benefits

1. **Simplified Wiring**: Single RS485 bus for all sensors
2. **Centralized Communication**: All sensor communication in one place
3. **Better Error Handling**: Individual device timeout detection
4. **Scalability**: Easy to add more devices with different addresses
5. **Debugging**: Centralized status monitoring
6. **Resource Efficiency**: Single serial port usage

## Usage Example

```cpp
// In setup()
setupUnifiedRS485();

// In loop()
loopUnifiedRS485();

// Get current sensor data
int* magnetData = getCurrentMagnetData();
bool obstacle = hasObstacle(true); // Check front sensors

// Check device status
if (!isDeviceOnline(0)) {
  // Handle magnet front offline
}

String status = getDeviceStatusString();
// Returns: "Devices: MF:OK UF:OK UB:ERR MB:OK"
```

## Troubleshooting

1. **Device Not Responding**: Check wiring and address configuration
2. **CRC Errors**: Verify data integrity and communication speed
3. **Timeout Errors**: Check device power and RS485 termination
4. **Address Conflicts**: Ensure each device has unique address

For detailed debugging, use `getDeviceStatusString()` to monitor real-time device status.