namespace Socli;

public static class StartCommand
{
  public static async Task<int> RunAsync()
  {
    var view = new StartCommandView();
    var controller = new StartCommandController();

    var config = SocliConfig.Load();

    if (!ValidatePrerequisites(controller, view, config, out var pomodoroExe))
      return 1;

    var activity = view.PromptForActivity(config.Activities);

    if (activity.Checklist.Count > 0)
      view.RunChecklist(activity.Checklist);

    var tasks = view.GatherTasks(activity);

    view.ShowSessionSummary(config, activity, tasks);

    if (!view.ConfirmStart())
    {
      view.ShowHint("Cancelled.");
      return 0;
    }

    view.ShowWarning("TODO: Twitch API integration (set title, tags, notification)");

    LaunchAllApplications(controller, view, config);

    if (tasks.Count > 0)
      await RunPomodoro(controller, view, pomodoroExe, tasks);

    view.ShowSuccess("Stream session started!");
    return 0;
  }

  private static bool ValidatePrerequisites(
      StartCommandController controller, StartCommandView view,
      SocliConfig config, out string pomodoroExe)
  {
    pomodoroExe = "";

    var missing = controller.FindMissingApplications(config);
    if (missing.Count > 0)
    {
      view.ShowError("The following application paths do not exist:");
      foreach (var app in missing)
        view.ShowError($"  - {app}");
      return false;
    }

    var exe = controller.FindPomodoroOnPath();
    if (exe is null)
    {
      view.ShowError("Could not find 'pomodoro' on your PATH.");
      view.ShowHint("Install it and make sure it's available in your terminal before running socli.");
      return false;
    }

    pomodoroExe = exe;
    return true;
  }

  private static void LaunchAllApplications(
      StartCommandController controller, StartCommandView view, SocliConfig config)
  {
    if (config.Applications.Count == 0) return;

    view.ShowInfo("Launching applications...");
    foreach (var appPath in config.Applications)
    {
      if (controller.IsApplicationRunning(appPath))
      {
        view.ShowWarning($"  Already running: {appPath}");
        continue;
      }

      controller.LaunchApplication(appPath);
      view.ShowSuccess($"  Started: {appPath}");
    }
  }

  private static async Task RunPomodoro(
      StartCommandController controller, StartCommandView view,
      string pomodoroExe, List<string> tasks)
  {
    view.ShowInfo("Resetting pomodoro...");
    await controller.ResetPomodoro(pomodoroExe);

    view.ShowInfo("Adding pomodoro tasks...");
    await controller.StartPomodoro(pomodoroExe, tasks);
  }
}
