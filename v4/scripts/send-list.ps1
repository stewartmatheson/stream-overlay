param(
    [string[]]$Items = @("[b]Task 1[/b]", "Task 2", "Task 3", "Task 4", "Task 5", "Task 6", "Task 7"),
    [int]$Width = 285,
    [string]$Position = "1598,475",
    [string]$Title = "tasks",
    [switch]$Clear,
    [int]$Port = 7777
)

if ($Clear) {
    $msg = "list clear"
} else {
    $joined = $Items -join "\n"
    $msg = "list set $joined|$Width|$Position|$Title"
}

$client = New-Object System.Net.Sockets.TcpClient("127.0.0.1", $Port)
$stream = $client.GetStream()
$bytes  = [System.Text.Encoding]::UTF8.GetBytes("$msg`n")
$stream.Write($bytes, 0, $bytes.Length)
$stream.Flush()
Start-Sleep -Milliseconds 100
$client.Close()

Write-Host "Sent: $msg"
