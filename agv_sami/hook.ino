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
    if (pinHook1 != 0) {
      digitalWrite(pinMotorHook, LOW);
    }
  } else if (actualMode == "turun") {
    digitalWrite(pinMotorHook, HIGH);
    if (pinHook2 != 0) {
      digitalWrite(pinMotorHook, LOW);
    }
  } else {
    digitalWrite(pinMotorHook, LOW);
  }
}