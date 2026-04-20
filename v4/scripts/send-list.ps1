param(
    [string]$Items = "Task 1\nTask 2\nTask 3",
    [int]$Width = 400,
    [string]$Position = "100,100",
    [switch]$Clear,
    [int]$Port = 7777
)

if ($Clear) {
    $msg = "list clear"
} else {
    $msg = "list set $Items|$Width|$Position"
}

$client = New-Object System.Net.Sockets.TcpClient("127.0.0.1", $Port)
$stream = $client.GetStream()
$bytes  = [System.Text.Encoding]::UTF8.GetBytes("$msg`n")
$stream.Write($bytes, 0, $bytes.Length)
$stream.Flush()
Start-Sleep -Milliseconds 100
$client.Close()

Write-Host "Sent: $msg"
