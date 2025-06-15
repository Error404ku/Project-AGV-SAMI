void rpmMotor(int motor1, int motor2) {
  // Hitung batas integral berdasarkan parameter PID
  double minIntegral = -1023.0 / ki;
  double maxIntegral = 1023.0 / ki;

  // Batasi RPM target agar tidak keluar batas
  motor1 = constrain(motor1, minrpm, maxrpm);
  motor2 = constrain(motor2, minrpm, maxrpm);

  // PWM kanan
  if (motor1 > 0) {
    pwmKanan = computePID(0, motor1, rpmKanan, kp, ki, kd, minIntegral, maxIntegral);
    pwmKanan = constrain(pwmKanan, pwm_zero, pwm_max);
  } else if (motor1 < 0) {
    pwmKanan = computePID(0, abs(motor1), rpmKanan, kp, ki, kd, minIntegral, maxIntegral);
    pwmKanan = -pwmKanan;
    pwmKanan = constrain(pwmKanan, pwm_min, pwm_zero);
  } else {
    pwmKanan = 0;
    pidData[0].error = 0;
    pidData[0].integral = 0;
    pidData[0].derivative = 0;
    pidData[0].previousError = 0;
    pidData[0].lastOutput = 0;
  }

  // PWM kiri
  if (motor2 > 0) {
    pwmKiri = computePID(1, motor2, rpmKiri, kp, ki, kd, minIntegral, maxIntegral);
    pwmKiri = constrain(pwmKiri, pwm_zero, pwm_max);
  } else if (motor2 < 0) {
    pwmKiri = computePID(1, abs(motor2), rpmKiri, kp, ki, kd, minIntegral, maxIntegral);
    pwmKiri = -pwmKiri;
    pwmKiri = constrain(pwmKiri, pwm_min, pwm_zero);
  } else {
    pwmKiri = 0;
    pidData[1].error = 0;
    pidData[1].integral = 0;
    pidData[1].derivative = 0;
    pidData[1].previousError = 0;
    pidData[1].lastOutput = 0;
  }

  // Debug (opsional)
  // Serial.print("RPM Kanan: "); Serial.print(rpmKanan);
  // Serial.print(" | PWM Kanan: "); Serial.println(pwmKanan);
  // Serial.print("RPM Kiri : "); Serial.print(rpmKiri);
  // Serial.print(" | PWM Kiri : "); Serial.println(pwmKiri);
// if (pwmKanan > 0 && pwmKanan < 60) pwmKanan = 60;
// if (pwmKanan < 0 && pwmKanan > -60) pwmKanan = -60;
// if (pwmKiri > 0 && pwmKiri < 60) pwmKiri = 60;
// if (pwmKiri < 0 && pwmKiri > -60) pwmKiri = -60;

  // Kirim PWM ke motor
  pwmMotor(pwmKanan, pwmKiri);
}

void pwmMotor(int motor1, int motor2)
{
    if (motor1 > 0)
    {
        ledcWrite(channelKanan, motor1);
        digitalWrite(mdKananA, LOW);
    }
    else if (motor1 < 0)
    {
        ledcWrite(channelKanan, maxPwm + motor1);
        digitalWrite(mdKananA, HIGH);
    }
    else
    {
        ledcWrite(channelKanan, 0);
        digitalWrite(mdKananA, LOW);
    }
    if (motor2 > 0){
        ledcWrite(channelKiri, motor2);
        digitalWrite(mdKiriA, LOW);
    }   
    else if (motor2 < 0){
        ledcWrite(channelKiri, maxPwm + motor2);
        digitalWrite(mdKiriA, HIGH);
    }
    else{   
        ledcWrite(channelKiri, 0);
        digitalWrite(mdKiriA, LOW);
    }   
}
