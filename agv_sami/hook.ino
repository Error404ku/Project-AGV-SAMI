// hook mode : naik dan turun
HookPosition hook(HookPositionMode mode) {
  HookPositionMode actualMode = mode;
  // Read sensors once
  bool sensor1 = digitalRead(pinHook1);
  bool sensor2 = digitalRead(pinHook2);
  
  if (actualMode == UP_HOOK) {
    // Check if already at top position
    if (sensor1 == HIGH) {
      digitalWrite(pinMotorHook, HIGH);  // Stop motor
      hookPosition = UP_POS;
      return UP_POS;
    } else {
      digitalWrite(pinMotorHook, LOW);   // Move up
      hookPosition = STOP_POS;
      return STOP_POS;  // Still moving
    }
  } else if (actualMode == DOWN_HOOK) {
    // Check if already at bottom position
    if (sensor2 == HIGH) {
      digitalWrite(pinMotorHook, HIGH);  // Stop motor
      hookPosition = DOWN_POS;
      return DOWN_POS;
    } else {
      digitalWrite(pinMotorHook, LOW);   // Move down
      hookPosition = STOP_POS;
      return STOP_POS;  // Still moving
    }
  } else {
    // Stop hook movement
    digitalWrite(pinMotorHook, HIGH); 
    hookPosition = STOP_POS;
    return STOP_POS;  
  }
}