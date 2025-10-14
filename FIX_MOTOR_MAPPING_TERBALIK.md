# Fix: Motor Mapping Terbalik pada Auto-Tuning

## 🐛 **Masalah yang Ditemukan**

### **Gejala:**

- ❌ **Tuning motor kanan** → yang bergerak **motor kiri**
- ❌ **Tuning motor kiri** → yang bergerak **motor kiri** (benar)
- ❌ **Tuning both motors** → hanya **motor kiri** yang bergerak

### **Root Cause:**

**Motor mapping terbalik** di fungsi `rpmMotor()` dalam file `ESP32_Motor_Controller_Slave/rpm.ino`

## 🔍 **Analysis**

### **Mapping yang Salah (sebelum fix):**

```cpp
// File: rpm.ino line 82-83
setMotorSpeed(1, pwmKiri);   // Motor 1 = Kanan ← SALAH!
setMotorSpeed(2, pwmKanan);  // Motor 2 = Kiri  ← SALAH!
```

### **Flow Auto-Tuning yang Terganggu:**

```
1. Master: "TUNE_RIGHT" → Slave
2. Slave: currentTuningTarget = TUNE_RIGHT
3. setTuningTargetRPM(40) → rpmMotor(40, 0)  // rpm1=40, rpm2=0
4. rpmMotor(): pwmKanan dihitung untuk rpm1=40
5. rpmMotor(): pwmKiri dihitung untuk rpm2=0
6. ERROR: setMotorSpeed(1, pwmKiri=0) → Motor kanan tidak bergerak!
7. ERROR: setMotorSpeed(2, pwmKanan=40) → Motor kiri bergerak!
```

## ✅ **Solusi yang Diimplementasikan**

### **Motor Mapping yang Benar:**

```cpp
// File: rpm.ino line 82-83 (FIXED)
setMotorSpeed(1, pwmKanan);  // Motor 1 = Kanan ✅
setMotorSpeed(2, pwmKiri);   // Motor 2 = Kiri  ✅
```

### **Flow Auto-Tuning yang Benar (setelah fix):**

```
1. Master: "TUNE_RIGHT" → Slave
2. Slave: currentTuningTarget = TUNE_RIGHT
3. setTuningTargetRPM(40) → rpmMotor(40, 0)  // rpm1=40, rpm2=0
4. rpmMotor(): pwmKanan dihitung untuk rpm1=40
5. rpmMotor(): pwmKiri dihitung untuk rpm2=0
6. FIXED: setMotorSpeed(1, pwmKanan=40) → Motor kanan bergerak! ✅
7. FIXED: setMotorSpeed(2, pwmKiri=0) → Motor kiri diam! ✅
```

## 🧪 **Testing Expected Results**

### **Test Case 1: Tune Right Motor**

```
Command: "TUNE_RIGHT"
Expected: Hanya motor kanan bergerak (RPM ~40)
          Motor kiri diam (RPM ~0)
```

### **Test Case 2: Tune Left Motor**

```
Command: "TUNE_LEFT"
Expected: Hanya motor kiri bergerak (RPM ~40)
          Motor kanan diam (RPM ~0)
```

### **Test Case 3: Tune Both Motors**

```
Command: "TUNE_BOTH"
Expected: Kedua motor bergerak (RPM ~40 each)
```

## 📋 **Verification Steps**

### **1. Upload Fixed Firmware**

```bash
arduino-cli upload --fqbn esp32:esp32:esp32s3 --port COMX .
```

### **2. Test Manual RPM Commands**

```
Master → Slave: "RPM40,0"  (Motor kanan 40 RPM, kiri 0)
Expected: Hanya motor kanan bergerak

Master → Slave: "RPM0,40"  (Motor kanan 0, kiri 40 RPM)
Expected: Hanya motor kiri bergerak

Master → Slave: "RPM40,40" (Kedua motor 40 RPM)
Expected: Kedua motor bergerak dengan RPM sama
```

### **3. Test Auto-Tuning Commands**

```
Master Menu: "Tune Right Motor"
Expected: Hanya motor kanan yang oscillating untuk tuning

Master Menu: "Tune Left Motor"
Expected: Hanya motor kiri yang oscillating untuk tuning

Master Menu: "Tune Both Motors"
Expected: Kedua motor oscillating bersamaan
```

## 🔧 **Technical Details**

### **Function Signature:**

```cpp
void rpmMotor(float rpm1, float rpm2)
// rpm1 = Target RPM untuk motor kanan
// rpm2 = Target RPM untuk motor kiri
```

### **Hardware Mapping:**

```cpp
setMotorSpeed(1, pwm) → Motor Index 1 → Motor Kanan
setMotorSpeed(2, pwm) → Motor Index 2 → Motor Kiri
```

### **PID Configuration:**

```cpp
pwmKanan = computePID(0, rpm1, rpm_depan_kanan, kpRight, kiRight, kdRight, ...)
pwmKiri  = computePID(2, rpm2, rpm_depan_kiri,  kpLeft,  kiLeft,  kdLeft,  ...)
```

## 📊 **Impact Analysis**

### **Before Fix:**

- ❌ Auto-tuning menghasilkan PID yang salah untuk motor yang dimaksud
- ❌ Performa AGV tidak optimal karena PID motor kanan/kiri tidak sesuai
- ❌ Debugging sulit karena perilaku yang tidak konsisten

### **After Fix:**

- ✅ Auto-tuning menghasilkan PID yang tepat untuk setiap motor
- ✅ Performa AGV optimal dengan PID yang sesuai karakteristik masing-masing motor
- ✅ Debugging mudah dengan perilaku yang konsisten dan predictable

## 🚨 **Regression Testing**

Pastikan test semua fungsi motor setelah fix:

### **Manual Motor Control:**

- ✅ `sendRPM(40, 0)` dari master
- ✅ `sendRPM(0, 40)` dari master
- ✅ `sendRPM(40, 40)` dari master

### **Auto-Tuning:**

- ✅ Tune Right Motor only
- ✅ Tune Left Motor only
- ✅ Tune Both Motors

### **Line Following:**

- ✅ Test basic line following untuk memastikan motor response benar
- ✅ Check PID effectiveness setelah auto-tuning

---

## **Status:** ✅ **FIXED & READY FOR TESTING**

**Motor mapping sudah diperbaiki dan siap untuk field testing!**

**Next Steps:**

1. Upload firmware ke ESP32 slave
2. Test manual RPM commands
3. Test auto-tuning individual motors
4. Verify line following performance
