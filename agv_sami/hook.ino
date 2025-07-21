// hook mode : naik dan turun
void hook(String mode) {
  // Apply hook inversion if enabled
  
  String actualMode = mode;
  if (invertHook) {
    if (mode == "naik") {
      actualMode = "turun";
    } else if (mode == "turun") {
      actualMode = "naik";
    }
  }
  
  if (actualMode == "naik") {
    digitalWrite(pinHook1, HIGH);
    digitalWrite(pinHook2, LOW);
    digitalWrite(pinMotorHook, HIGH);
  } else if (actualMode == "turun") {
    digitalWrite(pinHook1, LOW);
    digitalWrite(pinHook2, HIGH);
    digitalWrite(pinMotorHook, HIGH);
  } else {
    // Stop hook (both pins LOW)
    digitalWrite(pinHook1, LOW);
    digitalWrite(pinHook2, LOW);
    digitalWrite(pinMotorHook, LOW);
  }
}