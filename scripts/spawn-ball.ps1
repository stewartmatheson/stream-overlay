param(
    [int]   $X  = -1,
    [int]   $Y  = -1,
    [float] $Vx = [float]::NaN,
    [float] $Vy = [float]::NaN,
    [int]   $R  = -1,
    [int]   $G  = -1,
    [int]   $B  = -1,
    [int]   $Port = 7777
)

# Build command — only include args if all positional groups are supplied,
# otherwise let the overlay randomise the omitted ones.
# The protocol requires args in order: cx cy vx vy r g b
# If any value in a group is missing we stop there and let the server fill the rest.

$cx = if ($X  -ge 0)                        { $X  } else { Get-Random -Minimum 0 -Maximum 1920 }
$cy = if ($Y  -ge 0)                        { $Y  } else { Get-Random -Minimum 0 -Maximum 1080 }
$vx = if (-not [float]::IsNaN($Vx))         { $Vx } else { [float](Get-Random -Minimum -300 -Maximum 300) / 100.0 }
$vy = if (-not [float]::IsNaN($Vy))         { $Vy } else { [float](Get-Random -Minimum -300 -Maximum 300) / 100.0 }
$r  = if ($R -ge 0 -and $R -le 255)         { $R  } else { Get-Random -Minimum 0 -Maximum 256 }
$g  = if ($G -ge 0 -and $G -le 255)         { $G  } else { Get-Random -Minimum 0 -Maximum 256 }
$b  = if ($B -ge 0 -and $B -le 255)         { $B  } else { Get-Random -Minimum 0 -Maximum 256 }

$cmd = "spawn_ball $cx $cy $vx $vy $r $g $b`n"

try {
    $tcp    = [System.Net.Sockets.TcpClient]::new("127.0.0.1", $Port)
    $stream = $tcp.GetStream()
    $bytes  = [System.Text.Encoding]::ASCII.GetBytes($cmd)
    $stream.Write($bytes, 0, $bytes.Length)
    $stream.Flush()
    $tcp.Close()
    Write-Host "Spawned ball: $cmd".Trim()
} catch {
    Write-Error "Could not connect to overlay on port $Port - is it running?"
}
