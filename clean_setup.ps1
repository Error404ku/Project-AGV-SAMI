# Script untuk membersihkan Serial.print di setup.ino
$filePath = "C:\Users\NITRO 5\Documents\GitHub\Project-AGV-SAMI\agv_sami\setup.ino"

if (Test-Path $filePath) {
    Write-Host "Membersihkan Serial.print di setup.ino..."
    
    # Baca isi file
    $content = Get-Content $filePath -Raw
    
    # Hapus semua Serial.print, Serial.println, dan Serial.printf
    $content = $content -replace 'Serial\.println\([^;]*\);', '// Serial debug removed for production'
    $content = $content -replace 'Serial\.printf\([^;]*\);', '// Serial debug removed for production'  
    $content = $content -replace 'Serial\.print\([^;]*\);', '// Serial debug removed for production'
    
    # Tulis kembali ke file
    Set-Content $filePath $content -NoNewline
    
    Write-Host "setup.ino telah dibersihkan dari Serial debug statements"
} else {
    Write-Host "File setup.ino tidak ditemukan"
}
