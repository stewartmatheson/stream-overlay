using System.CommandLine;
using Socli;

var debugOption = new Option<bool>("--debug");
var startCommand = new Command("start", "Start a stream session") { debugOption };
startCommand.SetAction(async (parseResult, _) =>
{
  var debug = parseResult.GetValue(debugOption);
  Environment.ExitCode = await StartCommand.RunAsync(debug);
});

var rootCommand = new RootCommand("CLI tool for managing Twitch stream sessions");
rootCommand.Add(startCommand);

var parseResult = rootCommand.Parse(args);
await parseResult.InvokeAsync();
