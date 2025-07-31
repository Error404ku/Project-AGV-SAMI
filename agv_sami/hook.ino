// hook mode : naik dan turun
HookPosition hook(HookPositionMode mode) {
  if (mode == UP_HOOK) {
    digitalWrite(pinMotorHook, LOW);
    if (digitalRead(pinHook1) == HIGH) {
      digitalWrite(pinMotorHook, HIGH);
      return UP_POS;
    }
  } else if (mode == DOWN_HOOK) {
    digitalWrite(pinMotorHook, LOW);
    if (digitalRead(pinHook2) == HIGH) {
      digitalWrite(pinMotorHook, HIGH);
      return DOWN_POS;
    }
  } else {
    // Stop hook (both pins LOW)
    digitalWrite(pinMotorHook, HIGH);
    return STOP_POS;  
  }
}