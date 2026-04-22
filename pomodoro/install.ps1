#Requires -RunAsAdministrator

param(
    [Parameter(Position=0)]
    [ValidateSet("install", "uninstall")]
    [string]$Action = "install"
)

if (-not $env:STREAM_OVERLAY_HOME) {
    Write-Error "STREAM_OVERLAY_HOME environment variable is not set."
    exit 1
}

$installPath = "$env:STREAM_OVERLAY_HOME\pomodoro"

$ErrorActionPreference = "Stop"
$ServiceName = "PomodoroTimer"
$DisplayName = "Pomodoro Timer Service"
$Description = "Pomodoro timer server for stream overlay"

$PublishDir = "$PSScriptRoot\bin\Release\net10.0\win-x64\publish"
$ExePath = "$InstallPath\pom.exe"

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

        if (Get-Service -Name $ServiceName -ErrorAction SilentlyContinue) {
            Write-Output "Service '$ServiceName' already exists. Stopping and removing..."
            sc.exe stop $ServiceName 2>$null
            sc.exe delete $ServiceName

            $timeout = (Get-Date).AddSeconds(10)
            while (Get-Service -Name $ServiceName -ErrorAction SilentlyContinue) {
                if ((Get-Date) -gt $timeout) {
                    throw "Service '$ServiceName' did not finish deleting within 10s. Close Services.msc or anything holding a handle and retry."
                }
                Start-Sleep -Milliseconds 200
            }
        }

        sc.exe create $ServiceName binPath= "`"$ExePath`" server" start= auto displayname= "$DisplayName"
        sc.exe description $ServiceName "$Description"
        sc.exe failure $ServiceName reset= 86400 actions= restart/5000/restart/10000/restart/30000

        if ($LASTEXITCODE -eq 0) {
            Write-Output "Service '$ServiceName' installed successfully."
            Write-Output "Start it with: Start-Service $ServiceName"
        } else {
            throw "Failed to create service"
        }
    }
    "uninstall" {
        sc.exe stop $ServiceName 2>$null
        sc.exe delete $ServiceName
        if (Test-Path $InstallPath) {
            Remove-Item -Path $InstallPath -Recurse -Force
            Write-Output "Removed $InstallPath"
        }
        Write-Output "Service '$ServiceName' removed."
    }
}
