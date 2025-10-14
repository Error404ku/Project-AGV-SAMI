# Debug Motor Mapping - Analisis Lengkap

## 🔍 **Hardware Definition (motor.ino)**

```cpp
void setMotorSpeed(int motor, int speed) {
  if (motor == 1) { //kiri     ← Motor 1 = KIRI (Hardware)
    // MOTOR1_D1, MOTOR1_D2, MOTOR1_PWM
  } else if (motor == 2) { //kanan  ← Motor 2 = KANAN (Hardware)
    // MOTOR2_D1, MOTOR2_D2, MOTOR2_PWM
  }
}
```

**Hardware Mapping:**

- **Motor 1** = KIRI (pins: 5, 4, 6)
- **Motor 2** = KANAN (pins: 7, 15, 16)

## 🔄 **Function rpmMotor(rpm1, rpm2)**

```cpp
void rpmMotor(float rpm1, float rpm2) {
  // rpm1 → pwmKanan (untuk motor kanan)
  // rpm2 → pwmKiri  (untuk motor kiri)

  // Current implementation (CORRECTED):
  setMotorSpeed(1, pwmKiri);   // Motor 1 = Kiri ✅
  setMotorSpeed(2, pwmKanan);  // Motor 2 = Kanan ✅
}
```

**Parameter Mapping:**

- **rpm1** = Target RPM motor KANAN
- **rpm2** = Target RPM motor KIRI

## 🎯 **Auto-Tuning Flow**

### **Tune Right Motor:**

```
1. Command: "TUNE_RIGHT"
2. startAutoTuningRight() → startAutoTuningGeneric(TUNE_RIGHT)
3. currentTuningTarget = TUNE_RIGHT
4. setTuningTargetRPM(40) with TUNE_RIGHT:
   case TUNE_RIGHT:
     rpmMotor(targetRPM, 0);  // rpmMotor(40, 0)
5. rpmMotor(40, 0):
   - rpm1 = 40 → pwmKanan = PID_result
   - rpm2 = 0  → pwmKiri = 0
6. setMotorSpeed(1, pwmKiri=0);   // Motor 1 (Kiri) = STOP ✅
   setMotorSpeed(2, pwmKanan=PID); // Motor 2 (Kanan) = BERGERAK ✅
```

### **Tune Left Motor:**

```
1. Command: "TUNE_LEFT"
2. startAutoTuningLeft() → startAutoTuningGeneric(TUNE_LEFT)
3. currentTuningTarget = TUNE_LEFT
4. setTuningTargetRPM(40) with TUNE_LEFT:
   case TUNE_LEFT:
     rpmMotor(0, targetRPM);  // rpmMotor(0, 40)
5. rpmMotor(0, 40):
   - rpm1 = 0  → pwmKanan = 0
   - rpm2 = 40 → pwmKiri = PID_result
6. setMotorSpeed(1, pwmKiri=PID); // Motor 1 (Kiri) = BERGERAK ✅
   setMotorSpeed(2, pwmKanan=0);  // Motor 2 (Kanan) = STOP ✅
```

## 🧪 **Testing Commands**

### **Manual Test 1: Right Motor Only**

```
Serial Command: "RPM40,0"
Expected: Motor kanan bergerak 40 RPM, motor kiri diam

Flow:
1. rpmMotor(40, 0)
2. setMotorSpeed(1, 0);   // Motor kiri diam
3. setMotorSpeed(2, PID); // Motor kanan bergerak
```

### **Manual Test 2: Left Motor Only**

```
Serial Command: "RPM0,40"
Expected: Motor kiri bergerak 40 RPM, motor kanan diam

Flow:
1. rpmMotor(0, 40)
2. setMotorSpeed(1, PID); // Motor kiri bergerak
3. setMotorSpeed(2, 0);   // Motor kanan diam
```

### **Manual Test 3: Both Motors**

```
Serial Command: "RPM40,40"
Expected: Kedua motor bergerak 40 RPM

Flow:
1. rpmMotor(40, 40)
2. setMotorSpeed(1, PID); // Motor kiri bergerak
3. setMotorSpeed(2, PID); // Motor kanan bergerak
```

## 🔧 **Consistency Check**

### **File: serial.ino (PWM Commands)**

```cpp
// Format: L<left>R<right>
setMotorSpeed(1, leftSpeed);   // Motor 1 = Kiri ✅
setMotorSpeed(2, rightSpeed);  // Motor 2 = Kanan ✅
```

### **File: rpm.ino (RPM Commands)**

```cpp
// Format: rpmMotor(rpm_kanan, rpm_kiri)
setMotorSpeed(1, pwmKiri);   // Motor 1 = Kiri ✅
setMotorSpeed(2, pwmKanan);  // Motor 2 = Kanan ✅
```

### **File: pid_fixed.ino (Alternative)**

```cpp
// Format: rpmMotor_fixed(rpm_kanan, rpm_kiri)
setMotorSpeed(1, pwmKiri);   // Motor 1 = Kiri ✅
setMotorSpeed(2, pwmKanan);  // Motor 2 = Kanan ✅
```

## 🐛 **Potential Issues**

### **Issue 1: Command Parsing di Master**

Cek apakah master mengirim command yang benar:

```cpp
// Apakah master mengirim:
"TUNE_RIGHT" untuk tuning motor kanan? ✅
"TUNE_LEFT"  untuk tuning motor kiri?  ✅
```

### **Issue 2: Encoder/RPM Reading**

```cpp
// Apakah rpm_depan_kanan dan rpm_depan_kiri dibaca dari encoder yang benar?
getTargetRPM():
  case TUNE_RIGHT: return rpm_depan_kanan;  // ✅
  case TUNE_LEFT:  return rpm_depan_kiri;   // ✅
```

### **Issue 3: PID Configuration**

```cpp
// Apakah PID menggunakan parameter yang benar?
Motor kanan: pidConfigRight (kpRight, kiRight, kdRight) ✅
Motor kiri:  pidConfigLeft  (kpLeft,  kiLeft,  kdLeft)  ✅
```

## 📊 **Debug Output yang Diharapkan**

### **Saat TUNE_RIGHT:**

```
Received command: TUNE_RIGHT
Auto-tuning motor kanan dimulai atas permintaan master
MEMULAI AUTO-TUNING PID RPM - Motor Kanan
Target RPM: 40
Error RPM (double) - Kanan: <nilai>, Kiri: 0.000
```

### **Saat TUNE_LEFT:**

```
Received command: TUNE_LEFT
Auto-tuning motor kiri dimulai atas permintaan master
MEMULAI AUTO-TUNING PID RPM - Motor Kiri
Target RPM: 40
Error RPM (double) - Kanan: 0.000, Kiri: <nilai>
```

## 🎯 **Expected Final Behavior**

| Command    | Motor Kanan          | Motor Kiri           |
| ---------- | -------------------- | -------------------- |
| TUNE_RIGHT | Bergerak/Oscillating | Diam                 |
| TUNE_LEFT  | Diam                 | Bergerak/Oscillating |
| TUNE_BOTH  | Bergerak/Oscillating | Bergerak/Oscillating |

---

## **Status Mapping:** ✅ **SEHARUSNYA BENAR SEKARANG**

**All mappings consistent dengan hardware definition di motor.ino**
