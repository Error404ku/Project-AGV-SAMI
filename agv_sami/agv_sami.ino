#include "config.h"
extern bool modeMaju;
extern bool modeMundur;

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
  // loopUltrasonik(); // Akan dipanggil manual sesuai mode

  if (isAgvMode) {
    // AGV Mode - Run normal AGV operation
    // pembacaanRpm();
    displayPrint();
    // bacaSensorGaris(); // Akan dipanggil manual sesuai mode

    // --- Pembacaan sensor sesuai mode ---
    if (modeMaju) {
      bacaSensor(SLAVEID_MAGNET_DEPAN);
      setUltrasonicSlaveId(SLAVEID_ULTRASONIK_DEPAN);
      loopUltrasonik();
    } else if (modeMundur) {
      bacaSensor(SLAVEID_MAGNET_BELAKANG);
      setUltrasonicSlaveId(SLAVEID_ULTRASONIK_BELAKANG);
      loopUltrasonik();
    }

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
