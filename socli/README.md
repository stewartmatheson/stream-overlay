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
  "jukebox": [
    "music://music.apple.com/au/playlist/lofi-girl-beats-to-relax-study-to/pl.bf7a3cbca49644d8a33f09c1285aef5c",
    "music://music.apple.com/au/playlist/peaceful-piano-music-to-focus-study-to/pl.1347c78be3a44a8f9791f9bc5bebcb98",
    "music://music.apple.com/au/playlist/relaxing-jazz-music/pl.58cf4a62555c4a07a3afc618fe13ca04"
    "music://music.apple.com/au/playlist/sleep-lofi-bedtime-relaxing-music/pl.032a68a5660346758a80057d1ae4b71f",
    "music://music.apple.com/au/playlist/chill-guitar/pl.d9c56209d1e04a7b85ee880322ff0ec7"
    "music://music.apple.com/au/playlist/synthwave-beats-to-chill-game-to/pl.6207ac6ce1cb4c2f843f6a7eac264940"
    "music://music.apple.com/au/playlist/sleep-ambient/pl.3bcc3e01239c4bd189a068f729173aca",
    "music://music.apple.com/au/playlist/relaxing-classical-music/pl.c1ca77f1a0b241b29ac797e6ba7f2274",
    "music://music.apple.com/au/playlist/synth-ambient-chill-music-to-sleep-to/pl.9299cd226bc74f76b75b494c954225ba",
    "music://music.apple.com/au/station/streambeats-by-harris-heller/ra.a-1571523555"
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
      ],
      "jukebox": true
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
      ],
      "jukebox": true
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
