// hook mode : naik dan turun
void hook(String mode) {

  if (mode == "naik") {
    digitalWrite(pinMotorHook, LOW);
    if (digitalRead(pinHook1) == HIGH) {
      digitalWrite(pinMotorHook, HIGH);
    }
  } else if (mode == "turun") {
    digitalWrite(pinMotorHook, LOW);
    if (digitalRead(pinHook2) == HIGH) {
      digitalWrite(pinMotorHook, HIGH);
    }
  } else {
    // Stop hook (both pins LOW)
    digitalWrite(pinMotorHook, HIGH);
  }
}