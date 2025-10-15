import serial
import time

def monitor_esp32():
    try:
        # Connect to ESP32
        ser = serial.Serial('COM9', 115200, timeout=1)
        print("Connected to ESP32 on COM9")
        print("Monitoring auto-tuning process...")
        print("=" * 60)
        
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"[{time.strftime('%H:%M:%S')}] {line}")
                    
    except serial.SerialException as e:
        print(f"Error: {e}")
        print("Make sure ESP32 is connected to COM9 and not used by other applications")
    except KeyboardInterrupt:
        print("\nMonitoring stopped")
    finally:
        if 'ser' in locals():
            ser.close()

if __name__ == "__main__":
    monitor_esp32()