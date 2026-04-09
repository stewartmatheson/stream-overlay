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
else if (args.Length >= 2 && args[0] == "add")
{
  var taskName = string.Join(' ', args.Skip(1));
  await SendCommand($"ADD:{taskName}");
}
else if (args.Length > 0 && args[0] == "status")
{
  var response = await SendCommandWithResponse("STATUS");
  Console.Write(response);
}
else if (args.Length > 0 && args[0] == "reset")
{
  await SendCommand("RESET");
}
else
{
  await RunClient();
}

async Task RunClient()
{
  var json = await Console.In.ReadToEndAsync();
  await SendCommand($"SCHEDULE:{json}");
}

async Task SendCommand(string message)
{
  using var client = new TcpClient("127.0.0.1", Port);
  using var stream = client.GetStream();
  var bytes = Encoding.UTF8.GetBytes(message);
  stream.Write(bytes, 0, bytes.Length);
  client.Client.Shutdown(SocketShutdown.Send);
}

async Task<string> SendCommandWithResponse(string message)
{
  using var client = new TcpClient("127.0.0.1", Port);
  using var stream = client.GetStream();
  var bytes = Encoding.UTF8.GetBytes(message);
  stream.Write(bytes, 0, bytes.Length);
  client.Client.Shutdown(SocketShutdown.Send);

  using var reader = new StreamReader(stream, Encoding.UTF8);
  return await reader.ReadToEndAsync();
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

  var scheduler = new PomodoroScheduler(consoleEventHandler, overlayEventHandler);

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

    using (client)
    {
      var reader = new StreamReader(client.GetStream(), Encoding.UTF8, leaveOpen: true);
      var message = await reader.ReadToEndAsync(appCts.Token);

      HandleMessage(message, client, scheduler, overlayEventHandler, appCts.Token);
    }
  }

  scheduler.Stop();
  overlayEventHandler.ClearTimer();
  listener.Stop();
  Console.WriteLine("Shutting down.");
}

void HandleMessage(string message, TcpClient client, PomodoroScheduler scheduler, OverlayEventHandler overlayEventHandler, CancellationToken token)
{
  if (message.StartsWith("STATUS"))
    HandleStatus(client, scheduler);
  else if (message.StartsWith("ADD:"))
    HandleAdd(message.Substring(4).Trim(), scheduler, token);
  else if (message.StartsWith("RESET"))
    HandleReset(scheduler, overlayEventHandler);
  else if (message.StartsWith("SCHEDULE:"))
    HandleSchedule(message, scheduler, token);
  else
    throw new Exception("Unknown Command");
}

void HandleReset(PomodoroScheduler scheduler, OverlayEventHandler overlayEventHandler)
{
  scheduler.Reset();
  overlayEventHandler.ClearTimer();
  Console.WriteLine("Reset: all tasks cleared.");
}

void HandleStatus(TcpClient client, PomodoroScheduler scheduler)
{
  var status = scheduler.GetStatus();
  var responseBytes = Encoding.UTF8.GetBytes(status);
  client.GetStream().Write(responseBytes, 0, responseBytes.Length);
}

void HandleAdd(string taskName, PomodoroScheduler scheduler, CancellationToken token)
{
  var block = new TimeBlock
  {
    WorkDuration = 25,
    RestDuration = 5,
    Task = new PomodoroTask { Name = taskName, Description = taskName }
  };
  scheduler.AddBlock(block, token);
  Console.WriteLine($"Added task: {taskName}");
}

void HandleSchedule(string message, PomodoroScheduler scheduler, CancellationToken token)
{
  var json = message.Substring("SCHEDULE:".Length);

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
    return;
  }

  scheduler.ReplaceSchedule(schedule, token);
}
