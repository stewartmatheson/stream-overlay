namespace Pomodoro;

public class PomodoroTimer
{
  private readonly TimeSpan _workDuration;
  private readonly TimeSpan _restDuration;
  private readonly TimeSpan _intervalDuration;
  private readonly string _title;
  private readonly string _description;
  public event EventHandler<PomodoroEventArgs>? WorkStarted;
  public event EventHandler<PomodoroEventArgs>? RestStarted;
  public event EventHandler<PomodoroEventArgs>? BlockCompleted;
  public event EventHandler<PomodoroEventArgs>? IntervalElapsed;

  public PomodoroTimer(string title, string description, int workMinutes, int restMinutes, int intervalMinutes)
  {
    _title = title;
    _description = description;
    _workDuration = TimeSpan.FromMinutes(workMinutes);
    _restDuration = TimeSpan.FromMinutes(restMinutes);
    _intervalDuration = TimeSpan.FromMinutes(intervalMinutes);
  }

  public void RegisterEventHandler(IPomodoroEventHandler eventHandler)
  {
    WorkStarted += eventHandler.OnWorkStarted;
    RestStarted += eventHandler.OnRestStarted;
    BlockCompleted += eventHandler.OnBlockCompleted;
    IntervalElapsed += eventHandler.OnIntervalElapsed;
  }

  public async Task RunAsync(CancellationToken cancellationToken = default)
  {
    var args = new PomodoroEventArgs(_title, _description, true);

    WorkStarted?.Invoke(this, args);

    var remaining = _workDuration;
    while (remaining > _intervalDuration)
    {
      await Task.Delay(_intervalDuration, cancellationToken);
      remaining -= _intervalDuration;
      IntervalElapsed?.Invoke(this, args);
    }
    await Task.Delay(remaining, cancellationToken);

    var restArgs = new PomodoroEventArgs(_title, _description, false);
    RestStarted?.Invoke(this, restArgs);
    await Task.Delay(_restDuration, cancellationToken);

    BlockCompleted?.Invoke(this, args);
  }
}
