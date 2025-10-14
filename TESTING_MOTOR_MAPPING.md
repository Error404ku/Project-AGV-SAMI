# Testing Protocol - Motor Mapping Verification

## 🧪 **Step-by-Step Testing**

### **Step 1: Basic Connectivity Test**

```bash
# Buka Serial Monitor untuk kedua ESP32
# Master ESP32 (agv_sami): Baudrate 115200
# Slave ESP32 (Motor Controller): Baudrate 115200
```

### **Step 2: Manual RPM Test**

**Tujuan:** Verifikasi bahwa mapping motor hardware benar

#### **Test 2.1: Right Motor Only**

```
Expected: Hanya motor KANAN yang bergerak
Command dari Serial Monitor Master: (tidak ada interface manual, skip ke auto-tuning)
```

#### **Test 2.2: Auto-Tuning Test**

```
1. Power on kedua ESP32
2. Tunggu "System Ready" di LCD Master
3. Masuk Menu → Performance → Auto Tuning
4. Pilih "Tune Right Motor"
5. Tekan tombol untuk start
```

**Expected Result untuk "Tune Right Motor":**

- ✅ **Hanya motor KANAN yang bergerak/oscillating**
- ✅ **Motor KIRI diam (tidak bergerak)**
- ✅ **Serial Slave menampilkan**: "Error RPM (double) - Kanan: [nilai], Kiri: 0.000"

#### **Test 2.3: Left Motor Tuning**

```
1. Masuk Menu → Performance → Auto Tuning
2. Pilih "Tune Left Motor"
3. Tekan tombol untuk start
```

**Expected Result untuk "Tune Left Motor":**

- ✅ **Hanya motor KIRI yang bergerak/oscillating**
- ✅ **Motor KANAN diam (tidak bergerak)**
- ✅ **Serial Slave menampilkan**: "Error RPM (double) - Kanan: 0.000, Kiri: [nilai]"

#### **Test 2.4: Both Motors Tuning**

```
1. Masuk Menu → Performance → Auto Tuning
2. Pilih "Tune Both Motors"
3. Tekan tombol untuk start
```

**Expected Result untuk "Tune Both Motors":**

- ✅ **KEDUA motor bergerak/oscillating**
- ✅ **Serial Slave menampilkan**: "Error RPM (double) - Kanan: [nilai], Kiri: [nilai]"

## 📊 **Debug Information yang Harus Dimonitor**

### **Master ESP32 Serial Output:**

```
Requesting PID...
Received Right Motor PID: Kp=20.000 Ki=0.500 Kd=0.100
Received Left Motor PID: Kp=20.000 Ki=0.500 Kd=0.100
PID Status - Right: OK Left: OK
System Ready!

# Saat pilih Tune Right Motor:
Sending command: TUNE_RIGHT

# Saat pilih Tune Left Motor:
Sending command: TUNE_LEFT
```

### **Slave ESP32 Serial Output:**

```
# Saat TUNE_RIGHT:
Received command: TUNE_RIGHT
Auto-tuning motor kanan dimulai atas permintaan master
MEMULAI AUTO-TUNING PID RPM - Motor Kanan
Target RPM: 40
Error RPM (double) - Kanan: [nilai], Kiri: 0.000

# Saat TUNE_LEFT:
Received command: TUNE_LEFT
Auto-tuning motor kiri dimulai atas permintaan master
MEMULAI AUTO-TUNING PID RPM - Motor Kiri
Target RPM: 40
Error RPM (double) - Kanan: 0.000, Kiri: [nilai]
```

## 🔍 **Troubleshooting Guide**

### **❌ Problem: Motor yang salah bergerak**

#### **Jika TUNE_RIGHT tapi motor KIRI yang bergerak:**

```
Root Cause: Parameter rpmMotor() masih salah
Solution: Cek auto_tuner.ino line ~64:
  case TUNE_RIGHT:
    rpmMotor(targetRPM, 0);  // Harus rpm1=target, rpm2=0
```

#### **Jika TUNE_LEFT tapi motor KANAN yang bergerak:**

```
Root Cause: Parameter rpmMotor() masih salah
Solution: Cek auto_tuner.ino line ~67:
  case TUNE_LEFT:
    rpmMotor(0, targetRPM);  // Harus rpm1=0, rpm2=target
```

### **❌ Problem: Kedua motor bergerak saat tuning individual**

#### **Possible Causes:**

1. **Auto-tuner tidak menerima TUNE_RIGHT/TUNE_LEFT dengan benar**
2. **Default case di switch statement**
3. **Variable currentTuningTarget tidak ter-set**

#### **Debug Steps:**

```
1. Cek Serial Slave output:
   - Apakah muncul "Motor Kanan" atau "Motor Kiri"?
   - Apakah Error RPM menunjukkan 0.000 untuk motor yang seharusnya diam?

2. Cek variable assignment:
   - currentTuningTarget = TUNE_RIGHT (untuk right motor)
   - currentTuningTarget = TUNE_LEFT (untuk left motor)
```

### **❌ Problem: Tidak ada motor yang bergerak**

#### **Possible Causes:**

1. **PID parameters terlalu kecil**
2. **Hardware connection issue**
3. **PWM tidak reaching motors**

#### **Debug Steps:**

```
1. Cek Serial output untuk PWM values
2. Test manual motor control (jika ada)
3. Cek power supply ke motors
```

## 🎯 **Success Criteria**

### **✅ Test PASSED jika:**

1. **TUNE_RIGHT**: Hanya motor kanan bergerak
2. **TUNE_LEFT**: Hanya motor kiri bergerak
3. **TUNE_BOTH**: Kedua motor bergerak
4. **Serial output**: Error RPM sesuai ekspektasi
5. **Auto-tuning**: Berhasil menghasilkan PID values yang valid

### **❌ Test FAILED jika:**

1. Motor yang salah bergerak
2. Kedua motor bergerak saat individual tuning
3. Tidak ada motor yang bergerak
4. Auto-tuning tidak complete/timeout

---

## **Testing Checklist:**

- [ ] **System startup**: Master dan Slave connected
- [ ] **PID sync**: System ready status OK
- [ ] **TUNE_RIGHT**: Only right motor moves
- [ ] **TUNE_LEFT**: Only left motor moves
- [ ] **TUNE_BOTH**: Both motors move
- [ ] **Auto-tuning complete**: PID values saved successfully
- [ ] **Menu navigation**: All options accessible

## **Next Steps After Testing:**

1. **Jika semua test PASS**: System ready untuk line following!
2. **Jika ada test FAIL**: Analyze debug output dan fix sesuai troubleshooting guide
3. **Optimize PID**: Test line following performance dan fine-tune jika perlu

---

**Status: READY FOR FIELD TESTING** 🚀
