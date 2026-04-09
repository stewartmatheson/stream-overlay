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

Socli uses the Twitch Helix API to set your stream title, category, and
tags. Credentials are stored in Windows Credential Manager.

### Setup

1.  Go to <https://dev.twitch.tv/console> and register a new
    application.

2.  Set the OAuth redirect URL to `http://localhost:17563`.

3.  Copy the **Client ID** into your `~/.socli.json` under the `twitch`
    section:

    ``` json
    {
      "twitch": {
        "clientId": "your-client-id-here"
      }
    }
    ```

4.  Run `socli twitch-setup` and paste your **Client Secret** when
    prompted. This stores it in Windows Credential Manager under
    `socli:twitch:clientSecret`.

5.  On your first `socli start`, a browser window will open for Twitch
    OAuth authorization. After you approve, the access and refresh
    tokens are saved to Credential Manager automatically. Your
    broadcaster ID is saved to `~/.socli.json`.

### What is stored where

  ---------------------------------------------------------------------
  Value                       Location
  --------------------------- -----------------------------------------
  Client ID                   `~/.socli.json`

  Broadcaster ID              `~/.socli.json`

  Client Secret               Windows Credential Manager

  Access Token                Windows Credential Manager

  Refresh Token               Windows Credential Manager
  ---------------------------------------------------------------------

Tokens are refreshed automatically when they expire.

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
  `name`                      Yes                The name of the
                                                 activity

  `category`                  No                 Twitch stream
                                                 category. Must match a
                                                 Twitch category name
                                                 (searched via the
                                                 Helix API)

  `notification`              No                 TODO: What does this
                                                 do? Twitch
                                                 notification? Discord?
                                                 What happens if empty
                                                 or omitted?

  `tags`                      No                 Tags applied to the
                                                 Twitch stream

  `tasks`                     No                 Pomodoro timer tasks
  ---------------------------------------------------------------------

### Example

``` json
{
  "twitch": {
    "clientId": "abc123"
  },
  "pomodoro": {
    "taskTime": 25,
    "restTime": 5
  },
  "prelaunch": [
    "pom status"
  ],
  "applications": [
    "./path/to/exe/app.exe"
  ],
  "activities": [
    {
      "name": "Programming",
      "category": "Software And Game Development",
      "notification": "",
      "tags": [
        "c++",
        "dotnet",
        "whatever",
        "whatever else"
      ],
      "tasks": [
        "Building the pomodoro JSON with a cli",
        "Making the Pom timer display better messages"
      ],
      "checklist": [
        "Is HDR off?",
        "Make sure obs is running",
        "Are the levels correct in obs"
      ]
    },
    {
      "name": "ESO Gaming",
      "category": "Eso Online",
      "notification": "Watch the noob die.",
      "tags": [
        "noob",
        "noob streamer",
        "backseating allowed"
      ],
      "checklist": [
        "Is HDR on?",
        "Make sure obs is running",
        "Are the levels correct in obs"
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

- [x] Prompt for a stream activity (if programming, ask the user to
  define tasks for the pomodoro timer)
- [x] Set the stream title, tags, and category on Twitch
- [x] Start list of applications
- [x] Start pomodoro timer with provided tasks if not gaming

Before running each step we will validate the following

- All of the paths to the applications exist
- The configuration JSON is valid

### Twitch Setup

    socli twitch-setup

Prompts for your Twitch client secret and stores it in Windows
Credential Manager.

## Installation

Publish a self-contained exe and install it to `~/.local/bin`:

``` bash
dotnet publish .\socli.csproj -c Release -r win-x64 --self-contained -o ../install/socli
```
