using System.Diagnostics;

namespace Socli;

public class StartCommandController
{
  private static string pomAssemblyName = "pom";

  public static bool IsProtocolUri(string app) =>
      Uri.TryCreate(app, UriKind.Absolute, out var uri) && uri.Scheme != "file";

  public List<string> FindMissingApplications(SocliConfig config) =>
      config.Applications.Where(app => !IsProtocolUri(app) && !File.Exists(app)).ToList();

  public string? FindPomodoroOnPath() =>
      FindInPath(pomAssemblyName);

  public bool IsApplicationRunning(string appPath)
  {
    if (IsProtocolUri(appPath))
      return false;

    var processName = Path.GetFileNameWithoutExtension(appPath);
    return Process.GetProcessesByName(processName).Length > 0;
  }

  public void LaunchApplication(string appPath)
  {
    Process.Start(new ProcessStartInfo
    {
      FileName = appPath,
      UseShellExecute = true
    });
  }

  public async Task ResetPomodoro(string pomodoroExe)
  {
    var psi = new ProcessStartInfo
    {
      FileName = pomodoroExe,
      Arguments = "reset",
      UseShellExecute = false
    };

    using var process = Process.Start(psi)
        ?? throw new InvalidOperationException("Failed to start pomodoro process.");

    await process.WaitForExitAsync();
  }

  public async Task StartPomodoro(string pomodoroExe, List<string> tasks)
  {
    foreach (var task in tasks)
    {
      var psi = new ProcessStartInfo
      {
        FileName = pomodoroExe,
        Arguments = $"add \"{task}\"",
        UseShellExecute = false
      };

      using var process = Process.Start(psi)
          ?? throw new InvalidOperationException("Failed to start pomodoro process.");

      await process.WaitForExitAsync();
    }
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
