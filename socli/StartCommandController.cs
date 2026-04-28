using System.Diagnostics;
using System.Net.Sockets;
using System.Text;

namespace Socli;

public class StartCommandController
{
  private static string pomAssemblyName = "pom";

  public static bool IsProtocolUri(string app) =>
      Uri.TryCreate(app, UriKind.Absolute, out var uri) && uri.Scheme != "file";

  public List<string> FindMissingApplications(IEnumerable<ApplicationConfig> applications) =>
      applications
          .Where(app => !IsProtocolUri(app.Path) && !File.Exists(app.Path))
          .Select(app => app.Path)
          .ToList();

  public string? FindPomodoroOnPath() =>
      FindInPath(pomAssemblyName);

  public bool IsApplicationRunning(string appPath)
  {
    if (IsProtocolUri(appPath))
      return false;

    var processName = Path.GetFileNameWithoutExtension(appPath);
    return Process.GetProcessesByName(processName).Length > 0;
  }

  public void LaunchApplication(ApplicationConfig app)
  {
    var psi = new ProcessStartInfo
    {
      FileName = app.Path,
      UseShellExecute = true
    };

    if (!IsProtocolUri(app.Path))
      psi.WorkingDirectory = app.WorkingDirectory ?? Path.GetDirectoryName(Path.GetFullPath(app.Path));

    Process.Start(psi);
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

  public async Task<int> CheckPomodoroStatus(string pomodoroExe)
  {
    var psi = new ProcessStartInfo
    {
      FileName = pomodoroExe,
      Arguments = "status",
      UseShellExecute = false
    };

    using var process = Process.Start(psi)
        ?? throw new InvalidOperationException("Failed to start pomodoro status check.");

    await process.WaitForExitAsync();
    return process.ExitCode;
  }

  public async Task SendOverlayLabelAsync(string text)
  {
    using var client = new TcpClient("127.0.0.1", 7777);
    var stream = client.GetStream();
    var labelColor = "e0af68";
    var commandText = $"[b][color={labelColor}]{text}[/color][/b]";
    var bytes = Encoding.UTF8.GetBytes($"label set default|{commandText}|220,47\n");
    await stream.WriteAsync(bytes);
    await stream.FlushAsync();
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
