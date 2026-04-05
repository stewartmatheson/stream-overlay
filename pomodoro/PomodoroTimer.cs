namespace Pomodoro;

public class PomodoroTimer
{
  private readonly TimeSpan _workDuration;
  private readonly TimeSpan _restDuration;
  private readonly string _title;
  private readonly string _description;
  public event EventHandler<PomodoroEventArgs>? WorkStarted;
  public event EventHandler<PomodoroEventArgs>? RestStarted;
  public event EventHandler<PomodoroEventArgs>? BlockCompleted;

  public PomodoroTimer(string title, string description, int workMinutes, int restMinutes)
  {
    _title = title;
    _description = description;
    _workDuration = TimeSpan.FromMinutes(workMinutes);
    _restDuration = TimeSpan.FromMinutes(restMinutes);
  }

  public void RegisterEventHandler(IPomodoroEventHandler eventHandler)
  {
    WorkStarted += eventHandler.OnWorkStarted;
    RestStarted += eventHandler.OnRestStarted;
    BlockCompleted += eventHandler.OnBlockCompleted;
  }

  public async Task RunAsync(CancellationToken cancellationToken = default)
  {
    var args = new PomodoroEventArgs(_title, _description, true);

    WorkStarted?.Invoke(this, args);
    await Task.Delay(_workDuration, cancellationToken);

    var restArgs = new PomodoroEventArgs(_title, _description, false);
    RestStarted?.Invoke(this, restArgs);
    await Task.Delay(_restDuration, cancellationToken);

    BlockCompleted?.Invoke(this, args);
  }
}
