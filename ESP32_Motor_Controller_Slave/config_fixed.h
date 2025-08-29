// Perbaikan pada struktur config.h

#ifndef CONFIG_FIXED_H
#define CONFIG_FIXED_H

// PID Configuration - FIXED
// Ubah numOutputs menjadi 2 karena kita hanya menggunakan 2 motor
#define numOutputs 2  // Number of PID controllers (for 2 motors)

// Error Codes - Tambahkan kode error untuk debugging
#define ERROR_PID_CALCULATION 1001
#define ERROR_OVERFLOW 1002
#define ERROR_INVALID_INPUT 1003
#define ERROR_INVALID_OUTPUT 1004

// RPM Configuration - Tidak perlu diubah
#define minrpm 0
#define maxrpm 90    // RPM maksimal adalah 90

// PWM Configuration for PID - 12-bit PWM (0-4095)
// Pertimbangkan untuk membatasi nilai PWM maksimal jika motor bergerak terlalu cepat
#define pwm_zero 0
#define pwm_min -4095  // PWM minimum (reverse direction)
#define pwm_max 4095   // PWM maximum (forward direction)

// Penjelasan untuk perubahan:
// 1. numOutputs diubah dari 5 menjadi 2 karena kita hanya menggunakan 2 PID (motor kanan dan kiri)
// 2. Ditambahkan error codes untuk membantu debugging
// 3. Catatan untuk mempertimbangkan pembatasan nilai PWM jika diperlukan

#endif // CONFIG_FIXED_H
