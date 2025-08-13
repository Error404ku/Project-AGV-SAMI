void music(MusicMode mode) {
  Serial.print("Start Music : ");
  Serial.println(mode);

  if (statusMusic) {
    Serial.println("Music already on");
    return;
  }

  statusMusic = true;
  currentMusicMode = mode; // Store current music mode
  
    switch (mode) {
      case MUSIC_MODE_ON: 
        digitalWrite(pinMusic1, LOW);
        digitalWrite(pinMusic2, HIGH);
        digitalWrite(pinMusic3, HIGH);
        digitalWrite(pinMusic4, HIGH);
        digitalWrite(pinMusic5, HIGH);
        digitalWrite(pinMusic6, HIGH);
        break;
      case MUSIC_MODE_OBSTACLE:
        digitalWrite(pinMusic1, HIGH);
        digitalWrite(pinMusic2, LOW);
        digitalWrite(pinMusic3, HIGH);
        digitalWrite(pinMusic4, HIGH);
        digitalWrite(pinMusic5, HIGH);
        digitalWrite(pinMusic6, HIGH);
        break;
      case MUSIC_MODE_STATION:
        digitalWrite(pinMusic1, HIGH);
        digitalWrite(pinMusic2, HIGH);
        digitalWrite(pinMusic3, LOW);
        digitalWrite(pinMusic4, HIGH);
        digitalWrite(pinMusic5, HIGH);
        digitalWrite(pinMusic6, HIGH);
        break;
      case MUSIC_MODE_OUTOFLINE:
        digitalWrite(pinMusic1, HIGH);
        digitalWrite(pinMusic2, HIGH);
        digitalWrite(pinMusic3, HIGH);
        digitalWrite(pinMusic4, LOW);
        digitalWrite(pinMusic5, HIGH);
        digitalWrite(pinMusic6, HIGH);
        break;
      case MUSIC_MODE_WARNING:
        digitalWrite(pinMusic1, HIGH);
        digitalWrite(pinMusic2, HIGH);
        digitalWrite(pinMusic3, HIGH);
        digitalWrite(pinMusic4, HIGH);
        digitalWrite(pinMusic5, LOW);
        digitalWrite(pinMusic6, HIGH);
        break;
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
