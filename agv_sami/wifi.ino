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
}

// Fungsi untuk memulai koneksi WiFi - dipanggil saat tombol START ditekan
void startWifiConnection() {
  isConnectingWifi = true;
  wifiConnectStartTime = millis();
  
  // Optimized WiFi connection
  WiFi.mode(WIFI_AP_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  
  // Configure static IP if available
  if (strlen(staticIPStr) > 0) {
    WiFi.config(staticIP, gateway, subnet, dns);
  }
  
  // Only attempt connection if credentials are available
  if (strlen(ssid) > 0 && strlen(password) > 0) {
    WiFi.begin(ssid, password);
  } else {
    isConnectingWifi = false;
  }
}

// Fungsi untuk loop WiFi - dipanggil di loop() jika diperlukan
void loopWifi() {
  static unsigned long lastReconnectAttempt = 0;
  unsigned long currentTime = millis();
  
  // Check if we're in connecting state
  if (isConnectingWifi) {
    // Check for connection timeout (30 seconds)
    if (currentTime - wifiConnectStartTime > 30000) {
      isConnectingWifi = false;
    }
    // Check if connection succeeded
    else if (WiFi.status() == WL_CONNECTED) {
      isConnectingWifi = false;
    }
    // Still connecting, do nothing
    return;
  }
  
  // Check WiFi connection status and handle reconnection if needed
  if (WiFi.status() != WL_CONNECTED) {
    // Auto-reconnect with proper interval (5 seconds)
    if (currentTime - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = currentTime;
      startWifiConnection();
    }
  }
}