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

Publish a self-contained exe and install it to `~/.local/bin`:

``` bash
dotnet publish -c Release -r win-x64 --self-contained -o ~/.local/bin
```

Then you can run it directly:

``` bash
pomodoro server
cat schedule.json | pomodoro
```

Make sure `~/.local/bin` is on your `PATH`.

## Tech

- .NET 10.0
- C#
