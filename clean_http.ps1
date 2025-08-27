# Script untuk membersihkan sisa Serial.print di http.ino
$filePath = "C:\Users\NITRO 5\Documents\GitHub\Project-AGV-SAMI\agv_sami\http.ino"

if (Test-Path $filePath) {
    Write-Host "Membersihkan sisa Serial.print di http.ino..."
    
    # Baca isi file
    $content = Get-Content $filePath -Raw
    
    # Pattern untuk menghapus Serial.print dan Serial.println yang masih ada
    $patterns = @(
        'Serial\.println\("Mengirim daftar stasiun dari RAM\."\);',
        'Serial\.println\("Mengirim daftar alamat station RFID\."\);',
        'Serial\.println\("Mengirim daftar ujung station RFID\."\);',
        'Serial\.println\("Mengirim daftar warehouse RFID\."\);',
        'Serial\.println\("Mengirim data terminal RFID\."\);',
        'Serial\.println\("Daftar station di RAM telah dikosongkan\."\);',
        'Serial\.println\("Data station di Preferences telah.*dihapus\."\);',
        'Serial\.println\("=== saveWifiConfig dipanggil ==="\);',
        'Serial\.println\("Namespace: " \+ String\(PREFERENCES_NAMESPACE\)\);',
        'Serial\.println\("ERROR: Gagal membuka Preferences namespace!"\);',
        'Serial\.println\("Preferences berhasil dibuka, mulai menyimpan\.\.\."\);',
        'Serial\.println\("Hasil penyimpanan:"\);',
        'Serial\.println\("wifi_.*: " \+ String\(.*\) \+ " bytes"\);',
        'Serial\.println\("Konfigurasi WiFi berhasil disimpan ke Preferences\."\);',
        'Serial\.println\("ERROR: Gagal menyimpan beberapa konfigurasi WiFi!"\);',
        'Serial\.println\("Konfigurasi WiFi dimuat dari Preferences:"\);',
        'Serial\.println\("SSID: " \+ savedSSID\);',
        'Serial\.println\("Static IP: " \+ savedStaticIP\);',
        'Serial\.println\("Gateway: " \+ savedGateway\);',
        'Serial\.println\("IP Address objects telah diupdate dari string values\."\);',
        'Serial\.println\("=== handleSaveWifi dipanggil ==="\);',
        'Serial\.println\("Method: " \+ server\.method\(\)\);',
        'Serial\.println\("URI: " \+ server\.uri\(\)\);',
        'Serial\.println\("Args count: " \+ String\(server\.args\(\)\)\);',
        'Serial\.println\("Arg " \+ String\(i\) \+ ": " \+ server\.argName\(i\) \+ " = " \+ server\.arg\(i\)\);',
        'Serial\.println\("=== Data yang akan disimpan ==="\);',
        'Serial\.println\("SSID: " \+ ssid\);',
        'Serial\.println\("Password: " \+ password\);',
        'Serial\.println\("Static IP: " \+ staticIP\);',
        'Serial\.println\("Gateway: " \+ gateway\);',
        'Serial\.println\("Subnet: " \+ subnet\);',
        'Serial\.println\("DNS: " \+ dns\);',
        'Serial\.println\("=== Parameter tidak lengkap ==="\);',
        'Serial\.println\("hasArg.*: " \+ String\(server\.hasArg\(.*\)\)\);'
    )
    
    foreach ($pattern in $patterns) {
        $content = $content -replace $pattern, '// Serial debug removed for production'
    }
    
    # Tulis kembali ke file
    Set-Content $filePath $content -NoNewline
    
    Write-Host "http.ino telah dibersihkan dari Serial debug statements"
} else {
    Write-Host "File http.ino tidak ditemukan"
}
