using System.Text.Json;
using System.Text.Json.Serialization;

namespace Pomodoro;

public class PomodoroSchedule
{
  [JsonPropertyName("timeBlocks")]
  public List<TimeBlock> TimeBlocks { get; set; } = [];

  public static PomodoroSchedule LoadFromFile(string path)
  {
    var json = File.ReadAllText(path);
    return JsonSerializer.Deserialize<PomodoroSchedule>(json)
        ?? throw new InvalidOperationException("Failed to deserialize schedule.");
  }

}

public class TimeBlock
{
  [JsonPropertyName("workDuration")]
  public int WorkDuration { get; set; }

  [JsonPropertyName("restDuration")]
  public int RestDuration { get; set; }

  [JsonPropertyName("task")]
  public PomodoroTask Task { get; set; } = new();
}

public class PomodoroTask
{
  [JsonPropertyName("name")]
  public string Name { get; set; } = "";

  [JsonPropertyName("description")]
  public string Description { get; set; } = "";
}
