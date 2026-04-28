namespace Socli;

public static class StartCommand
{
  public static async Task<int> RunAsync()
  {
    var view = new StartCommandView();
    var controller = new StartCommandController();

    var config = SocliConfig.Load();

    if (!ValidatePomodoroOnPath(controller, view, out var pomodoroExe))
      return 1;

    var activity = view.PromptForActivity(config.Activities);

    if (!ValidateApplications(controller, view, config, activity))
      return 1;

    if (activity.UsePomodoro && !await CheckPomodoroStatus(controller, view, pomodoroExe))
      return 1;

    var overlayTitle = view.PromptForOverlayTitle(activity.Title);

    view.ShowSessionSummary(config, activity);

    if (!view.ConfirmStart())
    {
      view.ShowHint("Cancelled.");
      return 0;
    }

    if (!await UpdateTwitch(view, config, activity))
      return 1;

    LaunchAllApplications(controller, view, config.Applications.Concat(activity.Applications));

    await SendOverlayLabel(controller, view, overlayTitle);

    if (activity.Checklist.Count > 0)
      view.RunChecklist(activity.Checklist);

    if (activity.UsePomodoro)
      await RunPomodoro(controller, view, pomodoroExe, view.PromptForFirstTask());

    view.ShowSuccess("Stream session started!");
    return 0;
  }

  public static async Task<int> RunUpdateTwitchOnlyAsync()
  {
    var view = new StartCommandView();
    var config = SocliConfig.Load();

    var activity = view.PromptForActivity(config.Activities);

    return await UpdateTwitch(view, config, activity) ? 0 : 1;
  }

  private static async Task<bool> UpdateTwitch(
      StartCommandView view, SocliConfig config, Activity activity)
  {
    using var twitch = new TwitchService(config);

    if (!twitch.IsConfigured)
    {
      view.ShowError("Twitch not configured — add clientId to ~/.socli.json and run: socli twitch-setup");
      return false;
    }

    if (!twitch.IsAuthenticated)
    {
      view.ShowInfo("Twitch authentication required. Opening browser...");
      if (!await twitch.AuthenticateAsync())
      {
        view.ShowError("Twitch authentication failed.");
        return false;
      }
      view.ShowSuccess("Twitch authenticated!");
    }

    view.ShowInfo("Updating Twitch channel...");

    var categoryResult = await twitch.SearchCategoryAsync(activity.Category);
    if (!categoryResult.Success)
    {
      view.ShowError($"  Failed to search Twitch categories ({categoryResult.StatusCode}).");
      if (!string.IsNullOrWhiteSpace(categoryResult.Error))
        view.ShowError($"  {categoryResult.Error}");
      return false;
    }

    var category = categoryResult.Category
      ?? throw new InvalidOperationException($"Could not find Twitch category: {activity.Category}");

    if (!view.ConfirmCategory(activity.Category, category.Name))
    {
      view.ShowWarning("Cancelled — category not confirmed.");
      return false;
    }

    var result = await twitch.UpdateChannelAsync(activity.Title, category.Id, activity.Tags);
    if (!result.Success)
    {
      view.ShowError($"  Failed to update Twitch channel ({result.StatusCode}).");
      if (!string.IsNullOrWhiteSpace(result.Error))
        view.ShowError($"  {result.Error}");
      return false;
    }

    view.ShowSuccess("  Twitch channel updated!");
    return true;
  }

  private static async Task<bool> CheckPomodoroStatus(
      StartCommandController controller, StartCommandView view, string pomodoroExe)
  {
    view.ShowInfo("Checking pomodoro status...");
    var exitCode = await controller.CheckPomodoroStatus(pomodoroExe);
    if (exitCode != 0)
    {
      view.ShowError($"Pomodoro status check failed (exit code {exitCode}).");
      return false;
    }
    view.ShowSuccess("  Pomodoro status OK");
    return true;
  }

  private static bool ValidatePomodoroOnPath(
      StartCommandController controller, StartCommandView view, out string pomodoroExe)
  {
    pomodoroExe = "";

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

  private static bool ValidateApplications(
      StartCommandController controller, StartCommandView view,
      SocliConfig config, Activity activity)
  {
    var missing = controller.FindMissingApplications(
        config.Applications.Concat(activity.Applications));
    if (missing.Count > 0)
    {
      view.ShowError("The following application paths do not exist:");
      foreach (var app in missing)
        view.ShowError($"  - {app}");
      return false;
    }
    return true;
  }

  private static void LaunchAllApplications(
      StartCommandController controller, StartCommandView view,
      IEnumerable<ApplicationConfig> applications)
  {
    var apps = applications.ToList();
    if (apps.Count == 0) return;

    view.ShowInfo("Launching applications...");
    foreach (var app in apps)
    {
      if (controller.IsApplicationRunning(app.Path))
      {
        view.ShowWarning($"  Already running: {app.Path}");
        continue;
      }

      controller.LaunchApplication(app);
      view.ShowSuccess($"  Started: {app.Path}");
    }
  }

  private static async Task SendOverlayLabel(
      StartCommandController controller, StartCommandView view, string title)
  {
    view.ShowInfo("Sending stream title to overlay...");
    try
    {
      await controller.SendOverlayLabelAsync(title);
      view.ShowSuccess($"  Overlay label set: {title}");
    }
    catch (Exception ex)
    {
      view.ShowWarning($"  Could not send overlay label: {ex.Message}");
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
