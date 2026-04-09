param(
    [string]$Title = "Hello",
    [string]$Body  = "This is a test bottom popup message",
    [string]$BgColor = "",
    [int]$TopBorderThickness = 0,
    [string]$TopBorderColor = "",
    [int]$Port = 7777
)

$msg = "bottompopup $Title|$Body||"
if ($BgColor) { $msg += "$BgColor" }
if ($TopBorderThickness -gt 0) {
    $msg += "|$TopBorderThickness"
    if ($TopBorderColor) { $msg += "|$TopBorderColor" }
}

$client = New-Object System.Net.Sockets.TcpClient("127.0.0.1", $Port)
$stream = $client.GetStream()
$bytes  = [System.Text.Encoding]::UTF8.GetBytes("$msg`n")
$stream.Write($bytes, 0, $bytes.Length)
$stream.Flush()
Start-Sleep -Milliseconds 100
$client.Close()

Write-Host "Sent: $msg"
