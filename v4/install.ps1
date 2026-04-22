$ErrorActionPreference = "Stop"

if (-not $env:STREAM_OVERLAY_HOME) {
    Write-Error "STREAM_OVERLAY_HOME environment variable is not set."
    exit 1
}

pushd $PSScriptRoot

$buildDir = "build\release"
$installDir = "$env:STREAM_OVERLAY_HOME\v4\bin"

cmake -B $buildDir -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build $buildDir --config Release

if (-not (Test-Path $installDir)) {
    New-Item -ItemType Directory -Path $installDir -Force | Out-Null
}

Copy-Item "$buildDir/stream-overlay.exe" -Destination $installDir -Force
Write-Host "Installed stream-overlay.exe to $installDir"

popd
