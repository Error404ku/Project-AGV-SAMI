# PowerShell script untuk menghapus semua Serial.print debug dari kode production
# Hanya menyisakan Serial.print yang diperlukan untuk komunikasi motor

$agvPath = "C:\Users\NITRO 5\Documents\GitHub\Project-AGV-SAMI\agv_sami"

# File yang akan dibersihkan dari Serial.print
$filesToClean = @(
    "pembacaanTombol.ino",
    "menu.ino", 
    "pidMagnetFollower.ino",
    "sensorRfid.ino",
    "sensorUltrasonik.ino",
    "wifi.ino",
    "logicAgv.ino",
    "logicAndPotitionsPos.ino",
    "performance_optimization.ino"
)

foreach ($file in $filesToClean) {
    $filePath = Join-Path $agvPath $file
    if (Test-Path $filePath) {
        Write-Host "Cleaning $file..."
        
        # Baca isi file
        $content = Get-Content $filePath -Raw
        
        # Hapus atau comment Serial.print dan Serial.println
        $content = $content -replace 'Serial\.print\([^;]*\);', '// Serial.print() - removed for production'
        $content = $content -replace 'Serial\.println\([^;]*\);', '// Serial.println() - removed for production'
        
        # Tulis kembali ke file
        Set-Content $filePath $content -NoNewline
        
        Write-Host "Cleaned $file"
    } else {
        Write-Host "File not found: $file"
    }
}

Write-Host "Debug cleanup completed!"
