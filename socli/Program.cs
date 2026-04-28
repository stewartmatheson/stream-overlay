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

var updateHeadingCommand = new Command("update-heading", "Send a new title to the stream overlay");
updateHeadingCommand.SetAction(async (_, _) =>
{
  var view = new StartCommandView();
  var controller = new StartCommandController();
  var title = view.PromptForOverlayTitle("");
  if (string.IsNullOrWhiteSpace(title))
  {
    view.ShowError("No title provided.");
    Environment.ExitCode = 1;
    return;
  }
  try
  {
    await controller.SendOverlayLabelAsync(title);
    view.ShowSuccess($"Overlay label set: {title}");
  }
  catch (Exception ex)
  {
    view.ShowError($"Could not send overlay label: {ex.Message}");
    Environment.ExitCode = 1;
  }
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
rootCommand.Add(updateHeadingCommand);
rootCommand.Add(twitchSetupCommand);

var parseResult = rootCommand.Parse(args);
await parseResult.InvokeAsync();
