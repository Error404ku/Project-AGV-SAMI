// void rpmMotor(int motor1, int motor2) {
//   // Hitung batas integral berdasarkan parameter PID
//   double minIntegral = -1023.0 / ki;
//   double maxIntegral = 1023.0 / ki;

//   // Batasi RPM target agar tidak keluar batas
//   motor1 = constrain(motor1, minrpm, maxrpm);
//   motor2 = constrain(motor2, minrpm, maxrpm);

//   // PWM kanan
//   if (motor1 > 0) {
//     pwmKanan = computePID(0, motor1, rpmKanan, kp, ki, kd, minIntegral, maxIntegral);
//     pwmKanan = constrain(pwmKanan, pwm_zero, pwm_max);
//   } else if (motor1 < 0) {
//     pwmKanan = computePID(0, abs(motor1), rpmKanan, kp, ki, kd, minIntegral, maxIntegral);
//     pwmKanan = -pwmKanan;
//     pwmKanan = constrain(pwmKanan, pwm_min, pwm_zero);
//   } else {
//     pwmKanan = 0;
//     pidData[0].error = 0;
//     pidData[0].integral = 0;
//     pidData[0].derivative = 0;
//     pidData[0].previousError = 0;
//     pidData[0].lastOutput = 0;
//   }

//   // PWM kiri
//   if (motor2 > 0) {
//     pwmKiri = computePID(1, motor2, rpmKiri, kp, ki, kd, minIntegral, maxIntegral);
//     pwmKiri = constrain(pwmKiri, pwm_zero, pwm_max);
//   } else if (motor2 < 0) {
//     pwmKiri = computePID(1, abs(motor2), rpmKiri, kp, ki, kd, minIntegral, maxIntegral);
//     pwmKiri = -pwmKiri;
//     pwmKiri = constrain(pwmKiri, pwm_min, pwm_zero);
//   } else {
//     pwmKiri = 0;
//     pidData[1].error = 0;
//     pidData[1].integral = 0;
//     pidData[1].derivative = 0;
//     pidData[1].previousError = 0;
//     pidData[1].lastOutput = 0;
//   }

//   // Check for encoder failure - if motors are given high PWM but no RPM feedback
//   static unsigned long lastEncoderCheck = 0;
//   static int noEncoderFeedbackCount = 0;
  
//   if (millis() - lastEncoderCheck > 1000) { // Check every second
//     lastEncoderCheck = millis();
    
//     // If high PWM but very low RPM, possible encoder failure
//     if ((abs(pwmKanan) > 500 && abs(rpmKanan) < 50) || 
//         (abs(pwmKiri) > 500 && abs(rpmKiri) < 50)) {
//       noEncoderFeedbackCount++;
//       if (noEncoderFeedbackCount >= 3) {
//         logError(ERROR_ENCODER_FAILURE, "Encoder feedback rendah");
//         noEncoderFeedbackCount = 0; // Reset counter after logging
//       }
//     } else {
//       noEncoderFeedbackCount = 0; // Reset if working normally
//     }
//   }
  
//   // Debug (opsional)
//   // Serial.print("RPM Kanan: "); Serial.print(rpmKanan);
//   // Serial.print(" | PWM Kanan: "); Serial.println(pwmKanan);
//   // Serial.print("RPM Kiri : "); Serial.print(rpmKiri);
//   // Serial.print(" | PWM Kiri : "); Serial.println(pwmKiri);
// // if (pwmKanan > 0 && pwmKanan < 60) pwmKanan = 60;
// // if (pwmKanan < 0 && pwmKanan > -60) pwmKanan = -60;
// // if (pwmKiri > 0 && pwmKiri < 60) pwmKiri = 60;
// // if (pwmKiri < 0 && pwmKiri > -60) pwmKiri = -60;

//   // Kirim PWM ke motor
//   pwmMotor(pwmKanan, pwmKiri);
// }

void pwmMotor(int motor1, int motor2)
{
    // Safety checks for motor PWM values
    if (abs(motor1) > maxPwm || abs(motor2) > maxPwm) {
        logError(ERROR_MOTOR_CONTROL, "PWM nilai melebihi batas");
        motor1 = constrain(motor1, minPwm, maxPwm);
        motor2 = constrain(motor2, minPwm, maxPwm);
    }
    
    // Apply Y-axis inversion (maju-mundur) if enabled
    if (invertMotorY) {
        motor1 = -motor1;
        motor2 = -motor2;
    }
    
    // Apply individual motor inversion
    if (invertMotorKanan) {
        motor1 = -motor1;
    }
    if (invertMotorKiri) {
        motor2 = -motor2;
    }
    
    // Kontrol Motor Kanan (motor1) menggunakan L298N
    if (motor1 > 0) {
        // Motor kanan maju
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        ledcWriteChannel(channelKanan, motor1);
    }
    else if (motor1 < 0) {
        // Motor kanan mundur
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        ledcWriteChannel(channelKanan, abs(motor1));
    }
    else {
        // Motor kanan stop
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        ledcWriteChannel(channelKanan, 0);
    }
    
    // Kontrol Motor Kiri (motor2) menggunakan L298N
    if (motor2 > 0) {
        // Motor kiri maju
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
        ledcWriteChannel(channelKiri, motor2);
    }   
    else if (motor2 < 0) {
        // Motor kiri mundur
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
        ledcWriteChannel(channelKiri, abs(motor2));
    }
    else {   
        // Motor kiri stop
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        ledcWriteChannel(channelKiri, 0);
    }   
}
