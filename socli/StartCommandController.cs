using System.Diagnostics;
using System.Text.Json;

namespace Socli;

public class StartCommandController
{

  public List<string> FindMissingApplications(SocliConfig config) =>
      config.Applications.Where(app => !File.Exists(app)).ToList();

  public string? FindPomodoroOnPath() =>
      FindInPath("pomodoro");

  public void LaunchApplication(string appPath)
  {
    Process.Start(new ProcessStartInfo
    {
      FileName = appPath,
      UseShellExecute = true
    });
  }

  public bool IsPomodoroRunning()
  {
    var processes = Process.GetProcessesByName("pomodoro");
    return processes.Length > 0;
  }

  public async Task StartPomodoro(string pomodoroExe, PomodoroConfig pom, List<string> tasks)
  {
    var json = BuildScheduleJson(pom, tasks);

    var psi = new ProcessStartInfo
    {
      FileName = pomodoroExe,
      RedirectStandardInput = true,
      UseShellExecute = false
    };

    using var process = Process.Start(psi)
        ?? throw new InvalidOperationException("Failed to start pomodoro process.");

    await process.StandardInput.WriteAsync(json);
    process.StandardInput.Close();
    await process.WaitForExitAsync();
  }

  public static string BuildScheduleJson(PomodoroConfig pom, List<string> tasks)
  {
    var schedule = new
    {
      timeBlocks = tasks.Select(t => new
      {
        workDuration = pom.TaskTime,
        restDuration = pom.RestTime,
        task = new { name = t, description = t }
      }).ToArray()
    };

    return JsonSerializer.Serialize(schedule, new JsonSerializerOptions { WriteIndented = true });
  }

  private static string? FindInPath(string executable)
  {
    var pathVar = Environment.GetEnvironmentVariable("PATH") ?? "";
    var extensions = OperatingSystem.IsWindows()
        ? new[] { ".exe", ".cmd", ".bat" }
        : new[] { "" };

    foreach (var dir in pathVar.Split(Path.PathSeparator))
      foreach (var ext in extensions)
      {
        var fullPath = Path.Combine(dir, executable + ext);
        if (File.Exists(fullPath)) return fullPath;
      }

    return null;
  }
}
