# AGV SAMI - Optimized Version

## Overview

This is the optimized and restructured version of the AGV SAMI (Autonomous Guided Vehicle) project. The codebase has been completely reorganized for better maintainability, readability, and performance.

## Project Structure

### Core Files

- **`agv_sami.ino`** - Main program entry point
- **`types.h`** - Data structures and enumerations
- **`config.h`** - Hardware configuration and global variables

### Functional Modules

- **`setup.ino`** - System initialization functions
- **`sensors.ino`** - Sensor reading and processing (RS485, magnet, ultrasonic)
- **`control.ino`** - Motor control and PID systems
- **`interface.ino`** - User interface (LCD, menu, web server)
- **`rfid.ino`** - RFID management and station control
- **`agv_logic.ino`** - Main AGV operational logic

## Key Improvements

### 1. **Modular Architecture**

- Separated concerns into logical modules
- Clear separation between hardware control and business logic
- Reduced code duplication

### 2. **Unified Communication System**

- Consolidated RS485 communication for all sensors
- Improved device management and timeout handling
- Better error handling and recovery

### 3. **Enhanced Safety Systems**

- Comprehensive obstacle detection
- Emergency stop procedures
- System health monitoring
- Device timeout detection

### 4. **Improved Configuration Management**

- Centralized configuration in `config.h`
- Persistent storage using Preferences
- Runtime configuration updates via web interface

### 5. **Better Error Handling**

- Structured error codes and logging
- Graceful degradation on sensor failures
- Comprehensive diagnostic functions

## Hardware Configuration

### Sensors

- **Magnet Sensors**: Front and back arrays (16 sensors each) via RS485
- **Ultrasonic Sensors**: Front and back arrays (3 sensors each) via RS485
- **RFID Reader**: Wiegand protocol for station identification

### Actuators

- **Motors**: L298N driver with PWM control
- **Hook System**: Relay-controlled with limit switches
- **Audio/Visual**: Buzzer and LED for feedback

### Communication

- **RS485**: Unified sensor communication
- **WiFi**: Access Point mode for web interface
- **I2C**: LCD display

## Operating Modes

### 1. **Idle Mode**

- AGV is stopped and waiting for commands
- All systems monitored but no autonomous movement

### 2. **Manual Mode**

- Direct control via web interface or buttons
- Safety systems remain active

### 3. **Line Follow Mode**

- Autonomous line following using magnet sensors
- PID-controlled movement with obstacle avoidance

### 4. **Station Mode**

- Line following with RFID station recognition
- Automatic station-specific actions

## Web Interface

Access the AGV control panel at: `http://192.168.4.1`

### Features

- Real-time system status
- Manual control buttons
- Mode switching
- Configuration updates
- Station management

## Configuration

### PID Parameters

- **Kp**: Proportional gain (default: 2.0)
- **Ki**: Integral gain (default: 0.1)
- **Kd**: Derivative gain (default: 1.0)

### Motor Settings

- **Base Speed**: Default movement speed (default: 150)
- **Max PWM**: Maximum motor power (default: 255)
- **Motor Inversions**: X/Y axis and individual motor inversions

### Safety Parameters

- **Min Safe Distance**: Obstacle detection threshold (default: 30cm)
- **Device Timeout**: Communication timeout (default: 5000ms)
- **Stuck Timeout**: Movement timeout (default: 10000ms)

## RFID Station Management

### Adding Stations

1. Navigate to RFID Setup menu
2. Scan RFID card
3. Associate with station ID
4. Configure station-specific actions

### Station Actions

- **Station 1**: Stop and lower hook
- **Station 2**: Raise hook and continue
- **Station 3**: Turn around
- **Custom**: User-defined actions

## Troubleshooting

### Common Issues

1. **No Sensor Data**

   - Check RS485 connections
   - Verify device addresses
   - Check power supply

2. **AGV Not Following Line**

   - Calibrate magnet sensors
   - Adjust PID parameters
   - Check sensor positioning

3. **RFID Not Working**

   - Verify Wiegand connections
   - Check RFID reader power
   - Test with known cards

4. **Web Interface Inaccessible**
   - Check WiFi AP status
   - Verify IP address (192.168.4.1)
   - Restart ESP32

### Diagnostic Commands

Use the serial monitor (115200 baud) for debugging:

- System automatically logs errors and status
- Health checks run every 30 seconds
- Sensor data continuously updated

## Development

### Building

1. Open `agv_sami.ino` in Arduino IDE
2. Select ESP32 board
3. Install required libraries:
   - LiquidCrystal_I2C
   - WiFi
   - WebServer
   - Preferences
   - ModbusMaster
   - Wiegand
4. Upload to ESP32

### Adding Features

1. Define new data structures in `types.h`
2. Add configuration in `config.h`
3. Implement functionality in appropriate module
4. Update web interface if needed

### Testing

- Use built-in diagnostic functions
- Test individual subsystems
- Verify safety systems
- Check communication reliability

## Safety Considerations

⚠️ **Important Safety Notes**

- Always test in a safe environment
- Ensure emergency stop is accessible
- Verify obstacle detection before operation
- Check all connections before powering on
- Monitor system health during operation

## License

This project is part of the AGV SAMI development initiative. Please refer to the main project repository for licensing information.

## Support

For technical support or questions:

1. Check the troubleshooting section
2. Review serial monitor output
3. Use diagnostic functions
4. Contact the development team

---

**Version**: 2.0 Optimized  
**Last Updated**: 2024  
**Compatible Hardware**: ESP32-based AGV SAMI platform
