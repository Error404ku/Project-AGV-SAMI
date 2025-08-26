// ===== WIFI CONNECTION FUNCTIONS =====

// Fungsi untuk setup WiFi - dipanggil di setup()
void setupWifi() {
  // Load WiFi config from Preferences
  loadWifiConfig();
  
  // Setup WiFi mode - but don't auto-connect to avoid blocking
  WiFi.mode(WIFI_AP_STA);  // Enable both AP and STA mode
  WiFi.setAutoReconnect(false); // Disable auto-reconnect during setup
  WiFi.persistent(false); // Reduce flash writes
  
  // Only start AP mode for configuration access
  const char* ap_ssid = "ESP32-AGV-Config";
  const char* ap_password = "12345678";
  WiFi.softAP(ap_ssid, ap_password);
  WiFi.softAPConfig(IPAddress(192, 168, 121, 14), IPAddress(192, 168, 121, 14), IPAddress(255, 255, 255, 0));
  
  // Serial.println() - removed for production
  // Serial.println() - removed for production
}

// Fungsi untuk memulai koneksi WiFi - dipanggil saat tombol START ditekan
void startWifiConnection() {
  isConnectingWifi = true;
  wifiConnectStartTime = millis();
  
  // Load WiFi config from Preferences
  loadWifiConfig();
  WiFi.disconnect();
  delay(50); // Reduced delay
  
  // Optimized WiFi connection
  WiFi.mode(WIFI_AP_STA);  // Enable both AP and STA mode
  WiFi.setAutoReconnect(true); // Enable auto-reconnect
  WiFi.persistent(false); // Reduce flash writes
  
  // Configure static IP if available
  if (strlen(staticIPStr) > 0) {
    WiFi.config(staticIP, gateway, subnet, dns);
  }
  
  // Only attempt connection if credentials are available
  if (strlen(ssid) > 0 && strlen(password) > 0) {
    // Serial.printf("Attempting WiFi connection to: %s\n", ssid);
    WiFi.begin(ssid, password);
  } else {
    // Serial.println() - removed for production
    isConnectingWifi = false;
  }
  
  // Ensure AP is still active for web access
  WiFi.softAP("ESP32-AGV-Config", "12345678");
  WiFi.softAPConfig(IPAddress(192, 168, 121, 14), IPAddress(192, 168, 121, 14), IPAddress(255, 255, 255, 0));
}

// Fungsi untuk loop WiFi - dipanggil di loop() jika diperlukan
void loopWifi() {
  // Check WiFi connection status and handle reconnection if needed
  if (WiFi.status() != WL_CONNECTED && !isConnectingWifi) {
    // Auto-reconnect if not currently connecting
    unsigned long currentTime = millis();
    static unsigned long lastReconnectAttempt = 0;
    
    if (currentTime - lastReconnectAttempt > 5000) { // Try reconnect every 5 seconds
      lastReconnectAttempt = currentTime;
      startWifiConnection();
    }
  }
}