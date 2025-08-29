double computePID(int index, double setpoint, double input, double Kp, double Ki, double Kd, int Minintegral, double Maxintegral) {
  // Validate input parameters
  if (index < 0 || index >= numOutputs) {
    logError(ERROR_PID_CALCULATION, "PID index tidak valid");
    return 0.0;
  }

  // Check for invalid values (NaN or infinity)
  if (isnan(setpoint) || isnan(input) || isinf(setpoint) || isinf(input)) {
    logError(ERROR_PID_CALCULATION, "PID input NaN/Inf");
    return 0.0;
  }

  // Ensure input values are within reasonable limits
  setpoint = constrain(setpoint, 0.0, (double)maxrpm);
  input = constrain(input, -1.0 * (double)maxrpm, (double)maxrpm);

  // Debug output untuk diagnosa
  Serial.printf("PID Calculation - Index: %d, Setpoint: %.2f, Input: %.2f\n", index, setpoint, input);

  // Hitung error - pastikan tipe data double digunakan dengan benar
  double error = setpoint - input;
  
  // Validasi error - batasi nilai maksimum lebih ketat
  error = constrain(error, -50.0, 50.0);
  
  // Store error secara aman ke dalam pidData
  pidData[index].error = error;
  
  // Reset integral jika error sangat kecil (deadband) untuk mencegah oscillation
  if (fabs(error) < 0.5) {
    pidData[index].integral *= 0.9; // Slow decay pada integral saat error kecil
  }
  
  // Hitung integral dengan batas aman yang lebih ketat
  pidData[index].integral += error;
  pidData[index].integral = constrain(pidData[index].integral, Minintegral, Maxintegral);
  
  // Hitung derivative dengan filter sederhana untuk mengurangi noise
  double derivative = error - pidData[index].previousError;
  // Simple low-pass filter (0.7 * previous + 0.3 * current)
  pidData[index].derivative = 0.7 * pidData[index].derivative + 0.3 * derivative;
  
  // Hitung output PID dengan clipping untuk mencegah overflow
  double pTerm = Kp * error;
  double iTerm = Ki * pidData[index].integral;
  double dTerm = Kd * pidData[index].derivative;
  
  // Batasi masing-masing term untuk mencegah nilai ekstrim
  pTerm = constrain(pTerm, -1000.0, 1000.0);
  iTerm = constrain(iTerm, -500.0, 500.0);
  dTerm = constrain(dTerm, -500.0, 500.0);
  
  double output = pTerm + iTerm + dTerm;
                                                                                              
  // Check for invalid output
  if (isnan(output) || isinf(output)) {
    logError(ERROR_PID_CALCULATION, "PID output NaN/Inf");
    pidData[index].integral = 0; // Reset integral saat output tidak valid
    return 0.0;
  }

  // Batasi output PID
  output = constrain(output, -2000.0, 2000.0);

  // Simpan nilai error untuk penggunaan selanjutnya
  pidData[index].previousError = error;

  // Debug output untuk troubleshooting
  Serial.printf("PID Terms - P: %.2f, I: %.2f, D: %.2f, Output: %.2f\n", 
                pTerm, iTerm, dTerm, output);

  return output;
}
