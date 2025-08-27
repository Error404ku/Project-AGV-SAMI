#!/bin/bash
# Script untuk menghapus semua Serial.print kecuali motor commands

# Files to clean
files=(
    "agv_sami/agv_sami.ino"
    "agv_sami/setup.ino"
    "agv_sami/wifi.ino"
    "agv_sami/menu.ino"
    "agv_sami/pidMagnetFollower.ino"
    "agv_sami/sensorRfid.ino"
    "agv_sami/sensorUltrasonik.ino"
    "agv_sami/performance_optimization.ino"
    "agv_sami/error.ino"
    "agv_sami/logicAgv.ino"
)

# Patterns to remove (keep motor_serial.ino Serial.print for commands)
patterns=(
    "Serial.print"
    "Serial.println"
    "Serial.printf"
)

# Replace with comments or empty lines
for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        echo "Cleaning $file..."
        
        # Comment out Serial.print statements
        sed -i 's/^[[:space:]]*Serial\.print.*$/\/\/ Debug output disabled for production/' "$file"
        sed -i 's/^[[:space:]]*Serial\.println.*$/\/\/ Debug output disabled for production/' "$file"
        sed -i 's/^[[:space:]]*Serial\.printf.*$/\/\/ Debug output disabled for production/' "$file"
        
        echo "Cleaned $file"
    else
        echo "File $file not found"
    fi
done

echo "All Serial debug output has been disabled for production."
echo "Only motor_serial.ino Serial.print commands are preserved."
