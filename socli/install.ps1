param(
    [Parameter(Position=0)]
    [ValidateSet("install", "uninstall")]
    [string]$Action = "install"
)

if (-not $env:STREAM_OVERLAY_HOME) {
    Write-Error "STREAM_OVERLAY_HOME environment variable is not set."
    exit 1
}

$installPath = "$env:STREAM_OVERLAY_HOME\socli"
$PublishDir = "$PSScriptRoot\bin\Release\net10.0-windows\win-x64\publish"

function Publish-App {
    Write-Output "Publishing application..."
    dotnet publish "$PSScriptRoot" -c Release -r win-x64 --self-contained
    if ($LASTEXITCODE -ne 0) { throw "Publish failed" }
    Write-Output "Published to $PublishDir"
}

function Install-Files {
    Write-Output "Installing to $InstallPath..."
    if (-not (Test-Path $InstallPath)) {
        New-Item -ItemType Directory -Path $InstallPath -Force | Out-Null
    }
    Copy-Item -Path "$PublishDir\*" -Destination $InstallPath -Recurse -Force
    Write-Output "Files copied to $InstallPath"
}

switch ($Action) {
    "install" {
        Publish-App
        Install-Files
        Write-Output "Socli Installed $InstallPath"
    }
    "uninstall" {
        if (Test-Path $InstallPath) {
            Remove-Item -Path $InstallPath -Recurse -Force
            Write-Output "Removed $InstallPath"
        }
        Write-Output "Socli removed."
    }
}
