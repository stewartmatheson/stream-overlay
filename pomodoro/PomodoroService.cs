namespace Pomodoro;

using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;

public class PomodoroService : BackgroundService
{
  private const int Port = 7778;
  private readonly ILogger<PomodoroService> _logger;
  private readonly PomodoroScheduler _scheduler;
  private readonly OverlayEventHandler _overlayEventHandler;

  public PomodoroService(
    ILogger<PomodoroService> logger,
    PomodoroScheduler scheduler,
    OverlayEventHandler overlayEventHandler)
  {
    _logger = logger;
    _scheduler = scheduler;
    _overlayEventHandler = overlayEventHandler;
  }

  protected override async Task ExecuteAsync(CancellationToken stoppingToken)
  {
    var listener = new TcpListener(IPAddress.Loopback, Port);
    listener.Start();
    _logger.LogInformation("Listening for schedule updates on port {Port}...", Port);

    while (!stoppingToken.IsCancellationRequested)
    {
      TcpClient client;
      try
      {
        client = await listener.AcceptTcpClientAsync(stoppingToken);
      }
      catch (OperationCanceledException)
      {
        break;
      }

      using (client)
      {
        var reader = new StreamReader(client.GetStream(), Encoding.UTF8, leaveOpen: true);
        var message = await reader.ReadToEndAsync(stoppingToken);
        HandleMessage(message, client, stoppingToken);
      }
    }

    _scheduler.Stop();
    _overlayEventHandler.ClearTimer();
    listener.Stop();
    _logger.LogInformation("Shutting down.");
  }

  private void HandleMessage(string message, TcpClient client, CancellationToken token)
  {
    if (message.StartsWith("STATUS"))
      HandleStatus(client);
    else if (message.StartsWith("ADD:"))
      HandleAdd(message.Substring(4).Trim(), token);
    else if (message.StartsWith("RESET"))
      HandleReset();
    else if (message.StartsWith("SCHEDULE:"))
      HandleSchedule(message, token);
    else
      _logger.LogWarning("Unknown command: {Message}", message);
  }

  private void HandleReset()
  {
    _scheduler.Reset();
    _overlayEventHandler.ClearTimer();
    _logger.LogInformation("Reset: all tasks cleared.");
  }

  private void HandleStatus(TcpClient client)
  {
    var status = _scheduler.GetStatus();
    var responseBytes = Encoding.UTF8.GetBytes(status);
    client.GetStream().Write(responseBytes, 0, responseBytes.Length);
  }

  private void HandleAdd(string taskName, CancellationToken token)
  {
    var block = new TimeBlock
    {
      WorkDuration = 25,
      RestDuration = 5,
      Task = new PomodoroTask { Name = taskName, Description = taskName }
    };
    _scheduler.AddBlock(block, token);
    _logger.LogInformation("Added task: {TaskName}", taskName);
  }

  private void HandleSchedule(string message, CancellationToken token)
  {
    var json = message.Substring("SCHEDULE:".Length);

    PomodoroSchedule schedule;
    try
    {
      schedule = JsonSerializer.Deserialize<PomodoroSchedule>(json)
        ?? throw new InvalidOperationException("Failed to deserialize schedule.");
      _logger.LogInformation("Received new schedule with {Count} time block(s).", schedule.TimeBlocks.Count);
    }
    catch (Exception ex)
    {
      _logger.LogError(ex, "Invalid schedule.");
      return;
    }

    _scheduler.ReplaceSchedule(schedule, token);
  }
}
