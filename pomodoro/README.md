# 🍅 Pomodoro

A small service that allows for configuration of a Pomodoro schedule.
This service current just sends messages to our stream overlay.

## Models

Time blocks are configurable ranges defined by a start time and an end
time. They will run for firstly a work amount then a rest amount.
Typically this is a 30 minute window that has 25 minutes of work with 5
minutes of rest/reflection on the task just completed.

These blocks have a work title as well as a description associated with
them.

## JSON Configuration

The Pomodoro schedule is configured using a JSON file. Below is an
example:

``` json
{
  "timeBlocks": [
    {
      "startTime": "9:00 AM",
      "endTime": "9:30 AM",
      "workDuration": 25,
      "restDuration": 5,
      "title": "Feature Development",
      "description": "Work on the new authentication flow"
    },
    {
      "startTime": "9:30 AM",
      "endTime": "10:00 AM",
      "workDuration": 25,
      "restDuration": 5,
      "title": "Code Review",
      "description": "Review open pull requests"
    }
  ]
}
```

  Field            Type     Description
  ---------------- -------- --------------------------------------
  `startTime`      string   Start time of the block (h:mm AM/PM)
  `endTime`        string   End time of the block (h:mm AM/PM)
  `workDuration`   number   Minutes of work in the block
  `restDuration`   number   Minutes of rest in the block
  `title`          string   Title of the work session
  `description`    string   Description of the task

## Project Structure

    pomodoro/
    ├── README.md
    └── Pomodoro/
        ├── Pomodoro.csproj        # .NET 9.0 console application project
        ├── Program.cs             # Entry point, wires up events and runs the timer
        ├── PomodoroTimer.cs       # Timer that runs work/rest phases and fires events
        └── PomodoroEvent.cs       # Event interface and console implementation

- **Program.cs** --- Entry point that creates a timer, subscribes event
  handlers, and runs the pomodoro loop with Ctrl+C cancellation support.
- **PomodoroTimer.cs** --- Manages the work and rest phases using
  `Task.Delay`. Exposes `WorkStarted`, `RestStarted`, and
  `BlockCompleted` events.
- **PomodoroEvent.cs** --- Defines the `IPomodoroEvent` interface and a
  `ConsolePomodoroEvent` implementation that logs phase transitions to
  the console.

## Building and Running

Requires the [.NET 9.0 SDK](https://dotnet.microsoft.com/download).

``` bash
cd Pomodoro
dotnet build
dotnet run
```

## Tech

- .NET 9.0
- C#
