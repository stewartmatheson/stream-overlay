using System.Text;
using Pomodoro;

Console.OutputEncoding = Encoding.UTF8;

var fileName = "test.json";
var sechedule = PomodoroSchedule.LoadFromFile(fileName);
var consoleEventHandler = new ConsoleEventHandler();
var overlayEventHandler = new OverlayEventHandler();

using var cts = new CancellationTokenSource();
Console.CancelKeyPress += (_, e) =>
{
  e.Cancel = true;
  cts.Cancel();
};

Console.WriteLine(
  "Pomodoro timer started. Press Ctrl+C to stop."
);

foreach (TimeBlock timeBlock in sechedule.TimeBlocks)
{
  var timer = new PomodoroTimer(
    timeBlock.Task.Name,
    timeBlock.Task.Description,
    timeBlock.WorkDuration,
    timeBlock.RestDuration,
    1
  );

  timer.RegisterEventHandler(consoleEventHandler);
  timer.RegisterEventHandler(overlayEventHandler);

  try
  {
    await timer.RunAsync(cts.Token);
  }
  catch (TaskCanceledException)
  {
    Console.WriteLine("Timer cancelled.");
  }
}


