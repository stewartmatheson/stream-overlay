namespace Pomodoro;

using System;
using System.Net.Sockets;
using System.Text;

public class PomodoroEventArgs : EventArgs
{
  public string Title { get; }
  public string Description { get; }
  public bool IsWorkPhase { get; }

  public PomodoroEventArgs(string title, string description, bool isWorkPhase)
  {
    Title = title;
    Description = description;
    IsWorkPhase = isWorkPhase;
  }
}

public interface IPomodoroEventHandler
{
  void OnWorkStarted(object? sender, PomodoroEventArgs e);
  void OnRestStarted(object? sender, PomodoroEventArgs e);
  void OnBlockCompleted(object? sender, PomodoroEventArgs e);
  void OnIntervalElapsed(object? sender, PomodoroEventArgs e);
}

public class ConsoleEventHandler : IPomodoroEventHandler
{
  public void OnWorkStarted(object? sender, PomodoroEventArgs e)
  {
    Console.WriteLine($"[WORK] {e.Title}: {e.Description}");
  }

  public void OnRestStarted(object? sender, PomodoroEventArgs e)
  {
    Console.WriteLine($"[REST] {e.Title}: Time to rest!");
  }

  public void OnBlockCompleted(object? sender, PomodoroEventArgs e)
  {
    Console.WriteLine($"[DONE] {e.Title}: Block completed.");
  }

  public void OnIntervalElapsed(object? sender, PomodoroEventArgs e)
  {
    Console.WriteLine($"[INTERVAL] {e.Title}: Interval elapsed.");
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
    SendMessage($"🍅 Start Task {e.Title}", e.Description);
  }

  public void OnRestStarted(object? sender, PomodoroEventArgs e)
  {
    SendMessage($"🍅 Break", e.Description);
  }

  public void OnBlockCompleted(object? sender, PomodoroEventArgs e)
  {
    SendMessage($"🍅 Block Completed", e.Description);
  }

  public void OnIntervalElapsed(object? sender, PomodoroEventArgs e)
  {
    SendMessage($"🍅 Interval", e.Description);
  }
}
