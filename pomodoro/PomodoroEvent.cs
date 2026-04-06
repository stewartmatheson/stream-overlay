namespace Pomodoro;

using System;
using System.Net.Sockets;
using System.Text;

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
    return "🍅 Pomodoro: ";
  }

  public static string WorkStartedTitle(PomodoroEventArgs e)
  {
    return MessagePrefix() + $"[Started Task] {e.Title}";
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
    return $"{e.Description} Task Time Remaining {e.TimeRemaining}";
  }

  public static string RestSummary(PomodoroEventArgs e)
  {
    return $"Chill, relax take it easy... {e.TimeRemaining}";
  }
}

public class ConsoleEventHandler : IPomodoroEventHandler
{
  public void OnWorkStarted(object? sender, PomodoroEventArgs e)
  {
    SendMessage(EventTitleBuilder.WorkStartedTitle(e), EventMessageBodyBuilder.WorkSummary(e));
  }

  public void OnRestStarted(object? sender, PomodoroEventArgs e)
  {
    SendMessage(EventTitleBuilder.RestStartedTitle(e), EventMessageBodyBuilder.RestSummary(e));
  }

  public void OnBlockCompleted(object? sender, PomodoroEventArgs e)
  {
    SendMessage(EventTitleBuilder.BlockCompletedTitle(e), EventMessageBodyBuilder.WorkSummary(e));
  }

  public void OnIntervalElapsed(object? sender, PomodoroEventArgs e)
  {
    SendMessage(EventTitleBuilder.IntervalTitle(e), EventMessageBodyBuilder.WorkSummary(e));
  }

  private static void SendMessage(string title, string body)
  {
    Console.WriteLine(title);
    Console.WriteLine(body);
    Console.WriteLine("");
  }
}

public class OverlayEventHandler : IPomodoroEventHandler
{
  private static void SendMessage(string title, string body)
  {
    int port = 7777;
    string msg = $"bottompopup {title}|{body}";

    using var client = new TcpClient("127.0.0.1", port);
    using var stream = client.GetStream();
    byte[] bytes = Encoding.UTF8.GetBytes(msg + "\n");
    stream.Write(bytes, 0, bytes.Length);
    stream.Flush();
  }

  public void OnWorkStarted(object? sender, PomodoroEventArgs e)
  {
    SendMessage(EventTitleBuilder.WorkStartedTitle(e), EventMessageBodyBuilder.WorkSummary(e));
  }

  public void OnRestStarted(object? sender, PomodoroEventArgs e)
  {
    SendMessage(EventTitleBuilder.RestStartedTitle(e), "");
  }

  public void OnBlockCompleted(object? sender, PomodoroEventArgs e)
  {
    SendMessage(EventTitleBuilder.BlockCompletedTitle(e), EventMessageBodyBuilder.WorkSummary(e));
  }

  public void OnIntervalElapsed(object? sender, PomodoroEventArgs e)
  {
    SendMessage(EventTitleBuilder.IntervalTitle(e), EventMessageBodyBuilder.WorkSummary(e));
  }

}
