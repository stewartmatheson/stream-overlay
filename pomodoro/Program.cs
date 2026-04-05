using System.Text;
using Pomodoro;

Console.OutputEncoding = Encoding.UTF8;

var events = new ConsolePomodoroEvent();
var timer = new PomodoroTimer("Feature Development", "Work on the new authentication flow", 1, 1);

timer.WorkStarted += events.OnWorkStarted;
timer.RestStarted += events.OnRestStarted;
timer.BlockCompleted += events.OnBlockCompleted;

Console.WriteLine("🍅 Pomodoro timer started. Press Ctrl+C to stop.");

using var cts = new CancellationTokenSource();
Console.CancelKeyPress += (_, e) =>
{
    e.Cancel = true;
    cts.Cancel();
};

try
{
    await timer.RunAsync(cts.Token);
}
catch (TaskCanceledException)
{
    Console.WriteLine("Timer cancelled.");
}
