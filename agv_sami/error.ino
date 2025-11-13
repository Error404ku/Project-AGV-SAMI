
// Non-fatal error function - logs error but continues operation
void logError(int code, String text) {
  Serial.print("WARNING - Code: ");
  Serial.print(code);
  Serial.print(" - ");
  Serial.println(text);
}
