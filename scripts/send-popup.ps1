param(
    [string]$Title = "Hello",
    [string]$Body  = "This is a test popup message",
    [string]$BorderColor = "",
    [string]$BgColor = "",
    [int]$Port = 7777
)

$msg = "popup $Title|$Body"
if ($BorderColor) { $msg += "|$BorderColor" }
if ($BgColor)     { $msg += "|$BgColor" }

$client = New-Object System.Net.Sockets.TcpClient("127.0.0.1", $Port)
$stream = $client.GetStream()
$bytes  = [System.Text.Encoding]::UTF8.GetBytes("$msg`n")
$stream.Write($bytes, 0, $bytes.Length)
$stream.Flush()
Start-Sleep -Milliseconds 100
$client.Close()

Write-Host "Sent: $msg"
