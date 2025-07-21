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
  
  // Hitung error
  pidData[index].error = setpoint - input;
  // Hitung integral dan derivatif
  pidData[index].integral += pidData[index].error;

  pidData[index].integral = constrain(pidData[index].integral, Minintegral, Maxintegral);
  pidData[index].derivative = pidData[index].error - pidData[index].previousError;
  // Hitung output PID
  double output = Kp * pidData[index].error + Ki * pidData[index].integral + Kd * pidData[index].derivative;

  // Check for invalid output
  if (isnan(output) || isinf(output)) {
    logError(ERROR_PID_CALCULATION, "PID output NaN/Inf");
    return 0.0;
  }

  // Simpan nilai error untuk penggunaan selanjutnya
  pidData[index].previousError = pidData[index].error;

  return output;
}
