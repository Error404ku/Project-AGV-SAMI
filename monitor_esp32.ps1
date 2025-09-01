Write-Host "Starting ESP32 RPM Test Monitor - 30 seconds" -ForegroundColor Green

try {
    $port = New-Object System.IO.Ports.SerialPort('COM11', 115200)
    $port.Open()
    Write-Host "Serial port opened successfully" -ForegroundColor Yellow
    
    $startTime = Get-Date
    $lineCount = 0
    
    while (((Get-Date) - $startTime).TotalSeconds -lt 30) {
        if ($port.BytesToRead -gt 0) {
            try {
                $data = $port.ReadLine()
                $lineCount++
                $timestamp = (Get-Date).ToString("HH:mm:ss.fff")
                Write-Host "[$timestamp] $data"
            }
            catch {
                # Ignore read errors and continue
            }
        }
        Start-Sleep -Milliseconds 50
    }
    
    $port.Close()
    Write-Host "`nMonitoring completed. Total lines captured: $lineCount" -ForegroundColor Yellow
}
catch {
    Write-Host "Error: $($_.Exception.Message)" -ForegroundColor Red
}
