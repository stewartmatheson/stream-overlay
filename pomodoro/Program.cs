using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using Pomodoro;

const int Port = 7778;

if (args.Length > 0 && args[0] == "server")
{
  await RunServer();
}
else
{
  await RunClient();
}

async Task RunClient()
{
  var json = await Console.In.ReadToEndAsync();

  using var client = new TcpClient("127.0.0.1", Port);
  using var stream = client.GetStream();
  var bytes = Encoding.UTF8.GetBytes(json);
  stream.Write(bytes, 0, bytes.Length);
}

async Task RunServer()
{
  Console.OutputEncoding = Encoding.UTF8;

  var consoleEventHandler = new ConsoleEventHandler();
  var overlayEventHandler = new OverlayEventHandler();

  var listener = new TcpListener(IPAddress.Loopback, Port);
  listener.Start();
  Console.WriteLine($"Listening for schedule updates on port {Port}...");

  using var appCts = new CancellationTokenSource();
  Console.CancelKeyPress += (_, e) =>
  {
    e.Cancel = true;
    appCts.Cancel();
  };

  CancellationTokenSource? scheduleCts = null;

  while (!appCts.IsCancellationRequested)
  {
    TcpClient client;
    try
    {
      client = await listener.AcceptTcpClientAsync(appCts.Token);
    }
    catch (OperationCanceledException)
    {
      break;
    }

    string json;
    using (client)
    using (var reader = new StreamReader(client.GetStream(), Encoding.UTF8))
    {
      json = await reader.ReadToEndAsync(appCts.Token);
    }

    PomodoroSchedule schedule;
    try
    {
      schedule = JsonSerializer.Deserialize<PomodoroSchedule>(json)
        ?? throw new InvalidOperationException("Failed to deserialize schedule.");
      Console.WriteLine($"Received new schedule with {schedule.TimeBlocks.Count} time block(s).");
    }
    catch (Exception ex)
    {
      Console.WriteLine($"Invalid schedule: {ex.Message}");
      continue;
    }

    // Cancel any running schedule
    if (scheduleCts != null)
    {
      await scheduleCts.CancelAsync();
      scheduleCts.Dispose();
      Console.WriteLine("Cancelled previous schedule.");
    }

    scheduleCts = CancellationTokenSource.CreateLinkedTokenSource(appCts.Token);
    var token = scheduleCts.Token;

    // Run the schedule in the background so we can keep listening
    _ = Task.Run(async () =>
    {
      foreach (var timeBlock in schedule.TimeBlocks)
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
          await timer.RunAsync(token);
        }
        catch (OperationCanceledException)
        {
          Console.WriteLine("Schedule cancelled.");
          return;
        }
      }

      Console.WriteLine("Schedule complete.");
    }, token);
  }

  scheduleCts?.Dispose();
  listener.Stop();
  Console.WriteLine("Shutting down.");
}
