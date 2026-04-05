namespace Pomodoro;

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

public interface IPomodoroEvent
{
    void OnWorkStarted(object? sender, PomodoroEventArgs e);
    void OnRestStarted(object? sender, PomodoroEventArgs e);
    void OnBlockCompleted(object? sender, PomodoroEventArgs e);
}

public class ConsolePomodoroEvent : IPomodoroEvent
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
}
