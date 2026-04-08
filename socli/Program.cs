using System.CommandLine;
using Socli;

var startCommand = new Command("start", "Start a stream session");
startCommand.SetAction(async (_, _) => Environment.ExitCode = await StartCommand.RunAsync());

var rootCommand = new RootCommand("CLI tool for managing Twitch stream sessions");
rootCommand.Add(startCommand);

var parseResult = rootCommand.Parse(args);
await parseResult.InvokeAsync();
