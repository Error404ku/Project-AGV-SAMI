# 🎨 SOLUSI LCD FLICKERING - RPM DISPLAY

## 🔴 **MASALAH: LAYAR BERKEDIP-KEDIP**

### Penyebab Flicker:

#### Before (Flickering Version):
```cpp
// Update display every 300ms
if (currentTime - lastDisplayUpdate >= 300) {
  lcd.setCursor(3, 3);
  lcd.print("                 ");  // ❌ Clear with spaces → FLICKER!
  lcd.setCursor(3, 3);
  lcd.print("R:");
  lcd.print(currentRpmKanan);
  lcd.print(" L:");
  lcd.print(currentRpmKiri);
  lastDisplayUpdate = currentTime;
}
```

### Mengapa Berkedip?

1. **Clear kemudian Write**
   ```
   Frame 1: "                 " (blank → mata lihat kosong)
   Frame 2: "R:40 L:38"        (data muncul)
   Frame 3: "                 " (blank lagi)
   Frame 4: "R:40 L:38"        (data muncul lagi)
   ```
   
2. **Update Setiap 300ms**
   - Update terjadi 3.3x per detik
   - Mata manusia sensitive terhadap perubahan 3-10 Hz
   - Result: **FLICKER TERLIHAT JELAS**

3. **LCD Refresh Time**
   - LCD I2C butuh ~5-10ms untuk update character
   - Clear (17 chars) + Write (9 chars) = ~26 chars × 0.5ms = **13ms blank!**
   - Mata menangkap blank period ini sebagai kedipan

## ✅ **SOLUSI: UPDATE HANYA SAAT NILAI BERUBAH**

### After (Smooth Version):
```cpp
void handleMotorTestRPM() {
  static unsigned long lastRpmRequest = 0;
  static int lastDisplayedRpmKanan = -999;  // Track last displayed
  static int lastDisplayedRpmKiri = -999;
  unsigned long currentTime = millis();
  
  // Request RPM data
  if (currentTime - lastRpmRequest >= 200) {
    requestRpmDataFromSlave();
    lastRpmRequest = currentTime;
  }
  
  // ✅ Update ONLY when values actually CHANGE
  if (currentRpmKanan != lastDisplayedRpmKanan || 
      currentRpmKiri != lastDisplayedRpmKiri) {
    
    // Build string with fixed width padding
    char rpmBuffer[20];
    snprintf(rpmBuffer, sizeof(rpmBuffer), "R:%-3d L:%-3d", 
             currentRpmKanan, currentRpmKiri);
    
    lcd.setCursor(3, 3);
    lcd.print(rpmBuffer);  // Single write, no clear!
    
    lastDisplayedRpmKanan = currentRpmKanan;
    lastDisplayedRpmKiri = currentRpmKiri;
  }
}
```

## 🎯 **TEKNIK ANTI-FLICKER**

### 1. **Change Detection**
```cpp
static int lastDisplayedRpmKanan = -999;
static int lastDisplayedRpmKiri = -999;

// Only update when data changes
if (currentRpmKanan != lastDisplayedRpmKanan || 
    currentRpmKiri != lastDisplayedRpmKiri) {
  // Update display
  lastDisplayedRpmKanan = currentRpmKanan;
  lastDisplayedRpmKiri = currentRpmKiri;
}
```

**Keuntungan:**
- ✅ No unnecessary updates
- ✅ Update hanya saat RPM berubah
- ✅ Drastis kurangi flicker

### 2. **Fixed Width Formatting dengan snprintf()**
```cpp
char rpmBuffer[20];
snprintf(rpmBuffer, sizeof(rpmBuffer), "R:%-3d L:%-3d", 
         currentRpmKanan, currentRpmKiri);
```

**Format Explanation:**
- `%-3d` = Left-aligned, 3 digits width
- Contoh:
  ```
  RPM = 40  → "R:40  L:38 "  (spasi otomatis pad)
  RPM = 5   → "R:5   L:3  "  (spasi otomatis pad)
  RPM = 100 → "R:100 L:99 "  (fit perfectly)
  ```

**Keuntungan:**
- ✅ Consistent string length
- ✅ No need to clear first
- ✅ Overwrite old characters smoothly

### 3. **Single Write Operation**
```cpp
lcd.setCursor(3, 3);
lcd.print(rpmBuffer);  // Write once, no clear!
```

**Before (2 operations):**
```
Operation 1: Clear → 13ms blank
Operation 2: Write → visible
Result: FLICKER!
```

**After (1 operation):**
```
Operation 1: Write (overwrite) → no blank period
Result: SMOOTH!
```

## 📊 **PERFORMANCE COMPARISON**

### Flickering Version:
| Metric | Value | Impact |
|--------|-------|--------|
| Updates per second | 3.3 | Fixed timing |
| Clear operations | 3.3/sec | Causes flicker |
| Write operations | 3.3/sec | After clear |
| Blank period | 13ms × 3.3 = 43ms/sec | **VISIBLE** |
| Flicker frequency | 3.3 Hz | In flicker range |
| User experience | ❌ Annoying | Eye strain |

### Smooth Version:
| Metric | Value | Impact |
|--------|-------|--------|
| Updates per second | Variable (only on change) | Efficient |
| Clear operations | 0 | **NO FLICKER** |
| Write operations | ~2-5/sec (when RPM changes) | Smooth |
| Blank period | 0ms | **INVISIBLE** |
| Flicker frequency | 0 Hz | No flicker |
| User experience | ✅ Smooth | Professional |

## 🔬 **TECHNICAL DEEP DIVE**

### LCD I2C Communication:
```
1 Character = I2C transaction
I2C Speed = 100kHz (standard) or 400kHz (fast)
Character time = ~0.5ms (100kHz) or ~0.2ms (400kHz)

Clear 17 chars + Write 9 chars = 26 chars total
Total time = 26 × 0.5ms = 13ms @ 100kHz
           = 26 × 0.2ms = 5.2ms @ 400kHz

Update frequency = 3.3 Hz
Blank time/sec = 13ms × 3.3 = 43ms/sec visible blank!
```

### Human Eye Perception:
```
Flicker fusion threshold: 50-60 Hz
Noticeable flicker range: 3-30 Hz
Our update rate: 3.3 Hz → VERY NOTICEABLE!

Solution: Reduce update frequency OR eliminate blank period
Our choice: Eliminate blank period ✓
```

## 🎨 **DISPLAY FORMAT EXAMPLES**

### Dynamic Padding:
```cpp
snprintf(rpmBuffer, sizeof(rpmBuffer), "R:%-3d L:%-3d", rpm_r, rpm_l);

Input          Output          Screen
----------------------------------------
R=0,   L=0  → "R:0   L:0  " → "   R:0   L:0  "
R=5,   L=3  → "R:5   L:3  " → "   R:5   L:3  "
R=40,  L=38 → "R:40  L:38 " → "   R:40  L:38 "
R=100, L=99 → "R:100 L:99 " → "   R:100 L:99 "
R=-20, L=15 → "R:-20 L:15 " → "   R:-20 L:15 "
```

### Consistent Width:
```
All strings are exactly 13 characters:
"R:xxx L:yyy "
 123456789012 3

This ensures:
✓ No leftover characters from previous values
✓ Clean overwrite
✓ No visual artifacts
```

## 🧪 **TESTING SCENARIOS**

### Test 1: Motor at Constant Speed
```
Expected: Display stays stable, no flicker
RPM doesn't change → No updates → Perfect!
```

### Test 2: Motor Accelerating
```
RPM: 0 → 10 → 20 → 30 → 40
Updates: 5 times total
Each update: Smooth overwrite
Result: No flicker during transition
```

### Test 3: Motor with Small Fluctuations
```
RPM: 40 → 41 → 40 → 41 → 40 (encoder noise)
Updates: Every change
Old method: Flicker 5× per second
New method: Smooth, no blank period
```

### Test 4: Rapid Direction Change
```
RPM: 40 → 0 → -40 → 0 → 40
Updates: 4 times
Old: Clear → write → FLICKER
New: Overwrite → SMOOTH
```

## 💡 **ADDITIONAL OPTIMIZATIONS**

### Option 1: Debouncing (if still slight flicker)
```cpp
if (abs(currentRpmKanan - lastDisplayedRpmKanan) > 2 || 
    abs(currentRpmKiri - lastDisplayedRpmKiri) > 2) {
  // Update only if change > 2 RPM
}
```

### Option 2: Rate Limiting (extra safety)
```cpp
static unsigned long lastUpdate = 0;
if ((currentRpmKanan != lastDisplayedRpmKanan) && 
    (millis() - lastUpdate > 100)) {  // Min 100ms between updates
  // Update display
  lastUpdate = millis();
}
```

### Option 3: Custom Characters (for maximum smoothness)
```cpp
// Define custom spinning wheel character
lcd.createChar(0, spinChar);
lcd.write(byte(0));  // Ultra-fast, no flicker
```

## 📈 **RESULTS**

### Before Fix:
```
User Report: "Layar berkedip-kedip"
Flicker Rate: 3.3 Hz
Blank Period: 43ms per second
User Experience: ❌ Annoying, hard to read
```

### After Fix:
```
User Report: Should report smooth display
Flicker Rate: 0 Hz
Blank Period: 0ms
User Experience: ✅ Professional, easy to read
```

## 🎯 **KEY TAKEAWAYS**

1. **Never clear before write on LCD**
   - Use overwrite with padding instead
   
2. **Update only on change**
   - Track last displayed value
   - Compare before update
   
3. **Use fixed-width formatting**
   - snprintf() with width specifiers
   - Consistent string length
   
4. **Single write operation**
   - Build string in buffer
   - Write once to LCD
   
5. **No time-based updates for display**
   - Event-driven (value change) is better
   - Eliminates unnecessary flicker

**RESULT: Display tampil SMOOTH tanpa kedipan!** ✨
