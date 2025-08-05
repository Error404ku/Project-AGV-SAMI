// ===== WIFI CONNECTION FUNCTIONS =====

// Fungsi untuk setup WiFi - dipanggil di setup()
void setupWifi() {
  // Load WiFi config from   Preferences
  loadWifiConfig();
  
  // Setup WiFi mode
  WiFi.mode(WIFI_AP_STA);  // Enable both AP and STA mode
  WiFi.setAutoReconnect(true); // Enable auto-reconnect
  WiFi.persistent(false); // Reduce flash writes
  
  // Ensure AP is always active for web access
  const char* ap_ssid = "ESP32-AGV-Config";
  const char* ap_password = "12345678";
  WiFi.softAP(ap_ssid, ap_password);
  WiFi.softAPConfig(IPAddress(192, 168, 121, 14), IPAddress(192, 168, 121, 14), IPAddress(255, 255, 255, 0));
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
  WiFi.begin(ssid, password);
  
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
    
    if (currentTime - lastReconnectAttempt > 2000) { // Try reconnect every 5 seconds
      lastReconnectAttempt = currentTime;
      startWifiConnection();
    }
  }
}