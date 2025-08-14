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
  digitalWrite(pinMusic5, HIGH);
  digitalWrite(pinMusic6, HIGH);

  // Mapping
  int targetPin = -1;
  switch (mode) {
    case MUSIC_MODE_ON: targetPin = musicOnPin; break;
    case MUSIC_MODE_OBSTACLE: targetPin = musicObstaclePin; break;
    case MUSIC_MODE_STATION: targetPin = musicStationPin; break;
    case MUSIC_MODE_OUTOFLINE: targetPin = musicOutOfLinePin; break;
    case MUSIC_MODE_WARNING: targetPin = musicWarningPin; break;
  }

  // Set selected pin ON
  if (targetPin >= 0 && targetPin <= 5) {
    switch (targetPin) {
      case 0: digitalWrite(pinMusic1, LOW); break;
      case 1: digitalWrite(pinMusic2, LOW); break;
      case 2: digitalWrite(pinMusic3, LOW); break;
      case 3: digitalWrite(pinMusic4, LOW); break;
      case 4: digitalWrite(pinMusic5, LOW); break;
    }
  }

  // Atur timer hanya untuk on & station
  if (mode != MUSIC_MODE_OBSTACLE && mode != MUSIC_MODE_OUTOFLINE) {
    previousMillis = millis();  // <- UPDATE timer di sini
  }
}


void stopMusic() {
  digitalWrite(pinMusic1, HIGH);
  digitalWrite(pinMusic2, HIGH);
  digitalWrite(pinMusic3, HIGH);
  digitalWrite(pinMusic4, HIGH);
  digitalWrite(pinMusic5, HIGH);
  digitalWrite(pinMusic6, LOW);
  statusMusic = false;
}
