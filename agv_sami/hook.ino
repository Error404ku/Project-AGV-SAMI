// hook mode : naik dan turun
HookPosition hook(HookPositionMode mode) {
  // Apply hook inversion if enabled
  HookPositionMode actualMode = mode;
  if (invertHook && mode != STOP_HOOK) {
    actualMode = (mode == UP_HOOK) ? DOWN_HOOK : UP_HOOK;
    Serial.println("[HOOK] Inversion enabled - mode changed");
  }
  
  // Debug: Print sensor status
  bool sensor1 = digitalRead(pinHook1);
  bool sensor2 = digitalRead(pinHook2);
  Serial.printf("[HOOK] Mode: %d, Sensor1(top): %d, Sensor2(bottom): %d\n", actualMode, sensor1, sensor2);
  
  if (actualMode == UP_HOOK) {
    // Check if already at top position
    if (digitalRead(pinHook1) == HIGH) {
      digitalWrite(pinMotorHook, HIGH);  // Stop motor
      Serial.println("[HOOK] Reached TOP position - stopping motor");
      hookPosition = UP_POS;
      return UP_POS;
    } else {
      digitalWrite(pinMotorHook, LOW);   // Move up
      Serial.println("[HOOK] Moving UP - motor activated");
      hookPosition = STOP_POS;
      return STOP_POS;  // Still moving
    }
  } else if (actualMode == DOWN_HOOK) {
    // Check if already at bottom position
    if (digitalRead(pinHook2) == HIGH) {
      digitalWrite(pinMotorHook, HIGH);  // Stop motor
      Serial.println("[HOOK] Reached BOTTOM position - stopping motor");
      hookPosition = DOWN_POS;
      return DOWN_POS;
    } else {
      digitalWrite(pinMotorHook, LOW);   // Move down
      Serial.println("[HOOK] Moving DOWN - motor activated");
      hookPosition = STOP_POS;
      return STOP_POS;  // Still moving
    }
  } else {
    // Stop hook movement
    digitalWrite(pinMotorHook, HIGH);
    Serial.println("[HOOK] STOP mode - motor stopped");
    hookPosition = STOP_POS;
    return STOP_POS;  
  }
}