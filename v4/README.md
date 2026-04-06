# stream-overlay-v4

The stream overlay supports multiple modes. Modes are selectable and can
be toggled on or off. Each mode lives in its own file and can be
excluded from compilation if not needed. Modes respond to TCP commands.
The window should run at a size of 1920x1080.

## Mode: BottomPopup

Same as the standard popup with all the same styling and Graphical
treatment however this mode instead of floating over the top right
corner will slide in from the bottom of the screen, display for a
configurable amount of time and then slide back out. With this mode we
should avoid having to mess around with transparent pixels.

## Graphical Elements

Alerts share a consistent look across modes (Pomodoro and Pop Ups),
styled to match the Tokyo Night theme. Alerts hover at the bottom of the
screen for a set duration then fade out. They are triggered by commands
that include an alert title and body text, displayed title-above-text.

## Tech

We should render with true Direct2D so the background of the window is
full transparent

- C++23
- CMake 3.20+
- Winsock2 --- TCP networking

## Commands

Commands are sent as newline-terminated strings to TCP port `7777`.

### Mode: BottomPopup

#### `bottompopup Title|Body[|BorderColor][|BgColor]`

Displays a popup that slides in from the bottom of the screen. Fields
are pipe-delimited. Colors are optional and default to the Tokyo Night
theme.

  Parameter       Type     Description
  --------------- -------- ---------------------------
  `Title`         string   Popup heading text
  `Body`          string   Popup body text
  `BorderColor`   string   Optional border color
  `BgColor`       string   Optional background color

**Helper script** --- `scripts/send-bottompopup.ps1` (PowerShell):

``` powershell
# Defaults — sends "Hello / This is a test bottom popup message"
.\scripts\send-bottompopup.ps1

# Custom title and body
.\scripts\send-bottompopup.ps1 -Title "Welcome!" -Body "Enjoy the stream"

# With optional colors
.\scripts\send-bottompopup.ps1 -Title "Hey" -Body "Nice to see you" -BorderColor "255,0,0" -BgColor "26,27,38"

# Custom port
.\scripts\send-bottompopup.ps1 -Title "Hi" -Body "Test" -Port 9999
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

## Future Features

1.  Constatnt timer display so people can see the pomodoro clock ticking
    down.

2.  Some sort of markup in the message text allowing for setting bold
    and colours in the overlay text. Can we use something like bbcode
    for this?
