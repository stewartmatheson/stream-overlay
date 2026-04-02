param(
    [string] $WsUri    = "ws://127.0.0.1:8080/",
    [int]    $BallPort = 7777
)

function Send-OverlayCommand([string]$cmd) {
    try {
        $tcp    = [System.Net.Sockets.TcpClient]::new("127.0.0.1", $BallPort)
        $stream = $tcp.GetStream()
        $bytes  = [System.Text.Encoding]::ASCII.GetBytes("$cmd`n")
        $stream.Write($bytes, 0, $bytes.Length)
        $stream.Flush()
        $tcp.Close()
        Write-Host "  -> Sent: $cmd"
    } catch {
        Write-Warning "Could not connect to overlay on port $BallPort - is it running?"
    }
}

function Send-SpawnBall {
    $cx  = Get-Random -Minimum 0   -Maximum 1920
    $cy  = Get-Random -Minimum 0   -Maximum 1080
    $vx  = [float](Get-Random -Minimum -300 -Maximum 300) / 100.0
    $vy  = [float](Get-Random -Minimum -300 -Maximum 300) / 100.0
    $r   = Get-Random -Minimum 0   -Maximum 256
    $g   = Get-Random -Minimum 0   -Maximum 256
    $b   = Get-Random -Minimum 0   -Maximum 256
    Send-OverlayCommand "spawn_ball $cx $cy $vx $vy $r $g $b"
}

function Send-BottomPopup([string]$title, [string]$body) {
    $safeTitle = $title -replace '\|', '-'
    $safeBody  = $body  -replace '\|', '-'
    Send-OverlayCommand "bottompopup $safeTitle|$safeBody"
}

function Get-AlertFromEvent($msg) {
    $source = $msg.event.source
    $type   = $msg.event.type
    $data   = $msg.data

    switch ("$source/$type") {
        "Twitch/Sub"           { return @{ Title = "New Subscriber!";   Body = "$($data.displayName) subscribed!" } }
        "Twitch/ReSub"         { return @{ Title = "Re-Sub!";           Body = "$($data.displayName) resubscribed for $($data.cumulativeMonths) months!" } }
        "Twitch/GiftSub"       { return @{ Title = "Gift Sub!";         Body = "$($data.displayName) gifted a sub to $($data.recipientDisplayName)!" } }
        "Twitch/GiftBomb"      { return @{ Title = "Gift Bomb!";        Body = "$($data.displayName) gifted $($data.gifts) subs!" } }
        "Twitch/Follow"        { return @{ Title = "New Follower!";     Body = "$($data.displayName) followed!" } }
        "Twitch/Cheer"         { return @{ Title = "Cheer!";            Body = "$($data.displayName) cheered $($data.bits) bits!" } }
        "Twitch/Raid"          { return @{ Title = "Raid!";             Body = "$($data.displayName) raided with $($data.viewers) viewers!" } }
        "Twitch/ChatMessage"   { return $null }
        default                { return @{ Title = "$source $type";     Body = if ($data.displayName) { $data.displayName } else { "" } } }
    }
}

function Receive-FullMessage($ws, $ct) {
    $bufSize = 8192
    $buf     = [byte[]]::new($bufSize)
    $sb      = [System.Text.StringBuilder]::new()

    do {
        $seg    = [System.ArraySegment[byte]]::new($buf)
        $result = $ws.ReceiveAsync($seg, $ct).GetAwaiter().GetResult()

        if ($result.MessageType -eq [System.Net.WebSockets.WebSocketMessageType]::Close) {
            return $null
        }

        $sb.Append([System.Text.Encoding]::UTF8.GetString($buf, 0, $result.Count)) | Out-Null
    } while (-not $result.EndOfMessage)

    return $sb.ToString()
}

# Events to completely ignore (Source/Type format)
$ignoredEvents = [System.Collections.Generic.HashSet[string]]::new(
    [string[]]@(
        "Inputs/InputMouseClick"
    )
)

# Track users we've already greeted this session
$seenUsers = [System.Collections.Generic.HashSet[string]]::new()

# --- Connect ---
$ws  = [System.Net.WebSockets.ClientWebSocket]::new()
$cts = [System.Threading.CancellationTokenSource]::new()

try {
    Write-Host "Connecting to Streamer.bot at $WsUri ..."
    [void]$ws.ConnectAsync([Uri]::new($WsUri), $cts.Token).GetAwaiter().GetResult()
    Write-Host "Connected."

    # Subscribe to all events
    $sub = @{ request = "Subscribe"; id = "overlay-sub"; events = "*" } | ConvertTo-Json -Compress
    $subBytes = [System.Text.Encoding]::UTF8.GetBytes($sub)
    [void]$ws.SendAsync(
        [System.ArraySegment[byte]]::new($subBytes),
        [System.Net.WebSockets.WebSocketMessageType]::Text,
        $true,
        $cts.Token
    ).GetAwaiter().GetResult()

    # Wait for subscribe acknowledgement
    $ack = Receive-FullMessage $ws $cts.Token | ConvertFrom-Json
    Write-Host "Subscribed - status: $($ack.status)"
    Write-Host "Waiting for events... (Ctrl+C to stop)`n"

    while ($ws.State -eq [System.Net.WebSockets.WebSocketState]::Open) {
        $raw = Receive-FullMessage $ws $cts.Token
        if ($null -eq $raw) { break }

        $msg = $raw | ConvertFrom-Json

        # Only react to actual events (not request responses)
        if ($msg.event) {
            $source = $msg.event.source
            $type   = $msg.event.type
            $eventKey = "$source/$type"

            if ($ignoredEvents.Contains($eventKey)) { continue }

            Write-Host "[$source] $type"
            Send-SpawnBall

            # Greet first-time chatters this session
            if ("$source/$type" -eq "Twitch/ChatMessage") {
                $user = $msg.data.message.displayName
                if ($user -and $seenUsers.Add($user)) {
                    Write-Host "  -> First message from $user, greeting!"
                    Send-BottomPopup "Welcome!" "$user just joined the chat!"
                }
            }

            $alert = Get-AlertFromEvent $msg
            if ($alert) {
                Send-BottomPopup $alert.Title $alert.Body
            }
        }
    }

    Write-Host "Connection closed."
} catch [System.OperationCanceledException] {
    Write-Host "Stopped."
} catch {
    Write-Error "Error: $_"
} finally {
    if ($ws.State -eq [System.Net.WebSockets.WebSocketState]::Open) {
        [void]$ws.CloseAsync(
            [System.Net.WebSockets.WebSocketCloseStatus]::NormalClosure,
            "bye",
            $cts.Token
        ).GetAwaiter().GetResult()
    }
    $ws.Dispose()
    $cts.Dispose()
}
