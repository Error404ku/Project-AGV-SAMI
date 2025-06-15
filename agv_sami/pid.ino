double computePID(int index, double setpoint, double input, double Kp, double Ki, double Kd, int Minintegral, double Maxintegral) {
  // Hitung error
  pidData[index].error = setpoint - input;
  // Hitung integral dan derivatif
  pidData[index].integral += pidData[index].error;

  pidData[index].integral = constrain(pidData[index].integral, Minintegral, Maxintegral);
  pidData[index].derivative = pidData[index].error - pidData[index].previousError;
  // Hitung output PID
  double output = Kp * pidData[index].error + Ki * pidData[index].integral + Kd * pidData[index].derivative;

  // Simpan nilai error untuk penggunaan selanjutnya
  pidData[index].previousError = pidData[index].error;

  return output;
}
