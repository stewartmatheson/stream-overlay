param(
    [string]$Id = "default",
    [string]$Text = "[b][color=73daca]Building all my stream overlay software from scratch till its done! 10 streams in a row so far...[/color][/b]",
    [int]$Width = 0,
    [string]$Position = "220,47",
    [switch]$Clear,
    [int]$Port = 7777
)

if ($Clear) {
    if ($Id -eq "default") {
        $msg = "label clear"
    } else {
        $msg = "label clear $Id"
    }
} elseif ($Width -gt 0) {
    $msg = "label set $Id|$Text|$Width|$Position"
} else {
    $msg = "label set $Id|$Text|$Position"
}

$client = New-Object System.Net.Sockets.TcpClient("127.0.0.1", $Port)
$stream = $client.GetStream()
$bytes  = [System.Text.Encoding]::UTF8.GetBytes("$msg`n")
$stream.Write($bytes, 0, $bytes.Length)
$stream.Flush()
Start-Sleep -Milliseconds 100
$client.Close()

Write-Host "Sent: $msg"
