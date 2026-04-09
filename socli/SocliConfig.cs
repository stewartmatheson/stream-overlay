using System.Text.Json;
using System.Text.Json.Serialization;

namespace Socli;

public class SocliConfig
{
  [JsonPropertyName("pomodoro")]
  public PomodoroConfig Pomodoro { get; set; } = new();

  [JsonPropertyName("applications")]
  public List<ApplicationConfig> Applications { get; set; } = [];

  [JsonPropertyName("activities")]
  public List<Activity> Activities { get; set; } = [];

  [JsonPropertyName("twitch")]
  public TwitchConfig Twitch { get; set; } = new();

  [JsonIgnore]
  public string FilePath { get; private set; } = DefaultPath;

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
    var config = JsonSerializer.Deserialize<SocliConfig>(json)
        ?? throw new InvalidOperationException("Failed to deserialize configuration.");
    config.FilePath = path;
    return config;
  }

  public void Save(string? path = null)
  {
    path ??= FilePath;
    var options = new JsonSerializerOptions { WriteIndented = true };
    File.WriteAllText(path, JsonSerializer.Serialize(this, options));
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
  [JsonPropertyName("title")]
  public string Title { get; set; } = "";

  [JsonPropertyName("category")]
  public string Category { get; set; } = "";

  [JsonPropertyName("notification")]
  public string Notification { get; set; } = "";

  [JsonPropertyName("tags")]
  public List<string> Tags { get; set; } = [];

  [JsonPropertyName("usePomodoro")]
  public bool UsePomodoro { get; set; }

  [JsonPropertyName("checklist")]
  public List<string> Checklist { get; set; } = [];
}

public class ApplicationConfig
{
  [JsonPropertyName("path")]
  public string Path { get; set; } = "";

  [JsonPropertyName("workingDirectory")]
  public string? WorkingDirectory { get; set; }
}

public class TwitchConfig
{
  [JsonPropertyName("clientId")]
  public string ClientId { get; set; } = "";

  [JsonPropertyName("broadcasterId")]
  public string BroadcasterId { get; set; } = "";
}
