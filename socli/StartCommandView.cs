using Spectre.Console;

namespace Socli;

public class StartCommandView
{
    public void ShowError(string message) =>
        AnsiConsole.MarkupLine($"[red]{message}[/]");

    public void ShowHint(string message) =>
        AnsiConsole.MarkupLine($"[grey]{message}[/]");

    public void ShowInfo(string message) =>
        AnsiConsole.MarkupLine($"[blue]{message}[/]");

    public void ShowSuccess(string message) =>
        AnsiConsole.MarkupLine($"[green]{message}[/]");

    public void ShowWarning(string message) =>
        AnsiConsole.MarkupLine($"[yellow]{message}[/]");

    public Activity PromptForActivity(List<Activity> activities)
    {
        var activity = AnsiConsole.Prompt(
            new SelectionPrompt<Activity>()
                .Title("Select a [green]stream activity[/]:")
                .UseConverter(a => a.Title)
                .AddChoices(activities)
        );
        ShowSuccess($"Selected: {activity.Title}");
        return activity;
    }

    public List<string> GatherTasks(Activity activity)
    {
        var tasks = new List<string>(activity.Tasks);

        if (tasks.Count == 0)
            return tasks;

        ShowExistingTasks(tasks);
        if (AnsiConsole.Confirm("Add more tasks?", defaultValue: false))
            tasks.AddRange(PromptForNewTasks());

        return tasks;
    }

    public bool ConfirmStart() =>
        AnsiConsole.Confirm("Start stream session?");

    public bool ConfirmCategory(string searched, string found)
    {
        if (searched.Equals(found, StringComparison.OrdinalIgnoreCase))
        {
            ShowSuccess($"  Category matched: {found}");
            return true;
        }

        ShowWarning($"  Searched for \"{searched}\" but closest match is \"{found}\"");
        return AnsiConsole.Confirm($"  Use \"{found}\" as the category?");
    }

    public void ShowSessionSummary(SocliConfig config, Activity activity, List<string> tasks)
    {
        AnsiConsole.Write(new Rule("[yellow]Stream Session[/]"));
        AnsiConsole.MarkupLine($"[bold]Title:[/] {activity.Title}");
        AnsiConsole.MarkupLine($"[bold]Category:[/] {activity.Category}");

        if (!string.IsNullOrWhiteSpace(activity.Notification))
            AnsiConsole.MarkupLine($"[bold]Notification:[/] {activity.Notification}");
        if (activity.Tags.Count > 0)
            AnsiConsole.MarkupLine($"[bold]Tags:[/] {string.Join(", ", activity.Tags)}");
        if (tasks.Count > 0)
        {
            AnsiConsole.MarkupLine($"[bold]Pomodoro:[/] {config.Pomodoro.TaskTime}m work / {config.Pomodoro.RestTime}m rest");
            AnsiConsole.MarkupLine($"[bold]Tasks:[/] {tasks.Count}");
        }

        AnsiConsole.Write(new Rule());
    }

    public void RunChecklist(List<string> checklist)
    {
        AnsiConsole.Write(new Rule("[yellow]Pre-stream Checklist[/]"));
        for (var i = 0; i < checklist.Count; i++)
        {
            AnsiConsole.Prompt(
                new TextPrompt<string>($"  [bold][[{i + 1}/{checklist.Count}]][/] {checklist[i]} [grey](press Enter to confirm)[/]")
                    .AllowEmpty()
            );
            AnsiConsole.MarkupLine($"  [green]✓[/] {checklist[i]}");
        }
        AnsiConsole.Write(new Rule());
    }

    private void ShowExistingTasks(List<string> tasks)
    {
        ShowInfo("Pomodoro tasks from config:");
        foreach (var task in tasks)
            AnsiConsole.MarkupLine($"  - {task}");
    }

    private static List<string> PromptForNewTasks()
    {
        var tasks = new List<string>();
        AnsiConsole.MarkupLine("[grey]Enter tasks one per line. Empty line to finish.[/]");

        while (true)
        {
            var input = AnsiConsole.Prompt(new TextPrompt<string>("[blue]Task:[/]").AllowEmpty());
            if (string.IsNullOrWhiteSpace(input)) break;
            tasks.Add(input);
        }

        return tasks;
    }
}
