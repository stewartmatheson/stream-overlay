namespace Pomodoro;

using System;
using System.Net.Sockets;
using System.Text;
using Microsoft.Extensions.Logging;

public class PomodoroEventArgs : EventArgs
{
  public string Title { get; }
  public string Description { get; }
  public bool IsWorkPhase { get; }
  public TimeSpan TimeRemaining { get; }

  public PomodoroEventArgs(string title, string description, bool isWorkPhase, TimeSpan timeRemaining)
  {
    Title = title;
    Description = description;
    IsWorkPhase = isWorkPhase;
    TimeRemaining = timeRemaining;
  }
}

public interface IPomodoroEventHandler
{
  void OnWorkStarted(object? sender, PomodoroEventArgs e);
  void OnRestStarted(object? sender, PomodoroEventArgs e);
  void OnBlockCompleted(object? sender, PomodoroEventArgs e);
  void OnIntervalElapsed(object? sender, PomodoroEventArgs e);
}

public class EventTitleBuilder
{

  private static string MessagePrefix()
  {
    return "🍅 ";
  }

  public static string WorkStartedTitle(PomodoroEventArgs e)
  {
    return MessagePrefix() + $"Started Task {e.Title}";
  }

  public static string RestStartedTitle(PomodoroEventArgs e)
  {
    return MessagePrefix() + "Rest";
  }

  public static string BlockCompletedTitle(PomodoroEventArgs e)
  {
    return MessagePrefix() + "Task Complete";
  }

  public static string IntervalTitle(PomodoroEventArgs e)
  {
    return MessagePrefix() + "Task In Progres";
  }
}

public class EventMessageBodyBuilder
{
  public static string WorkSummary(PomodoroEventArgs e)
  {
    return $"{TimeRemaining(e)} {e.Description}";
  }

  public static string RestSummary(PomodoroEventArgs e)
  {
    return $"{TimeRemaining(e)} Chill, relax, take it easy ";
  }

  private static string TimeRemaining(PomodoroEventArgs e)
  {
    var hexColor = "8f5815";
    return $"[[color={hexColor}][b]⏰ {e.TimeRemaining}[/b][/color]]";
  }
}

public class LoggingEventHandler : IPomodoroEventHandler
{
  private readonly ILogger<LoggingEventHandler> _logger;

  public LoggingEventHandler(ILogger<LoggingEventHandler> logger)
  {
    _logger = logger;
  }

  public void OnWorkStarted(object? sender, PomodoroEventArgs e)
  {
    _logger.LogInformation("{Title} - {Body}", EventTitleBuilder.WorkStartedTitle(e), EventMessageBodyBuilder.WorkSummary(e));
  }

  public void OnRestStarted(object? sender, PomodoroEventArgs e)
  {
    _logger.LogInformation("{Title} - {Body}", EventTitleBuilder.RestStartedTitle(e), EventMessageBodyBuilder.RestSummary(e));
  }

  public void OnBlockCompleted(object? sender, PomodoroEventArgs e)
  {
    _logger.LogInformation("{Title} - {Body}", EventTitleBuilder.BlockCompletedTitle(e), EventMessageBodyBuilder.WorkSummary(e));
  }

  public void OnIntervalElapsed(object? sender, PomodoroEventArgs e)
  {
    _logger.LogInformation("{Title} - {Body}", EventTitleBuilder.IntervalTitle(e), EventMessageBodyBuilder.WorkSummary(e));
  }
}

public record OverlayPosition(int X, int Y);

public static class OverlayPositions
{
  public static readonly OverlayPosition DesktopBottomRight = new(1459, 1046);
  public static readonly OverlayPosition TasksListTitleRight = new(1767, 480);
}

public class OverlayEventHandler : IPomodoroEventHandler
{
  private const int Port = 7777;
  private const string Host = "127.0.0.1";

  private static void SendTcp(string msg)
  {
    using var client = new TcpClient(Host, Port);
    using var stream = client.GetStream();
    byte[] bytes = Encoding.UTF8.GetBytes(msg + "\n");
    stream.Write(bytes, 0, bytes.Length);
    stream.Flush();
  }

  private static void SendBottomPopupMessage(string title, string body)
  {
    SendTcp($"bottompopup {title}|{body}");
  }

  private static void SendTimerCommand(string label, TimeSpan remaining, OverlayPosition position)
  {
    int minutes = (int)remaining.TotalMinutes;
    int seconds = remaining.Seconds;
    SendTcp($"timer {label}|{minutes:D2}:{seconds:D2}|{position.X},{position.Y}");
  }

  public void OnWorkStarted(object? sender, PomodoroEventArgs e)
  {
    SendBottomPopupMessage(EventTitleBuilder.WorkStartedTitle(e), EventMessageBodyBuilder.WorkSummary(e));
    SendTimerCommand("🍅", e.TimeRemaining, OverlayPositions.TasksListTitleRight);
  }

  public void OnRestStarted(object? sender, PomodoroEventArgs e)
  {
    SendBottomPopupMessage(EventTitleBuilder.RestStartedTitle(e), EventMessageBodyBuilder.RestSummary(e));
    SendTimerCommand("🍅", e.TimeRemaining, OverlayPositions.TasksListTitleRight);
  }

  public void OnBlockCompleted(object? sender, PomodoroEventArgs e)
  {
    SendBottomPopupMessage(EventTitleBuilder.BlockCompletedTitle(e), EventMessageBodyBuilder.WorkSummary(e));
  }

  public void OnIntervalElapsed(object? sender, PomodoroEventArgs e)
  {
    SendBottomPopupMessage(EventTitleBuilder.IntervalTitle(e), EventMessageBodyBuilder.WorkSummary(e));
  }

  public void ClearTimer()
  {
    SendTcp("timer_clear");
  }

  public void SendListUpdate(List<TimeBlock> blocks, int currentIndex, bool isResting)
  {
    var items = new List<string>();

    var activeTaskHexColor = "73daca";
    var restColor = "28344a";

    for (int i = currentIndex; i < blocks.Count; i++)
    {
      var name = blocks[i].Task.Name;
      if (i == currentIndex)
      {
        if (isResting)
        {
          items.Add($"💤 [color={restColor}]rest[/color]");
        }
        else
        {
          items.Add($"🗒️ [color={activeTaskHexColor}][b]{name}[/b][/color]");
        }
      }
      else
        items.Add($"🗒️ {name}");

      items.Add($"💤 [color={restColor}]rest[/color]");
    }

    var content = string.Join("\\n", items);
    var title = isResting ? "resting" : "tasks";
    SendTcp($"list set {content}|285|1598,475|{title}");
  }

  public void ClearList()
  {
    SendTcp("list clear");
  }
}
