Param()

# Comprueba si el puerto 3333 ya está en uso (OpenOCD ya corriendo)
try {
    $conn = Get-NetTCPConnection -LocalPort 3333 -ErrorAction SilentlyContinue
} catch {
    $conn = $null
}

if ($conn) {
    Write-Output "OpenOCD already running on port 3333 (OwningProcess: $($conn.OwningProcess))"
    exit 0
}

# Ruta al ejecutable OpenOCD (ajusta si es necesario)
$openocdPath = 'C:\Espressif\tools\openocd-esp32\v0.12.0-esp32-20251215\openocd-esp32\bin\openocd.exe'

if (-not (Test-Path $openocdPath)) {
    Write-Error "openocd.exe not found at $openocdPath"
    exit 1
}

Write-Output "Starting OpenOCD: $openocdPath -f board/esp32c6-builtin.cfg"
& $openocdPath -f board/esp32c6-builtin.cfg
