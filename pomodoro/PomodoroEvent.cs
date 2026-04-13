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

  private static void SendTimerCommand(string label, TimeSpan remaining, int x = 1459, int y = 1046)
  {
    int minutes = (int)remaining.TotalMinutes;
    int seconds = remaining.Seconds;
    SendTcp($"timer {label}|{minutes:D2}:{seconds:D2}|{x},{y}");
  }

  public void OnWorkStarted(object? sender, PomodoroEventArgs e)
  {
    SendBottomPopupMessage(EventTitleBuilder.WorkStartedTitle(e), EventMessageBodyBuilder.WorkSummary(e));
    SendTimerCommand("🍅", e.TimeRemaining);
  }

  public void OnRestStarted(object? sender, PomodoroEventArgs e)
  {
    SendBottomPopupMessage(EventTitleBuilder.RestStartedTitle(e), EventMessageBodyBuilder.RestSummary(e));
    SendTimerCommand("🍅", e.TimeRemaining);
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
}
