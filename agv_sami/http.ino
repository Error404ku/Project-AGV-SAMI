// --- FUNGSI PERSISTENSI & PEMUATAN UNTUK DEVICE MAP ---
bool saveMapToPreferences() {
  preferences.begin(PREFERENCES_NAMESPACE, false);
  preferences.clear();
  Serial.println("Menyimpan map ke Preferences...");
  for (const auto& pair : deviceMap) {
    DynamicJsonDocument doc(256);
    JsonArray array = doc.to<JsonArray>();
    for (const String& val : pair.second) {
      array.add(val);
    }
    String outputString;
    serializeJson(doc, outputString);
    preferences.putString(String(pair.first).c_str(), outputString);
  }
  preferences.end();
  Serial.println("Berhasil disimpan.");
  return true;
}

bool loadMapFromPreferences() {
  preferences.begin(PREFERENCES_NAMESPACE, true);
  deviceMap.clear();
  Serial.println("Memuat data dari Preferences ke map...");
  for (int i = 0; i <= 50; i++) { // Assuming keys up to 50
    String key = String(i);
    if (preferences.isKey(key.c_str())) {
      String storedVectorString = preferences.getString(key.c_str(), "[]");
      DynamicJsonDocument doc(256);
      deserializeJson(doc, storedVectorString);
      std::vector<String> values;
      for (JsonVariant v : doc.as<JsonArray>()) {
        values.push_back(v.as<String>());
      }
      deviceMap[i] = values;
    }
  }
  preferences.end();
  Serial.println("Selesai memuat data.");
  return true;
}


// --- FUNGSI PERSISTENSI & PEMUATAN UNTUK STATION ---

bool saveStationsToPreferences() {
  stationsPreferences.begin(STATIONS_NAMESPACE, false);
  stationsPreferences.clear();
  Serial.println("Menyimpan daftar station ke Preferences...");

  DynamicJsonDocument doc(512);
  JsonArray array = doc.to<JsonArray>();
  for (int key : stationsList) {
    array.add(key);
  }
  String outputString;
  serializeJson(doc, outputString);
  stationsPreferences.putString("stations_array", outputString);

  stationsPreferences.end();
  Serial.println("Berhasil disimpan daftar station.");
  return true;
}

bool loadStationsFromPreferences() {
  stationsPreferences.begin(STATIONS_NAMESPACE, true);
  stationsList.clear();
  Serial.println("Memuat daftar station dari Preferences...");

  String storedKeysString = stationsPreferences.getString("stations_array", "[]");
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, storedKeysString);

  if (!error) {
    JsonArray array = doc.as<JsonArray>();
    for (JsonVariant v : array) {
      stationsList.push_back(v.as<int>());
    }
    Serial.println("Selesai memuat daftar station.");
  } else {
    Serial.print("Gagal memuat daftar station: ");
    Serial.println(error.c_str());
  }
  stationsPreferences.end();
  return true;
}


// --- FUNGSI PENCARIAN ---
int findKeyByStringValue(const String& valueToFind) {
  for (const auto& pair : deviceMap) {
    for (const String& info : pair.second) {
      if (info == valueToFind) {
        return pair.first;
      }
    }
  }
  return -1;
}

// --- HANDLER ENDPOINT ---

void handleUpdateRequest() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Body JSON tidak ada.");
    return;
  }

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    server.send(400, "text/plain", "JSON tidak valid.");
    return;
  }

  deviceMap.clear();
  JsonObject newJsonObj = doc.as<JsonObject>();
  Serial.println("Map di RAM diperbarui dengan data lowercase:");
  for (JsonPair pair : newJsonObj) {
    int key = atoi(pair.key().c_str());
    std::vector<String> values;

    for (JsonVariant v : pair.value().as<JsonArray>()) {
      String strValue = v.as<String>();
      strValue.toLowerCase();  // Konversi ke lowercase
      values.push_back(strValue);
    }
    deviceMap[key] = values;
  }

  if (saveMapToPreferences()) {
    server.send(200, "application/json", "{\"status\":\"ok\", \"message\":\"Data diperbarui dan disimpan sebagai lowercase\"}");
  } else {
    server.send(500, "application/json", "{\"status\":\"error\", \"message\":\"Gagal menyimpan perubahan\"}");
  }
}

void handleFindRequest() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Body JSON tidak ada.");
    return;
  }

  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    server.send(400, "text/plain", "JSON tidak valid.");
    return;
  }

  if (!doc.containsKey("kode_mesin")) {
    server.send(400, "application/json", "{\"status\":\"error\", \"message\":\"Field 'kode_mesin' tidak ada dalam JSON\"}");
    return;
  }

  String searchValue = doc["kode_mesin"].as<String>();
  searchValue.toLowerCase();

  Serial.print("Mencari nilai (sudah di-lowercase): '");
  Serial.print(searchValue);
  Serial.println("'...");

  int foundID = findKeyByStringValue(searchValue);

  DynamicJsonDocument responseDoc(256);
  responseDoc["status"] = "not_found";

  if (foundID != -1) {
    // Cek apakah station sudah ada di stationsList
    auto it = std::find(stationsList.begin(), stationsList.end(), foundID);
    if (it == stationsList.end()) { // Jika station TIDAK ditemukan di stationsList
      stationsList.push_back(foundID); // Tambahkan station yang ditemukan ke daftar
      if (saveStationsToPreferences()) { // Simpan daftar yang diperbarui
        responseDoc["status"] = "ok";
        responseDoc["station_id"] = foundID;

        // Tambahkan array station yang ditemukan ke respons JSON
        JsonArray foundArray = responseDoc.createNestedArray("all_stations");
        for (int key : stationsList) {
          foundArray.add(key);
        }
        Serial.println("Station ditemukan dan disimpan ke daftar: " + String(foundID));
      } else {
        responseDoc["status"] = "error";
        responseDoc["message"] = "Station ditemukan tetapi gagal disimpan ke Preferences.";
      }
    } else { // Jika station SUDAH ada di stationsList
      responseDoc["status"] = "ok";
      responseDoc["station_id"] = foundID; // Gunakan station_id konsisten
      responseDoc["message"] = "Station sudah ada di daftar.";

      // Tetap sertakan array station yang ditemukan ke respons JSON
      JsonArray foundArray = responseDoc.createNestedArray("all_stations");
      for (int key : stationsList) {
        foundArray.add(key);
      }
      Serial.println("Station sudah ada di daftar: " + String(foundID));
    }
  } else {
    Serial.println("Station tidak ditemukan.");
  }

  String output;
  serializeJson(responseDoc, output);
  server.send(foundID != -1 ? 200 : 404, "application/json", output);
}


void handleShowMapRequest() {
  DynamicJsonDocument doc(1024);
  for (const auto& pair : deviceMap) {
    JsonArray array = doc.createNestedArray(String(pair.first));
    for (const String& val : pair.second) {
      array.add(val);
    }
  }
  String output;
  serializeJsonPretty(doc, output);
  server.send(200, "application/json", output);
}

// --- HANDLER: Tampilkan Station yang Ditemukan ---
void handleShowStationsRequest() {
  DynamicJsonDocument doc(512);
  JsonArray array = doc.to<JsonArray>();
  for (int key : stationsList) {
    array.add(key);
  }
  String output;
  serializeJsonPretty(doc, output);
  server.send(200, "application/json", output);
}

void sortStationsList() {
  // Mengurutkan daftar station secara ascending (dari kecil ke besar)
  std::sort(stationsList.begin(), stationsList.end());
  Serial.println("Daftar station telah diurutkan.");

  // Simpan daftar yang sudah diurutkan ke Preferences
  if (saveStationsToPreferences()) {
    Serial.println("Daftar station berhasil diurutkan dan disimpan ke Preferences.");
  } else {
    Serial.println("Gagal menyimpan daftar station yang diurutkan ke Preferences.");
    // Anda mungkin ingin menambahkan penanganan kesalahan di sini,
    // misalnya mencoba menyimpan lagi atau mencetak pesan kesalahan yang lebih detail.
  }
}