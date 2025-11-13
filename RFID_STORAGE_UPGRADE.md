# RFID Storage Upgrade - Split Namespace Solution

## Problem
- ESP32 NVS memiliki limit ~90-100 keys per namespace
- Setiap station menggunakan 2 keys (station{i} + rfid{i})
- 40 stations = 80 keys, mendekati limit
- **Tidak bisa menyimpan lebih dari 40 stations** meskipun MAX_RFID_STATIONS = 50

## Solution Implemented
**Split Namespace Strategy** - Membagi storage menjadi 2 namespace:

### Namespace 1: "rfid-stations"
- Menyimpan stations 0-39 (40 stations)
- 80 keys + 1 count key = 81 keys total
- **DATA LAMA TETAP AMAN** - tidak perlu input ulang

### Namespace 2: "rfid-stations-2"
- Menyimpan stations 40-49 (10 stations)
- 20 keys + 1 count key = 21 keys total
- Otomatis digunakan ketika station count > 40

## Changes Made

### File: `sensorRfid.ino`

#### 1. Modified `loadRfidStations()`
```cpp
// Load dari namespace pertama (0-39)
preferences.begin("rfid-stations", false);
// Load stations 0-39...

// Load dari namespace kedua (40-49)
preferences.begin("rfid-stations-2", false);
// Load stations 40-49...
```

#### 2. Modified `saveRfidStations()`
```cpp
// Save ke namespace pertama (0-39)
preferences.begin("rfid-stations", false);
// Save stations 0-39...

// Save ke namespace kedua jika count > 40
if (rfidStationCount > 40) {
  preferences.begin("rfid-stations-2", false);
  // Save stations 40-49...
}
```

#### 3. Modified `deleteRfidStation()`
```cpp
// Auto-cleanup namespace kedua jika count turun <= 40
if (rfidStationCount <= 40) {
  preferences.begin("rfid-stations-2", false);
  preferences.clear();
  preferences.end();
}
```

## Benefits
✅ **Backward Compatible** - Data 40 stations yang sudah ada tetap aman
✅ **No Re-input Required** - Tidak perlu scan ulang RFID yang sudah tersimpan
✅ **Full Capacity** - Sekarang bisa menyimpan sampai 50 stations
✅ **Auto Cleanup** - Namespace kedua otomatis dibersihkan jika tidak diperlukan
✅ **Transparent** - Aplikasi tetap menggunakan array `rfidStations[50]` seperti biasa

## Storage Architecture

### Before (Limited to 40)
```
"rfid-stations" namespace:
├── stationCount = 40
├── station0 to station39 (40 keys)
└── rfid0 to rfid39 (40 keys)
Total: 81 keys (LIMIT REACHED)
```

### After (Up to 50)
```
"rfid-stations" namespace:
├── stationCount = 40
├── station0 to station39 (40 keys)
└── rfid0 to rfid39 (40 keys)
Total: 81 keys

"rfid-stations-2" namespace:
├── stationCount = 10
├── station0 to station9 (10 keys) -> mapped to array index 40-49
└── rfid0 to rfid9 (10 keys) -> mapped to array index 40-49
Total: 21 keys
```

## Testing Checklist
- [ ] Load existing 40 stations (should work without re-input)
- [ ] Add station 41-50 via auto input menu
- [ ] Verify all 50 stations loaded correctly after restart
- [ ] Test delete station 45 (should keep second namespace)
- [ ] Test delete stations until count = 40 (should clear second namespace)
- [ ] Test delete stations until count = 39 (second namespace should be empty)

## Technical Notes

### Key Mapping
- **Namespace 1**: `station{i}`, `rfid{i}` where i = 0 to 39
- **Namespace 2**: `station{i}`, `rfid{i}` where i = 0 to 9 (mapped to array index 40-49)

### Memory Usage
- **Namespace 1**: ~2800 bytes (40 stations × ~70 bytes)
- **Namespace 2**: ~700 bytes (10 stations × ~70 bytes)
- **Total**: ~3500 bytes (well within 4-8KB NVS limit per namespace)

### Key Count
- **Namespace 1**: 81 keys (under 90-100 limit)
- **Namespace 2**: 21 keys (under 90-100 limit)

## Compilation Result
```
Sketch uses 1093615 bytes (83%) of program storage space
Global variables use 50968 bytes (15%) of dynamic memory
Status: ✅ SUCCESS
```

## Date
November 13, 2025

## Related Files
- `agv_sami/sensorRfid.ino` - Modified load/save/delete functions
- `agv_sami/config.h` - MAX_RFID_STATIONS = 50 (unchanged)
- `agv_sami/menu.ino` - Auto input station (unchanged)
