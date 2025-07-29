bool saveStationsListToPreferences() {
  stationsPreferences.begin(STATIONS_NAMESPACE, false);  // Buka untuk menulis
  stationsPreferences.clear();                           // Hapus semua data lama di namespace ini

  Serial.println("Menyimpan daftar stasiun ke Preferences...");
  // Ukuran buffer JSON, disesuaikan untuk daftar stasiun (misal 5 integer = ~50-100 byte)
  // 1024 byte sudah sangat cukup untuk banyak integer.
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();
  for (int id : stationsList) {
    array.add(id);
  }

  String outputString;
  size_t bytesSerialized = serializeJson(doc, outputString);  // Serialisasi JSON ke string

  if (bytesSerialized == 0) {
    Serial.println("Error: Gagal menserialisasi daftar stasiun, mungkin buffer JSON terlalu kecil.");
    stationsPreferences.end();
    return false;
  }

  Serial.println("JSON daftar stasiun yang akan disimpan: " + outputString);

  size_t bytesWritten = stationsPreferences.putString("stations_array", outputString);  // Simpan string JSON

  stationsPreferences.end();  // Tutup sesi Preferences

  if (bytesWritten > 0) {
    Serial.println("Berhasil menyimpan daftar stasiun ke flash (" + String(bytesWritten) + " bytes).");
    return true;
  } else {
    Serial.println("Gagal menyimpan daftar stasiun ke flash (0 bytes written).");
    return false;
  }
}
// Memuat stationsList dari Preferences (Flash) ke RAM
bool loadStationsListFromPreferences() {
  stationsPreferences.begin(STATIONS_NAMESPACE, true);  // Buka untuk membaca
  stationsList.clear();                                 // Kosongkan daftar di RAM sebelum memuat

  Serial.println("Memuat daftar stasiun dari Preferences ke RAM...");
  String storedStations = stationsPreferences.getString("stations_array", "[]");  // Ambil string JSON
  stationsPreferences.end();                                                      // Tutup sesi Preferences

  Serial.println("Data stasiun tersimpan: " + storedStations);

  DynamicJsonDocument doc(1024);                                      // Ukuran buffer JSON untuk deserialisasi
  DeserializationError error = deserializeJson(doc, storedStations);  // Deserialisasi string JSON

  if (!error) {  // Jika TIDAK ada error saat deserialisasi
    JsonArray array = doc.as<JsonArray>();
    if (array) {  // Pastikan itu adalah JsonArray yang valid
      for (JsonVariant v : array) {
        stationsList.push_back(v.as<int>());
      }
      Serial.println("Berhasil memuat daftar stasiun ke RAM. Jumlah stasiun: " + String(stationsList.size()));
    } else {
      Serial.println("Data dari Preferences bukan format array JSON yang diharapkan.");
    }
  } else {
    Serial.print("Gagal mem-parsing data stasiun dari Preferences: ");
    Serial.println(error.c_str());
  }
  return true;
}
// --- HANDLER ENDPOINT HTTP ---
// Handler untuk memperbarui daftar stasiun
void handleUpdateStations() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Error: Body JSON tidak ada.");
    return;
  }

  DynamicJsonDocument doc(1024);  // Ukuran buffer untuk deserialisasi JSON input
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  // Validasi JSON input: Pastikan ada field "stations" dan itu adalah array
  if (error || !doc.is<JsonObject>() || !doc.containsKey("stations") || !doc["stations"].is<JsonArray>()) {
    Serial.print("JSON invalid atau format salah (handleUpdateStations): ");
    Serial.println(error.c_str());

    // Log HTTP parsing error
    logError(ERROR_INVALID_CONFIGURATION, "HTTP JSON parsing gagal");

    server.send(400, "application/json", "{\"status\":\"error\", \"message\":\"JSON tidak valid atau format salah. Harap kirim { \\\"stations\\\": [1,2,3] }\" }");
    return;
  }

  stationsList.clear();  // Kosongkan daftar stasiun yang lama di RAM
  JsonArray receivedStations = doc["stations"].as<JsonArray>();
  for (JsonVariant v : receivedStations) {
    stationsList.push_back(v.as<int>());
  }
  Serial.println("Variabel RAM 'stationsList' telah diperbarui dengan " + String(stationsList.size()) + " elemen.");

  if (saveStationsListToPreferences()) {  // Simpan ke Preferences
    server.send(200, "application/json", "{\"status\":\"ok\", \"message\":\"Daftar stasiun berhasil diperbarui di RAM dan flash.\" }");
  } else {
    server.send(500, "application/json", "{\"status\":\"error\", \"message\":\"Gagal menyimpan data ke penyimpanan.\" }");
  }
}
// Handler untuk menampilkan daftar stasiun yang ditemukan (stationsList)
void handleShowStations() {
  DynamicJsonDocument doc(1024);  // Ukuran buffer untuk respons JSON (bisa disesuaikan)
  JsonArray array = doc.to<JsonArray>();
  for (int id : stationsList) {
    array.add(id);
  }
  String output;
  serializeJsonPretty(doc, output);  // Serialisasi dengan format PrettyPrint

  Serial.println("Mengirim daftar stasiun dari RAM.");
  server.send(200, "application/json", output);
}
// --- FUNGSI UNTUK MENGOSONGKAN DAFTAR STATION ---
void clearStationsData() {
  // 1. Kosongkan std::vector di RAM
  stationsList.clear();
  Serial.println("Daftar station di RAM telah dikosongkan.");

  // 2. Kosongkan data di Preferences
  stationsPreferences.begin(STATIONS_NAMESPACE, false);  // Buka untuk menulis
  stationsPreferences.clear();                           // Hapus semua data di namespace ini
  stationsPreferences.end();                             // Tutup sesi Preferences
  Serial.println("Data station di Preferences telah dihapus.");
}

// --- FUNGSI UNTUK MENGURUTKAN DAFTAR STATION ---
void sortStationsList() {
  if (stationsList.size() > 1) {
    std::sort(stationsList.begin(), stationsList.end());
    Serial.println("Daftar station telah diurutkan.");

    // Print sorted list for debugging
    Serial.print("Sorted stations: ");
    for (size_t i = 0; i < stationsList.size(); i++) {
      Serial.print(stationsList[i]);
      if (i < stationsList.size() - 1) Serial.print(", ");
    }
    Serial.println();
  } else {
    Serial.println("Tidak ada station untuk diurutkan.");
  }
}

// --- FUNGSI KONFIGURASI WIFI ---
// Menyimpan konfigurasi WiFi ke Preferences
bool saveWifiConfig(const String& ssid, const String& password, const String& staticIP, const String& gateway, const String& subnet, const String& dns) {
  Serial.println("=== saveWifiConfig dipanggil ===");
  Serial.println("Namespace: " + String(PREFERENCES_NAMESPACE));
  
  if (!preferences.begin(PREFERENCES_NAMESPACE, false)) {
    Serial.println("ERROR: Gagal membuka Preferences namespace!");
    return false;
  }
  
  Serial.println("Preferences berhasil dibuka, mulai menyimpan...");
  
  size_t result1 = preferences.putString("wifi_ssid", ssid);
  size_t result2 = preferences.putString("wifi_password", password);
  size_t result3 = preferences.putString("wifi_static_ip", staticIP);
  size_t result4 = preferences.putString("wifi_gateway", gateway);
  size_t result5 = preferences.putString("wifi_subnet", subnet);
  size_t result6 = preferences.putString("wifi_dns", dns);
  
  Serial.println("Hasil penyimpanan:");
  Serial.println("wifi_ssid: " + String(result1) + " bytes");
  Serial.println("wifi_password: " + String(result2) + " bytes");
  Serial.println("wifi_static_ip: " + String(result3) + " bytes");
  Serial.println("wifi_gateway: " + String(result4) + " bytes");
  Serial.println("wifi_subnet: " + String(result5) + " bytes");
  Serial.println("wifi_dns: " + String(result6) + " bytes");
  
  preferences.end();
  
  if (result1 > 0 && result2 > 0 && result3 > 0 && result4 > 0 && result5 > 0 && result6 > 0) {
    Serial.println("Konfigurasi WiFi berhasil disimpan ke Preferences.");
    return true;
  } else {
    Serial.println("ERROR: Gagal menyimpan beberapa konfigurasi WiFi!");
    return false;
  }
}

// Memuat konfigurasi WiFi dari Preferences
bool loadWifiConfig() {
  preferences.begin(PREFERENCES_NAMESPACE, true);
  
  String savedSSID = preferences.getString("wifi_ssid", "My Phone");
  String savedPassword = preferences.getString("wifi_password", "kalolaparmakan");
  String savedStaticIP = preferences.getString("wifi_static_ip", "192.168.121.14");
  String savedGateway = preferences.getString("wifi_gateway", "192.168.121.99");
  String savedSubnet = preferences.getString("wifi_subnet", "255.255.255.0");
  String savedDNS = preferences.getString("wifi_dns", "192.168.121.99");
  
  preferences.end();
  
  // Update variabel global di config.h
  savedSSID.toCharArray(ssid, sizeof(ssid));
  savedPassword.toCharArray(password, sizeof(password));
  savedStaticIP.toCharArray(staticIPStr, sizeof(staticIPStr));
  savedGateway.toCharArray(gatewayStr, sizeof(gatewayStr));
  savedSubnet.toCharArray(subnetStr, sizeof(subnetStr));
  savedDNS.toCharArray(dnsStr, sizeof(dnsStr));
  
  // Update IP Address objects
  updateIPAddressesFromStrings();
  
  Serial.println("Konfigurasi WiFi dimuat dari Preferences:");
  Serial.println("SSID: " + savedSSID);
  Serial.println("Static IP: " + savedStaticIP);
  Serial.println("Gateway: " + savedGateway);
  
  return true;
}

// Fungsi untuk mengupdate IP Address objects dari string
void updateIPAddressesFromStrings() {
  staticIP.fromString(staticIPStr);
  gateway.fromString(gatewayStr);
  subnet.fromString(subnetStr);
  dns.fromString(dnsStr);
  
  Serial.println("IP Address objects telah diupdate dari string values.");
}

// Handler untuk halaman konfigurasi WiFi
void handleWifiConfig() {
  String html = "<!DOCTYPE html>";
  html += "<html><head><title>Konfigurasi WiFi AGV</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;margin:20px;background:#f0f0f0}";
  html += ".container{max-width:500px;margin:0 auto;background:white;padding:20px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}";
  html += "h1{color:#333;text-align:center}";
  html += "input[type=text],input[type=password]{width:100%;padding:10px;margin:5px 0;border:1px solid #ddd;border-radius:5px;box-sizing:border-box}";
  html += "button{background:#4CAF50;color:white;padding:12px 20px;border:none;border-radius:5px;cursor:pointer;width:100%;font-size:16px}";
  html += "button:hover{background:#45a049}";
  html += ".form-group{margin-bottom:15px}";
  html += "label{display:block;margin-bottom:5px;font-weight:bold}";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>Konfigurasi WiFi AGV</h1>";
  html += "<form action='/save-wifi' method='POST'>";
  html += "<div class='form-group'>";
  html += "<label for='ssid'>SSID WiFi:</label>";
  html += "<input type='text' id='ssid' name='ssid' required>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='password'>Password WiFi:</label>";
  html += "<input type='password' id='password' name='password' required>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='static_ip'>Static IP:</label>";
  html += "<input type='text' id='static_ip' name='static_ip' placeholder='192.168.1.100' required>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='gateway'>Gateway:</label>";
  html += "<input type='text' id='gateway' name='gateway' placeholder='192.168.1.1' required>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='subnet'>Subnet Mask:</label>";
  html += "<input type='text' id='subnet' name='subnet' value='255.255.255.0' required>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label for='dns'>DNS Server:</label>";
  html += "<input type='text' id='dns' name='dns' placeholder='Gunakan gateway sebagai DNS' required>";
  html += "</div>";
  html += "<button type='submit'>Simpan Konfigurasi</button>";
  html += "</form>";
  html += "<br><a href='/'>Kembali ke Menu Utama</a>";
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

// Handler untuk menyimpan konfigurasi WiFi
void handleSaveWifi() {
  Serial.println("=== handleSaveWifi dipanggil ===");
  Serial.println("Method: " + server.method());
  Serial.println("URI: " + server.uri());
  Serial.println("Args count: " + String(server.args()));
  
  // Print semua arguments yang diterima
  for (int i = 0; i < server.args(); i++) {
    Serial.println("Arg " + String(i) + ": " + server.argName(i) + " = " + server.arg(i));
  }
  
  if (server.hasArg("ssid") && server.hasArg("password") && 
      server.hasArg("static_ip") && server.hasArg("gateway") && 
      server.hasArg("subnet") && server.hasArg("dns")) {
    
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    String staticIP = server.arg("static_ip");
    String gateway = server.arg("gateway");
    String subnet = server.arg("subnet");
    String dns = server.arg("dns");
    
    Serial.println("=== Data yang akan disimpan ===");
    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);
    Serial.println("Static IP: " + staticIP);
    Serial.println("Gateway: " + gateway);
    Serial.println("Subnet: " + subnet);
    Serial.println("DNS: " + dns);
    
    if (saveWifiConfig(ssid, password, staticIP, gateway, subnet, dns)) {
      String html = "<!DOCTYPE html>";
      html += "<html><head><title>Konfigurasi Tersimpan</title>";
      html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
      html += "<style>body{font-family:Arial,sans-serif;margin:20px;text-align:center;background:#f0f0f0}";
      html += ".container{max-width:400px;margin:0 auto;background:white;padding:20px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}";
      html += "h1{color:#4CAF50}";
      html += "button{background:#4CAF50;color:white;padding:10px 20px;border:none;border-radius:5px;cursor:pointer;margin:10px}";
      html += "</style></head><body>";
      html += "<div class='container'>";
      html += "<h1>Konfigurasi Berhasil Disimpan!</h1>";
      html += "<p>ESP32 akan restart untuk menerapkan konfigurasi baru.</p>";
      html += "<button onclick='window.location.href=\"/\"'>Kembali ke Menu</button>";
      html += "<script>setTimeout(function(){alert('ESP32 akan restart sekarang!');}, 3000);</script>";
      html += "</div></body></html>";
      
      server.send(200, "text/html", html);
      
      // Restart ESP32 setelah 5 detik
      delay(5000);
      ESP.restart();
    } else {
      server.send(500, "text/plain", "Gagal menyimpan konfigurasi WiFi.");
    }
  } else {
    Serial.println("=== Parameter tidak lengkap ===");
    Serial.println("hasArg ssid: " + String(server.hasArg("ssid")));
    Serial.println("hasArg password: " + String(server.hasArg("password")));
    Serial.println("hasArg static_ip: " + String(server.hasArg("static_ip")));
    Serial.println("hasArg gateway: " + String(server.hasArg("gateway")));
    Serial.println("hasArg subnet: " + String(server.hasArg("subnet")));
    Serial.println("hasArg dns: " + String(server.hasArg("dns")));
    
    String errorMsg = "Parameter tidak lengkap. Missing: ";
    if (!server.hasArg("ssid")) errorMsg += "ssid ";
    if (!server.hasArg("password")) errorMsg += "password ";
    if (!server.hasArg("static_ip")) errorMsg += "static_ip ";
    if (!server.hasArg("gateway")) errorMsg += "gateway ";
    if (!server.hasArg("subnet")) errorMsg += "subnet ";
    if (!server.hasArg("dns")) errorMsg += "dns ";
    
    server.send(400, "text/plain", errorMsg);
  }
}

// Handler untuk halaman utama dengan menu
void handleRoot() {
  String html = "<!DOCTYPE html>";
  html += "<html><head><title>AGV Control Panel</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;margin:20px;background:#f0f0f0}";
  html += ".container{max-width:600px;margin:0 auto;background:white;padding:20px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}";
  html += "h1{color:#333;text-align:center}";
  html += ".menu-item{background:#4CAF50;color:white;padding:15px;margin:10px 0;border-radius:5px;text-decoration:none;display:block;text-align:center;font-size:16px}";
  html += ".menu-item:hover{background:#45a049}";
  html += ".status{background:#e7f3ff;padding:10px;border-radius:5px;margin:10px 0}";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>AGV Control Panel</h1>";
  html += "<div class='status'>";
  html += "<strong>Status:</strong> Online<br>";
  html += "<strong>IP:</strong> " + WiFi.localIP().toString() + "<br>";
  html += "<strong>SSID:</strong> " + WiFi.SSID();
  html += "</div>";
  html += "<a href='/wifi-config' class='menu-item'>Konfigurasi WiFi</a>";
  html += "<a href='/showstations' class='menu-item'>Lihat Daftar Stasiun</a>";
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}