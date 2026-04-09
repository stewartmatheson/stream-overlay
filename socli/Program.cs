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

var rootCommand = new RootCommand("CLI tool for managing Twitch stream sessions");
rootCommand.Add(startCommand);
rootCommand.Add(updateTwitchCommand);
rootCommand.Add(twitchSetupCommand);

var parseResult = rootCommand.Parse(args);
await parseResult.InvokeAsync();
