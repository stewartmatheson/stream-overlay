using System.CommandLine;
using Socli;
using Spectre.Console;

var startCommand = new Command("start", "Start a stream session");
startCommand.SetAction(async (_, _) =>
{
  Environment.ExitCode = await StartCommand.RunAsync();
});

var updateTwitchCommand = new Command("update-twitch", "Update Twitch channel title/category/tags only (for testing)");
updateTwitchCommand.SetAction(async (_, _) =>
{
  Environment.ExitCode = await StartCommand.RunUpdateTwitchOnlyAsync();
});

var updateTitleCommand = new Command("update-title", "Update the stream overlay heading and Twitch stream title");
updateTitleCommand.SetAction(async (_, _) =>
{
  Environment.ExitCode = await StartCommand.RunUpdateTitleAsync();
});

var twitchSetupCommand = new Command("twitch-setup", "Store Twitch client secret in Windows Credential Manager");
twitchSetupCommand.SetAction((_, _) =>
{
  var secret = AnsiConsole.Prompt(
      new TextPrompt<string>("Enter your Twitch [green]client secret[/]:")
          .Secret());
  TwitchCredentials.SaveClientSecret(secret);
  AnsiConsole.MarkupLine("[green]Client secret saved to Windows Credential Manager.[/]");
  return Task.CompletedTask;
});

var jukeboxCommand = new Command("jukebox", "Pick a random playlist from the jukebox and open it");
jukeboxCommand.SetAction((_, _) =>
{
  var config = SocliConfig.Load();
  if (config.Jukebox.Count == 0)
  {
    AnsiConsole.MarkupLine("[red]No playlists configured in jukebox.[/]");
    Environment.ExitCode = 1;
    return Task.CompletedTask;
  }
  var controller = new StartCommandController();
  controller.LaunchJukebox(config.Jukebox);
  AnsiConsole.MarkupLine("[green]Jukebox started![/]");
  return Task.CompletedTask;
});

var rootCommand = new RootCommand("CLI tool for managing Twitch stream sessions");
rootCommand.Add(startCommand);
rootCommand.Add(updateTwitchCommand);
rootCommand.Add(updateTitleCommand);
rootCommand.Add(twitchSetupCommand);
rootCommand.Add(jukeboxCommand);

var parseResult = rootCommand.Parse(args);
await parseResult.InvokeAsync();
