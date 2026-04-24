namespace Pomodoro;

using System.Text;
using Microsoft.Extensions.Logging;

public class PomodoroScheduler
{
  private readonly IPomodoroEventHandler _loggingHandler;
  private readonly OverlayEventHandler _overlayHandler;
  private readonly ILogger<PomodoroScheduler> _logger;
  private readonly List<TimeBlock> _blocks = new();
  private readonly object _lock = new();
  private CancellationTokenSource? _scheduleCts;
  private int _currentIndex = -1;
  private bool _isWorkPhase = true;
  private DateTime _phaseStartedAt;
  private TimeSpan _phaseDuration;

  public PomodoroScheduler(IPomodoroEventHandler loggingHandler, OverlayEventHandler overlayHandler, ILogger<PomodoroScheduler> logger)
  {
    _loggingHandler = loggingHandler;
    _overlayHandler = overlayHandler;
    _logger = logger;
  }

  public void ReplaceSchedule(PomodoroSchedule schedule, CancellationToken appToken)
  {
    Stop();

    lock (_lock)
    {
      _blocks.Clear();
      _blocks.AddRange(schedule.TimeBlocks);
      _currentIndex = -1;
    }

    StartRunner(0, appToken);
  }

  public void AddBlock(TimeBlock block, CancellationToken appToken)
  {
    lock (_lock)
    {
      _blocks.Add(block);

      // If nothing is running, start from this new block
      if (_currentIndex == -1 || _scheduleCts == null || _scheduleCts.IsCancellationRequested)
      {
        var startIndex = _blocks.Count - 1;
        StartRunner(startIndex, appToken);
        return;
      }

      _overlayHandler.SendListUpdate(_blocks, _currentIndex, !_isWorkPhase);
    }
  }

  public void Stop()
  {
    if (_scheduleCts != null)
    {
      _scheduleCts.Cancel();
      _scheduleCts.Dispose();
      _scheduleCts = null;
      _logger.LogInformation("Cancelled previous schedule.");
    }
  }

  public void Reset()
  {
    Stop();
    lock (_lock)
    {
      _blocks.Clear();
      _currentIndex = -1;
    }
    _overlayHandler.ClearList();
  }

  public string GetStatus()
  {
    lock (_lock)
    {
      if (_blocks.Count == 0)
      {
        return "No tasks.\n";
      }

      var sb = new StringBuilder();
      sb.AppendLine("Active Tasks");
      sb.AppendLine("------------");

      for (int i = 0; i < _blocks.Count; i++)
      {
        var block = _blocks[i];

        if (i < _currentIndex)
          continue;

        if (i == _currentIndex)
        {
          var elapsed = DateTime.UtcNow - _phaseStartedAt;
          var remaining = _phaseDuration - elapsed;
          if (remaining < TimeSpan.Zero) remaining = TimeSpan.Zero;

          var restTime = _isWorkPhase
            ? FormatTime(TimeSpan.FromMinutes(block.RestDuration))
            : FormatTime(remaining);
          var workTime = _isWorkPhase
            ? FormatTime(remaining)
            : "done";
          sb.AppendLine($"\u001b[32m[{workTime} work, {restTime} rest] {block.Task.Name} *\u001b[0m");
        }
        else
        {
          sb.AppendLine($"[{FormatTime(TimeSpan.FromMinutes(block.WorkDuration))} work, " +
            $"{FormatTime(TimeSpan.FromMinutes(block.RestDuration))} rest] {block.Task.Name}");
        }
      }

      return sb.ToString();
    }
  }

  private static string FormatTime(TimeSpan t) => $"{(int)t.TotalMinutes}:{t.Seconds:D2}";

  private void StartRunner(int startIndex, CancellationToken appToken)
  {
    _scheduleCts = CancellationTokenSource.CreateLinkedTokenSource(appToken);
    var token = _scheduleCts.Token;

    _ = Task.Run(async () =>
    {
      int i = startIndex;
      while (true)
      {
        TimeBlock block;
        lock (_lock)
        {
          if (i >= _blocks.Count) break;
          block = _blocks[i];
          _currentIndex = i;
          _overlayHandler.SendListUpdate(_blocks, i, false);
        }

        var timer = new PomodoroTimer(
          block.Task.Name,
          block.Task.Description,
          block.WorkDuration,
          block.RestDuration,
          1
        );

        timer.RegisterEventHandler(_loggingHandler);
        timer.RegisterEventHandler(_overlayHandler);
        timer.WorkStarted += (_, e) =>
        {
          lock (_lock)
          {
            _isWorkPhase = true;
            _phaseStartedAt = DateTime.UtcNow;
            _phaseDuration = e.TimeRemaining;
          }
        };
        timer.RestStarted += (_, e) =>
        {
          lock (_lock)
          {
            _isWorkPhase = false;
            _phaseStartedAt = DateTime.UtcNow;
            _phaseDuration = e.TimeRemaining;
            _overlayHandler.SendListUpdate(_blocks, _currentIndex, true);
          }
        };

        try
        {
          await timer.RunAsync(token);
        }
        catch (OperationCanceledException)
        {
          _logger.LogInformation("Schedule cancelled.");
          return;
        }

        i++;
      }

      lock (_lock)
      {
        _currentIndex = -1;
      }

      _overlayHandler.ClearList();
      _logger.LogInformation("Schedule complete.");
    }, token);
  }
}
