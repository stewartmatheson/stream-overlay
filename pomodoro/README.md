# 🍅 Pomodoro

A small service that allows for configuration of a Pomodoro schedule.
This service sends messages to our stream overlay via TCP.

## Usage

The program runs in two modes: **server** and **client**.

### Server mode

Starts a long-running process that listens on TCP port 7778 for schedule
updates. When a new schedule is received, any running schedule is
cancelled and replaced.

``` bash
dotnet run -- server
```

### Client mode

Reads schedule JSON from stdin and sends it to the running server. This
is the default mode (no arguments needed), making it easy to pipe in
configuration from other processes.

``` bash
cat schedule.json | dotnet run
echo '{"timeBlocks":[...]}' | dotnet run
some-other-process | dotnet run
```

## Models

Time blocks are configurable ranges defined by a work duration and a
rest duration. They will run firstly for the work amount then the rest
amount. Typically this is a 30 minute window that has 25 minutes of work
with 5 minutes of rest/reflection on the task just completed.

These blocks have a work title as well as a description associated with
them.

## JSON Configuration

The Pomodoro schedule is configured using a JSON file. Below is an
example:

``` json
{
  "timeBlocks": [
    {
      "workDuration": 25,
      "restDuration": 5,
      "task": {
        "name": "Feature Development",
        "description": "Work on the new authentication flow"
      }
    },
    {
      "workDuration": 25,
      "restDuration": 5,
      "task": {
        "name": "Code Review",
        "description": "Review open pull requests"
      }
    }
  ]
}
```

  Field                Type     Description
  -------------------- -------- ------------------------------
  `workDuration`       number   Minutes of work in the block
  `restDuration`       number   Minutes of rest in the block
  `task.name`          string   Title of the work session
  `task.description`   string   Description of the task

## Project Structure

    pomodoro/
    ├── README.md
    ├── Pomodoro.csproj        # .NET 10.0 console application project
    ├── Program.cs             # Entry point, server/client modes
    ├── PomodoroTimer.cs       # Timer that runs work/rest phases and fires events
    ├── PomodoroSchedule.cs    # Schedule model and JSON deserialization
    └── PomodoroEvent.cs       # Event interfaces and handlers (console + overlay)

## Building and Running

Requires the [.NET 10.0 SDK](https://dotnet.microsoft.com/download).

``` bash
dotnet build
dotnet run -- server
```

## Installation

The `install.ps1` script publishes a self-contained exe, copies it to an
install directory, and registers it as a Windows service
(`PomodoroTimer`) that runs in server mode. Run from an elevated
PowerShell prompt.

``` powershell
# Install to the default location ($env:ProgramFiles\PomodoroTimer)
.\install.ps1 install

# Install to a custom directory
.\install.ps1 install "C:\Tools\Pomodoro"

# Uninstall (removes the service and the install directory)
.\install.ps1 uninstall
```

The service is configured with automatic startup and recovery actions
that restart it after 5s, 10s, then 30s on failure.

Once installed, manage it with the standard Windows tools:

``` powershell
Start-Service PomodoroTimer
Stop-Service PomodoroTimer
Get-Service PomodoroTimer
```

Or use `services.msc` for the GUI.

### Manual publish

If you just want a standalone exe without the service:

``` bash
dotnet publish .\Pomodoro.csproj -c Release -r win-x64 --self-contained -o ~/.local/bin
```

## Commands

### Starting the server

The following in start a server that will not do anything until its been
given tasks.

``` bash
pomodoro server
```

### Task JSON

### reset schedule

The server supports a schedule command which provides full control via
the json above. Just pipe it to the pomodoro command. Note this will
clear all active timers and restart.

``` bash
cat schedule.json | pomodoro schedule
```

### add

At runtime we should be able to add a task to the stack of tasks.
Calling this command will add the provided task to the end of the task
list. It will not change any tasks in progress or reset any timers.

``` bash
pomodoro add "This is the text of the task we want to add"
```

### status

``` bash
pomodoro status
```

View all tasks in the system. This will provde a list in the following
format

    Active Tasks
    ------------
    [4:55] Doing something *
    [25:00] Doing something else
    [25:00] Doing something else again

## Tech

- .NET 10.0
- C#
