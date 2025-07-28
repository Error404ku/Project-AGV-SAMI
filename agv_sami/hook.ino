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
    digitalWrite(pinMotorHook, HIGH);
    if (digitalRead(pinHook1) == HIGH) {
      digitalWrite(pinMotorHook, LOW);
    }
  } else if (actualMode == "turun") {
    digitalWrite(pinMotorHook, HIGH);
    if (digitalRead(pinHook2) == HIGH) {
      digitalWrite(pinMotorHook, LOW);
    }
  } else {
    // Stop hook (both pins LOW)
    digitalWrite(pinMotorHook, LOW);
  }
}