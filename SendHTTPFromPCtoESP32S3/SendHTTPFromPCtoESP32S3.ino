#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// --- KONFIGURASI ---
// Ganti dengan nama dan password jaringan Wi-Fi Anda
const char* ssid = "My Phone";
const char* password = "kalolaparmakan";

// Membuat objek server web pada port 80 (HTTP standar)
WebServer server(80);

// --- SETUP ---
// Fungsi ini hanya berjalan sekali saat ESP32 pertama kali dinyalakan atau di-reset
void setup() {
  // Mulai komunikasi serial untuk debugging
  Serial.begin(115200);

  // Mulai koneksi ke Wi-Fi
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Tunggu hingga koneksi berhasil
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Jika terhubung, cetak alamat IP ESP32
  Serial.println("\nKoneksi Wi-Fi berhasil!");
  Serial.print("Alamat IP Server ESP32-S3: ");
  Serial.println(WiFi.localIP());

  // --- DEFINISI ENDPOINT SERVER ---
  // Endpoint untuk menangani permintaan GET ke root "/"
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", "<h1>Selamat Datang di Server ESP32-S3</h1><p>Gunakan endpoint /data dengan metode POST untuk mengirim JSON.</p>");
  });

  // Endpoint utama kita: menangani permintaan POST ke "/data"
  server.on("/data", HTTP_POST, handleJsonPost);

  // Endpoint jika halaman tidak ditemukan (404 Not Found)
  server.onNotFound([]() {
    server.send(404, "text/plain", "Error: Endpoint tidak ditemukan.");
  });

  // Mulai server
  server.begin();
  Serial.println("Server HTTP telah dimulai.");
}

// --- LOOP UTAMA ---
// Fungsi ini berjalan berulang-ulang
void loop() {
  // Tangani permintaan klien yang masuk
  server.handleClient();
}

// --- FUNGSI HANDLER ---
// Fungsi ini akan dipanggil ketika ada permintaan POST ke /data
void handleJsonPost() {
  // Periksa apakah permintaan berisi data
  if (server.hasArg("plain") == false) {
    // Jika tidak ada data, kirim error
    server.send(400, "text/plain", "Bad Request: Tidak ada data body.");
    return;
  }

  // Ambil data dari body permintaan
  String jsonBody = server.arg("plain");
  Serial.println("Menerima body JSON:");
  Serial.println(jsonBody);

  // Alokasikan memori untuk dokumen JSON.
  // Ukuran 256 byte sudah cukup untuk contoh kita. Sesuaikan jika JSON lebih kompleks.
  StaticJsonDocument<256> doc;

  // Lakukan deserialisasi (parsing) string JSON ke dalam dokumen
  DeserializationError error = deserializeJson(doc, jsonBody);

  // Periksa jika terjadi error saat parsing
  if (error) {
    Serial.print("deserializeJson() gagal: ");
    Serial.println(error.c_str());
    server.send(400, "text/plain", "Bad Request: Format JSON tidak valid.");
    return;
  }

  // Jika parsing berhasil, akses data berdasarkan 'key'
  // Pastikan key ini cocok dengan yang dikirim dari skrip Python
  const char* sensor_type = doc["sensor"]; // contoh: "suhu"
  long time_stamp = doc["time"];           // contoh: 1678886400
  float value = doc["value"];              // contoh: 27.5

  // Cetak data yang sudah di-parsing ke Serial Monitor untuk verifikasi
  Serial.println("Data berhasil di-parsing:");
  Serial.print("Tipe Sensor: ");
  Serial.println(sensor_type);
  Serial.print("Timestamp: ");
  Serial.println(time_stamp);
  Serial.print("Nilai: ");
  Serial.println(value);

  // Kirim balasan ke klien bahwa data telah diterima dengan sukses
  server.send(200, "application/json", "{\"status\":\"ok\", \"message\":\"Data diterima\"}");
}