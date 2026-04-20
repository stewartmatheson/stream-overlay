param(
    [string]$Label = "Timer",
    [string]$Duration = "00:02",
    [string]$Position = "100,50",
    [switch]$Clear,
    [int]$Port = 7777
)

if ($Clear) {
    $msg = "timer_clear"
} else {
    $msg = "timer $Label|$Duration|$Position"
}

$client = New-Object System.Net.Sockets.TcpClient("127.0.0.1", $Port)
$stream = $client.GetStream()
$bytes  = [System.Text.Encoding]::UTF8.GetBytes("$msg`n")
$stream.Write($bytes, 0, $bytes.Length)
$stream.Flush()
Start-Sleep -Milliseconds 100
$client.Close()

Write-Host "Sent: $msg"
