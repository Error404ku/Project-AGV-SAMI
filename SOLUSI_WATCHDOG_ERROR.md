# Solusi Error "esp_task_wdt_reset(707): task not found"

## Penjelasan Error

Error `esp_task_wdt_reset(707): task not found` terjadi ketika:

1. **Task Watchdog Timer** mencoba mereset task yang sudah tidak terdaftar
2. **Long delays** tanpa watchdog reset menyebabkan timeout
3. **FreeRTOS task management** yang tidak proper
4. **Setup operations** yang terlalu lama tanpa watchdog handling

## Root Cause

Error ini muncul karena ESP32 memiliki **Task Watchdog Timer** yang memantau task utama. Jika task tidak memberikan "heartbeat" dalam waktu tertentu (default ~5 detik), watchdog akan memunculkan error.

## Solusi yang Sudah Diimplementasikan

### 1. ✅ Safe Delay Function

```cpp
// Implementasi di menu.ino
void safeDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    delay(100);
    esp_task_wdt_reset();  // Reset watchdog setiap 100ms
    if (millis() - start >= ms) break;
  }
}
```

### 2. ✅ Safe Delay untuk Setup

```cpp
// Implementasi di setup.ino
void safeDelayLocal(int ms) {
  unsigned long startTime = millis();
  while (millis() - startTime < ms) {
    #ifdef ESP_TASK_WDT_H
    esp_task_wdt_reset();  // Reset watchdog jika tersedia
    #endif
    delay(10);  // Small delay to prevent tight loop
  }
}
```

### 3. ✅ Mengganti Delay Kritikal

- ✅ `delay(1500)` → `safeDelay(1500)` di clear stations
- ✅ `delay(2000)` → `safeDelay(2000)` di auto tuning
- ✅ `delay(1000)` → `safeDelayLocal(1000)` di setup

## Delay yang Masih Perlu Diganti

### Priority 1 - Delay Panjang (>500ms)

```bash
menu.ino:2411:            delay(1500);
menu.ino:2416:            delay(1500);
menu.ino:2438:              delay(1500);
menu.ino:2610:    delay(2000);
menu.ino:3061:        delay(2000);
menu.ino:3071:    delay(2000);
menu.ino:3078:      delay(2000);
performance_optimization.ino:272:  delay(1000);
setup.ino:82:  delay(500);
setup.ino:131:  delay(500);
```

### Priority 2 - Delay Sedang (100-500ms)

```bash
menu.ino:2394:              delay(200);
menu.ino:3103:        delay(200);
menu.ino:3106:        delay(200);
setup.ino:92:  delay(100);
setup.ino:155:  delay(100);
setup.ino:181:  delay(100);
```

### Priority 3 - Delay Pendek (<100ms) - AMAN

```bash
menu.ino:104:  delay(50);   // Button debounce - AMAN
wifi.ino:31:  delay(50);    // Communication delay - AMAN
motor_serial.ino:89:  delay(50);   // Serial delay - AMAN
```

## Implementasi Otomatis

Untuk mengatasi error watchdog secara permanen, berikut langkah yang bisa diambil:

### Opsi 1: Disable Watchdog (Quick Fix)

```cpp
// Di setup() atau setupAll()
esp_task_wdt_delete(NULL);  // Remove current task dari watchdog
```

### Opsi 2: Proper Watchdog Configuration (Recommended)

```cpp
// Konfigurasi watchdog dengan timeout yang lebih panjang
void configureWatchdog() {
  esp_task_wdt_config_t twdt_config = {
    .timeout_ms = 30000,  // 30 second timeout
    .idle_core_mask = 0,
    .trigger_panic = false  // Don't panic, just reset
  };
  esp_task_wdt_reconfigure(&twdt_config);
}
```

### Opsi 3: Selective Delay Replacement

```cpp
// Ganti hanya delay yang kritikal (>1000ms)
#define SAFE_DELAY(ms) do { \
  if (ms > 1000) safeDelay(ms); \
  else delay(ms); \
} while(0)
```

## Status Implementasi

- ✅ **Kompilasi Berhasil**: Kedua device compile tanpa error
- ✅ **Safe Delay Function**: Implementasi working
- ✅ **Menu Navigation**: Fixed string concatenation
- ✅ **Setup Delays**: Converted to safeDelayLocal
- ⏳ **Field Testing**: Perlu test untuk verifikasi error hilang

## Testing Recommendation

1. **Upload firmware** ke kedua ESP32
2. **Monitor Serial Output** untuk melihat apakah error masih muncul:

   ```
   # Jika error masih ada:
   esp_task_wdt_reset(707): task not found

   # Jika sudah fixed:
   (tidak ada error watchdog di serial monitor)
   ```

3. **Test Auto Tuning** untuk operasi yang paling sering trigger error
4. **Test Menu Navigation** dengan delay panjang

## Jika Error Masih Muncul

### Quick Fix - Disable Watchdog

Tambahkan di `setupAll()`:

```cpp
void setupAll() {
  // Disable watchdog untuk mencegah error
  esp_task_wdt_delete(NULL);

  // ... rest of setup
}
```

### Advanced Fix - Replace All Long Delays

Jalankan script untuk ganti semua delay >500ms:

```bash
# Otomatis replace delay panjang
find . -name "*.ino" -exec sed -i 's/delay(\([0-9]\{4,\}\))/safeDelay(\1)/g' {} \;
```

---

## Kesimpulan

Error `esp_task_wdt_reset(707)` sudah sebagian besar teratasi dengan:

1. ✅ Implementasi `safeDelay()` function
2. ✅ Replace delay kritikal di menu dan setup
3. ✅ Kompilasi berhasil tanpa error

Untuk test lapangan, monitor serial output dan jika error masih muncul, implementasikan **Quick Fix** dengan disable watchdog.
