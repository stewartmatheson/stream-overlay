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

#### `bottompopup Title|Body[||BgColor][|TopBorderThickness][|TopBorderColor]`

Displays a popup that slides in from the bottom of the screen. Fields
are pipe-delimited. Colors are optional and default to the Tokyo Night
theme. Display duration scales automatically based on word count
(configurable, default 150 WPM) so longer messages stay on screen
longer. A minimum display time of 3 seconds is enforced.

  Parameter              Type     Description
  ---------------------- -------- -------------------------------------------
  `Title`                string   Popup heading text
  `Body`                 string   Popup body text (supports BBCode)
  `BgColor`              string   Optional background color (`R,G,B` 0-255)
  `TopBorderThickness`   int      Optional top border thickness in pixels
  `TopBorderColor`       string   Optional top border color (hex `#RRGGBB`)

**Helper script** --- `scripts/send-bottompopup.ps1` (PowerShell):

``` powershell
# Defaults — sends "Hello / This is a test bottom popup message"
.\scripts\send-bottompopup.ps1

# Custom title and body
.\scripts\send-bottompopup.ps1 -Title "Welcome!" -Body "Enjoy the stream"

# With background color
.\scripts\send-bottompopup.ps1 -Title "Hey" -Body "Nice to see you" -BgColor "26,27,38"

# With a top border (4px, purple)
.\scripts\send-bottompopup.ps1 -Title "Hey" -Body "Nice to see you" -TopBorderThickness 4 -TopBorderColor "#BB9AF7"

# Custom port
.\scripts\send-bottompopup.ps1 -Title "Hi" -Body "Test" -Port 9999
```

### Mode: Timer

#### `timer Label|MM:SS|X,Y`

Displays a countdown timer at the given pixel position. If a timer is
already running it is replaced. The timer auto-hides when it reaches
zero. Fields are pipe-delimited.

  Parameter   Type     Description
  ----------- -------- ----------------------------
  `Label`     string   Text shown before the time
  `MM:SS`     string   Initial countdown duration
  `X,Y`       string   Screen position in pixels

Example:

``` text
timer Break|05:00|100,50
```

#### `timer_clear`

Hides the current timer immediately.

### BBCode Formatting

The `Body` field supports BBCode tags for inline formatting. Tags can be
nested.

  Tag                               Effect
  --------------------------------- -----------------------
  `[b]text[/b]` B                   old
  `[i]text[/i]` I                   talic
  \`\[color=#RRGGBB\]text\[/color   \]\` Text color (hex)

Example:

``` text
bottompopup Alert|This is [b]bold[/b], [i]italic[/i], and [color=#FF6600]orange[/color]
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

## Install

After building, install the executable to a folder of your choice using
`CMAKE_INSTALL_PREFIX`:

``` bash
cmake --install build/ninja --prefix "C:/your/install/folder"
```

This places the binary at `<prefix>/bin/stream-overlay.exe`. If
`--prefix` is omitted CMake uses its default
(`C:/Program Files/stream-overlay`).

For a Visual Studio Release build:

``` bash
cmake --build build/vs --config Release
cmake --install build/vs --config Release --prefix "C:/your/install/folder"
```

## Future Features

### Sounds

An alarm that plays when the timer is at zero. What does the alarm sound
like? How do we generate the sound? Can we use programming to render a
wave for the alarm sound?

### Window at the Bottom

Start the app so the window is always at the bottom of all other already
open windows. Can we make it stay at the bottom?
