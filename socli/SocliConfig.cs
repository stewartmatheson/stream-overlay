using System.Text.Json;
using System.Text.Json.Serialization;

namespace Socli;

public class SocliConfig
{
  [JsonPropertyName("pomodoro")]
  public PomodoroConfig Pomodoro { get; set; } = new();

  [JsonPropertyName("applications")]
  public List<string> Applications { get; set; } = [];

  [JsonPropertyName("activities")]
  public List<Activity> Activities { get; set; } = [];

  public static string DefaultPath =>
      Path.Combine(
          Environment.GetFolderPath(Environment.SpecialFolder.UserProfile),
          ".socli.json"
      );

  public static SocliConfig Load(string? path = null)
  {
    path ??= DefaultPath;

    if (!File.Exists(path))
      throw new FileNotFoundException($"Configuration file not found: {path}");

    var json = File.ReadAllText(path);
    return JsonSerializer.Deserialize<SocliConfig>(json)
        ?? throw new InvalidOperationException("Failed to deserialize configuration.");
  }
}

public class PomodoroConfig
{
  [JsonPropertyName("taskTime")]
  public int TaskTime { get; set; } = 25;

  [JsonPropertyName("restTime")]
  public int RestTime { get; set; } = 5;
}

public class Activity
{
  [JsonPropertyName("name")]
  public string Name { get; set; } = "";

  [JsonPropertyName("category")]
  public string Category { get; set; } = "";

  [JsonPropertyName("notification")]
  public string Notification { get; set; } = "";

  [JsonPropertyName("tags")]
  public List<string> Tags { get; set; } = [];

  [JsonPropertyName("tasks")]
  public List<string> Tasks { get; set; } = [];

  [JsonPropertyName("checklist")]
  public List<string> Checklist { get; set; } = [];
}
