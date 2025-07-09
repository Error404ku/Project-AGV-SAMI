bool saveStationsListToPreferences() {
  stationsPreferences.begin(PREFERENCES_NAMESPACE_STATIONS, false); // Buka untuk menulis
  stationsPreferences.clear(); // Hapus semua data lama di namespace ini

  Serial.println("Menyimpan daftar stasiun ke Preferences...");
  // Ukuran buffer JSON, disesuaikan untuk daftar stasiun (misal 5 integer = ~50-100 byte)
  // 1024 byte sudah sangat cukup untuk banyak integer.
  DynamicJsonDocument doc(1024); 
  JsonArray array = doc.to<JsonArray>();
  for (int id : stationsList) {
    array.add(id);
  }

  String outputString;
  size_t bytesSerialized = serializeJson(doc, outputString); // Serialisasi JSON ke string

  if (bytesSerialized == 0) {
    Serial.println("Error: Gagal menserialisasi daftar stasiun, mungkin buffer JSON terlalu kecil.");
    stationsPreferences.end();
    return false;
  }
  
  Serial.println("JSON daftar stasiun yang akan disimpan: " + outputString);
  
  size_t bytesWritten = stationsPreferences.putString("stations_array", outputString); // Simpan string JSON

  stationsPreferences.end(); // Tutup sesi Preferences

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
  stationsPreferences.begin(PREFERENCES_NAMESPACE_STATIONS, true); // Buka untuk membaca
  stationsList.clear(); // Kosongkan daftar di RAM sebelum memuat

  Serial.println("Memuat daftar stasiun dari Preferences ke RAM...");
  String storedStations = stationsPreferences.getString("stations_array", "[]"); // Ambil string JSON
  stationsPreferences.end(); // Tutup sesi Preferences

  Serial.println("Data stasiun tersimpan: " + storedStations);

  DynamicJsonDocument doc(1024); // Ukuran buffer JSON untuk deserialisasi
  DeserializationError error = deserializeJson(doc, storedStations); // Deserialisasi string JSON

  if (!error) { // Jika TIDAK ada error saat deserialisasi
    JsonArray array = doc.as<JsonArray>();
    if (array) { // Pastikan itu adalah JsonArray yang valid
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

  DynamicJsonDocument doc(1024); // Ukuran buffer untuk deserialisasi JSON input
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  
  // Validasi JSON input: Pastikan ada field "stations" dan itu adalah array
  if (error || !doc.is<JsonObject>() || !doc.containsKey("stations") || !doc["stations"].is<JsonArray>()) {
    Serial.print("JSON invalid atau format salah (handleUpdateStations): ");
    Serial.println(error.c_str());
    server.send(400, "application/json", "{\"status\":\"error\", \"message\":\"JSON tidak valid atau format salah. Harap kirim { \\\"stations\\\": [1,2,3] }\" }");
    return;
  }

  stationsList.clear(); // Kosongkan daftar stasiun yang lama di RAM
  JsonArray receivedStations = doc["stations"].as<JsonArray>();
  for(JsonVariant v : receivedStations) {
    stationsList.push_back(v.as<int>());
  }
  Serial.println("Variabel RAM 'stationsList' telah diperbarui dengan " + String(stationsList.size()) + " elemen.");

  if (saveStationsListToPreferences()) { // Simpan ke Preferences
    server.send(200, "application/json", "{\"status\":\"ok\", \"message\":\"Daftar stasiun berhasil diperbarui di RAM dan flash.\" }");
  } else {
    server.send(500, "application/json", "{\"status\":\"error\", \"message\":\"Gagal menyimpan data ke penyimpanan.\" }");
  }
}

// Handler untuk menampilkan daftar stasiun yang ditemukan (stationsList)
void handleShowStations() {
  DynamicJsonDocument doc(1024); // Ukuran buffer untuk respons JSON (bisa disesuaikan)
  JsonArray array = doc.to<JsonArray>();
  for (int id : stationsList) {
    array.add(id);
  }
  String output;
  serializeJsonPretty(doc, output); // Serialisasi dengan format PrettyPrint
  
  Serial.println("Mengirim daftar stasiun dari RAM.");
  server.send(200, "application/json", output);
}

// --- FUNGSI UNTUK MENGOSONGKAN DAFTAR STATION ---
void clearStationsData() {
  // 1. Kosongkan std::vector di RAM
  stationsList.clear();
  Serial.println("Daftar station di RAM telah dikosongkan.");

  // 2. Kosongkan data di Preferences
  stationsPreferences.begin(PREFERENCES_NAMESPACE_STATIONS, false); // Buka untuk menulis
  stationsPreferences.clear(); // Hapus semua data di namespace ini
  stationsPreferences.end(); // Tutup sesi Preferences
  Serial.println("Data station di Preferences telah dihapus.");
}