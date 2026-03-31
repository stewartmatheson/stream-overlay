# stream-overlay-v3

> **Work in progress**

The stream overlay supports multiple modes. Modes are selectable and can
be toggled on or off. Each mode lives in its own file and can be
excluded from compilation if not needed. Modes respond to TCP commands.

## Mode: Pomodoro

Sets a 25 minute timer for tasks and insists on a 5 minute break. This
should be visible at all times and should alert when the timer runs out.
It should have the tomato emoji displayed.

## Mode: Physics Test

A physics-based stream overlay that renders bouncing balls on a
1920x1080 transparent window. Balls are spawned via TCP commands from
StreamerBot, letting viewers trigger interactions during a stream.

## Mode: Pop Ups

Intended to drive engagement. A popup that will display when a new user
enters the stream. Will display a message greeting them.

## Graphical Elements

Alerts share a consistent look across modes (Pomodoro and Pop Ups),
styled to match the Tokyo Night theme. Alerts hover at the bottom of the
screen for a set duration then fade out. They are triggered by commands
that include an alert title and body text, displayed title-above-text.

## Tech

- C++23
- SDL2 --- rendering
- Box2D 2.4.1 --- physics simulation
- CMake 3.20+
- Winsock2 --- TCP networking

## How it works

The overlay listens on TCP port `7777` for newline-terminated commands
sent by StreamerBot. Each mode registers its own commands. StreamerBot
integration scripts are in `/scripts/`.

## Commands

Commands are sent as newline-terminated strings to TCP port `7777`.

### Mode: Physics Test

#### `spawn_ball [cx cy vx vy r g b]`

Spawns a ball in the physics simulation. All parameters are optional ---
omitted values are randomized.

  Parameter   Type    Description
  ----------- ------- --------------------------------
  `cx`        float   Center X position (pixels)
  `cy`        float   Center Y position (pixels)
  `vx`        float   Velocity X (pixels/frame)
  `vy`        float   Velocity Y (pixels/frame)
  `r`         uint8   Red color component (0--255)
  `g`         uint8   Green color component (0--255)
  `b`         uint8   Blue color component (0--255)

Examples:

    spawn_ball
    spawn_ball 960 540
    spawn_ball 960 540 1.5 -2.0 255 0 0

### Mode: Pop Ups

#### `popup Title|Body[|BorderColor][|BgColor]`

Displays a greeting popup. Fields are pipe-delimited. Colors are
optional and default to the Tokyo Night theme.

  Parameter       Type     Description
  --------------- -------- ---------------------------
  `Title`         string   Popup heading text
  `Body`          string   Popup body text
  `BorderColor`   string   Optional border color
  `BgColor`       string   Optional background color

**Helper script** --- `scripts/send-popup.ps1` (PowerShell):

``` powershell
# Defaults — sends "Hello / This is a test popup message"
.\scripts\send-popup.ps1

# Custom title and body
.\scripts\send-popup.ps1 -Title "Welcome!" -Body "Enjoy the stream"

# With optional colors
.\scripts\send-popup.ps1 -Title "Hey" -Body "Nice to see you" -BorderColor "#ff0000" -BgColor "#1a1b26"

# Custom port
.\scripts\send-popup.ps1 -Title "Hi" -Body "Test" -Port 9999
```

## Build

Two build configurations are maintained under `build/`:

  ---------------------------------------------------------------------
  Config                           Purpose
  -------------------------------- ------------------------------------
  `build/ninja`                    Fast builds, generates
                                   `compile_commands.json` for tooling

  `build/vs`                       Visual Studio solution for debugging
  ---------------------------------------------------------------------

### Ninja (compile_commands.json)

``` bash
cmake -B build/ninja -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/ninja
```

### Visual Studio

``` bash
cmake -B build/vs -G "Visual Studio 17 2022" -A x64
```

Then open `build/vs/stream-overlay.sln` in Visual Studio.

## TODO

Make sure we static link in libs that don't change like box2d and SDL
