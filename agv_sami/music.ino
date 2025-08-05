void music(MusicMode mode) {
  Serial.print("Start Music : ");
  Serial.println(mode);

  if (statusMusic) {
    Serial.println("Music already on");
    return;
  }

  statusMusic = true;
  currentMusicMode = mode; // Store current music mode

  // Reset all relays (OFF)
  digitalWrite(pinMusic1, HIGH);
  digitalWrite(pinMusic2, HIGH);
  digitalWrite(pinMusic3, HIGH);
  digitalWrite(pinMusic4, HIGH);

  // Mapping
  int targetPin = -1;
  switch (mode) {
    case MUSIC_MODE_STATION: targetPin = musicStationPin; break;
    case MUSIC_MODE_ERROR: targetPin = musicErrorPin; break;
    case MUSIC_MODE_DETECT: targetPin = musicDetectPin; break;
    case MUSIC_MODE_KOMPUTER: targetPin = musicKomputerPin; break;
  }
  // print target pin
  Serial.print("Target Pin : ");
  Serial.println(targetPin);

  // Set selected pin ON
  if (targetPin >= 0 && targetPin <= 3) {
    switch (targetPin) {
      case 0: digitalWrite(pinMusic1, LOW); break;
      case 1: digitalWrite(pinMusic2, LOW); break;
      case 2: digitalWrite(pinMusic3, LOW); break;
      case 3: digitalWrite(pinMusic4, LOW); break;
    }
  }

  // Atur timer hanya untuk station & komputer
  if (mode != MUSIC_MODE_ERROR && mode != MUSIC_MODE_DETECT) {
    previousMillis = millis();  // <- UPDATE timer di sini
  }
}


void stopMusic() {
  digitalWrite(pinMusic1, HIGH);
  digitalWrite(pinMusic2, HIGH);
  digitalWrite(pinMusic3, HIGH);
  digitalWrite(pinMusic4, HIGH);
  statusMusic = false;
}
