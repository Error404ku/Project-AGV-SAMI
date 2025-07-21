#include "config.h"

void setup() {
  Serial.begin(115200);
  setupAll();

  // lcd.clear();
  changeStateMode("maju");
  Serial.println("SETUP SELESAI");
}

void loop() {
  server.handleClient();
  loopRfid(); // Handle RFID scanning - now controlled internally by conditions
  loopUltrasonik(); // Handle ultrasonic obstacle detection
  // delay(1000);
  if (isAgvMode) {
    // AGV Mode - Run normal AGV operation
    // pembacaanRpm();
    displayPrint();
    bacaSensorGaris();
    logicAgv();

    // Check for B button to exit AGV mode
    if (STOP()) {
      isAgvMode = false;
      modeBerhenti = true;
      buttonStep = 0;  // Reset button step
    }
  } else {
    // Menu Mode
    inTerminal();
    handleMenu();
  }
  // LCD doesn't need display() call - content shows immediately
}
