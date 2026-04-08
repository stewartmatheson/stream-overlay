# Stream Overlay CLI

A CLI tool for managing Twitch stream sessions, launching required
applications, and configuring stream metadata.

## Prerequisites

- stream-overlay-server --- UI for displaying information on the stream.
- Pomodoro timer --- a service that will run and manage the pom messages
  via the stream-overlay-server

## Installation

We will install as a dotnet tool

## Twitch Authentication

TODO: How to set up Twitch API credentials / OAuth tokens. Where are
they stored? Environment variables? Config file?

## Configuration

Configuration is stored in a JSON file `~/.socli.json`

### Pomodoro fields

  ---------------------------------------------------------------------
  Field                       Required?          Description
  --------------------------- ------------------ ----------------------
  `taskTime`                  TODO               Duration in minutes
                                                 for each work block

  `restTime`                  TODO               Duration in minutes
                                                 for each rest break
  ---------------------------------------------------------------------

### Application fields

  ---------------------------------------------------------------------
  Field                       Required?          Description
  --------------------------- ------------------ ----------------------
  `applications`              TODO               List of executable
                                                 paths to launch when
                                                 starting a stream
                                                 session

  ---------------------------------------------------------------------

### Activity fields

  ---------------------------------------------------------------------
  Field                       Required?          Description
  --------------------------- ------------------ ----------------------
  `name`                      TODO               The name of the
                                                 activity

  `category`                  TODO               Twitch stream
                                                 category. TODO: What
                                                 are valid values? Free
                                                 text or must match
                                                 Twitch categories?

  `notification`              TODO               TODO: What does this
                                                 do? Twitch
                                                 notification? Discord?
                                                 What happens if empty
                                                 or omitted?

  `tags`                      TODO               Tags applied to the
                                                 Twitch stream

  `tasks`                     TODO               Pomodoro timer tasks.
                                                 TODO: Can this be used
                                                 with non-programming
                                                 activities?
  ---------------------------------------------------------------------

### Example

``` json
{
  "pomodoro": {
    "taskTime": 25
    "restTime": 5
  },
  "applications": [
   './path/to/exe/app.exe'
  ],
  "activities": [
    {
      "name": "Programming",
      "category": "Software And Game Development",
      "notification": "",
      "tags": ["c++", "dotnet", "whatever", "whatever else"],
      "tasks": ["Building the pomodoro JSON with a cli", "Making the Pom timer display better messages"]
      "checklist": [
        'Is HDR off?',
        'Make sure obs is running',
        'Are the levels correct in obs',
      ]
    },
    {
      "name": "ESO Gaming",
      "category": "Eso Online",
      "notification": "Watch the noob die.",
      "tags": ["noob", "noob streamer", "backseating allowed"],
      "checklist": [
        'Is HDR on?',
        'Make sure obs is running',
        'Are the levels correct in obs',
      ]
    }
  ]
}
```

## Commands

### Start

    socli start

Starts a single stream session based on a configured activity. This
will:

- [ ] Prompt for a stream activity (if programming, ask the user to
  define tasks for the pomodoro timer)
- [ ] Set the stream title, tags, and notification on Twitch
- [ ] Start list of applications
- [ ] Start pomodoro timer with provided tasks if not gaming

Before running each step we will validate the following

- All of the paths to the applications exist
- The configuration JSON is valid
- Do a health check on twitch might just slow things down
