using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Pomodoro;

const int Port = 7778;

if (args.Length > 0 && args[0] == "server")
{
  await RunServer(args);
}
else if (args.Length >= 2 && args[0] == "add")
{
  var taskName = string.Join(' ', args.Skip(1));
  await SendCommand($"ADD:{taskName}");
}
else if (args.Length > 0 && args[0] == "status")
{
  var response = await SendCommandWithResponse("STATUS");
  Console.Write(response);
}
else if (args.Length > 0 && args[0] == "reset")
{
  await SendCommand("RESET");
}
else
{
  await RunClient();
}

async Task RunClient()
{
  var json = await Console.In.ReadToEndAsync();
  await SendCommand($"SCHEDULE:{json}");
}

async Task SendCommand(string message)
{
  using var client = new TcpClient("127.0.0.1", Port);
  using var stream = client.GetStream();
  var bytes = Encoding.UTF8.GetBytes(message);
  stream.Write(bytes, 0, bytes.Length);
  client.Client.Shutdown(SocketShutdown.Send);
}

async Task<string> SendCommandWithResponse(string message)
{
  using var client = new TcpClient("127.0.0.1", Port);
  using var stream = client.GetStream();
  var bytes = Encoding.UTF8.GetBytes(message);
  stream.Write(bytes, 0, bytes.Length);
  client.Client.Shutdown(SocketShutdown.Send);

  using var reader = new StreamReader(stream, Encoding.UTF8);
  return await reader.ReadToEndAsync();
}

async Task RunServer(string[] args)
{
  var builder = Host.CreateApplicationBuilder(args);
  builder.Services.AddWindowsService();

  builder.Services.AddSingleton<LoggingEventHandler>();
  builder.Services.AddSingleton<OverlayEventHandler>();
  builder.Services.AddSingleton<PomodoroScheduler>(sp =>
    new PomodoroScheduler(
      sp.GetRequiredService<LoggingEventHandler>(),
      sp.GetRequiredService<OverlayEventHandler>(),
      sp.GetRequiredService<ILogger<PomodoroScheduler>>()));
  builder.Services.AddHostedService<PomodoroService>();

  var host = builder.Build();
  await host.RunAsync();
}
