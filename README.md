# Stream Overlay

A mono-repo of tools for managing a Twitch live-stream. The system is
built around a native Windows overlay that renders on top of OBS, a
pomodoro timer service, and a CLI that ties everything together.

## Architecture

    socli start
      │
      ├─► Launches applications (OBS, Streamer.bot, music, etc.)
      ├─► Sets Twitch stream title, tags & notification
      ├─► Starts the pomodoro timer with tasks
      │
      │        TCP :7778              TCP :7777
      │   socli ──────► pomodoro ──────► stream-overlay (v4)
      │                (schedule)        (timer + popup commands)

All three components communicate over local TCP connections. The CLI
sends a pomodoro schedule to the timer service on port 7778, and the
timer service sends display commands (timers, popups) to the overlay on
port 7777.

## Projects

  ----------------------------------------------------------------------
  Directory                   Language          Description
  --------------------------- ----------------- ------------------------
  **[v4](v4/)**               C++23 / CMake     The stream overlay
                                                window. Renders at
                                                1920x1080 with a fully
                                                transparent background
                                                using Direct2D. Supports
                                                bottom popups, countdown
                                                timers, BBCode
                                                formatting, and Tokyo
                                                Night theming. Listens
                                                for newline-terminated
                                                commands on TCP port
                                                7777.

  **[pomodoro](pomodoro/)**   C# / .NET 10.0    A pomodoro timer
                                                service. Runs as a
                                                server on TCP port 7778
                                                and accepts schedule
                                                JSON defining work/rest
                                                blocks with task names.
                                                Sends timer and popup
                                                commands to the overlay.

  **[socli](socli/)**         C# / .NET         A CLI that orchestrates
                                                a stream session.
                                                Prompts the user to pick
                                                an activity, configures
                                                the Twitch stream
                                                metadata, launches
                                                required applications,
                                                and starts the pomodoro
                                                schedule.

  **install**                                   Installation scripts.
  ----------------------------------------------------------------------

## Configuration

The CLI reads its configuration from `~/.socli.json`. This file defines
pomodoro durations, applications to launch, and a list of stream
activities (each with a Twitch category, tags, tasks, and a pre-stream
checklist).

See the [socli README](socli/README.md) for full configuration details.

## Quick Start

1.  **Build the overlay** (requires CMake 3.20+ and a C++23 compiler):

    ``` bash
    cd v4
    cmake -B build/ninja -G Ninja
    cmake --build build/ninja
    ```

2.  **Build and run the pomodoro server** (requires .NET 10.0 SDK):

    ``` bash
    cd pomodoro
    dotnet run -- server
    ```

3.  **Run the CLI**:

    ``` bash
    socli start
    ```

## TODO

- Rename v4 to something better
- Wrap v4 in a service. Are we going too hard at Windows?
