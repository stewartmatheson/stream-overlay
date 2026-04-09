using System.Diagnostics;
using System.Net;
using System.Net.Http.Json;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Socli;

public class TwitchService : IDisposable
{
  private readonly HttpClient _http = new();
  private readonly SocliConfig _config;
  private const string ApiBase = "https://api.twitch.tv/helix";
  private const string AuthBase = "https://id.twitch.tv/oauth2";
  private const int LocalPort = 17563;
  private const string RedirectUri = "http://localhost:17563";
  private const string Scopes = "channel:manage:broadcast";

  public TwitchService(SocliConfig config)
  {
    _config = config;
    ApplyHeaders();
  }

  private void ApplyHeaders()
  {
    _http.DefaultRequestHeaders.Clear();
    var accessToken = TwitchCredentials.AccessToken;
    if (!string.IsNullOrEmpty(accessToken))
      _http.DefaultRequestHeaders.Add("Authorization", $"Bearer {accessToken}");
    if (!string.IsNullOrEmpty(_config.Twitch.ClientId))
      _http.DefaultRequestHeaders.Add("Client-Id", _config.Twitch.ClientId);
  }

  public bool IsConfigured =>
      !string.IsNullOrEmpty(_config.Twitch.ClientId) &&
      !string.IsNullOrEmpty(TwitchCredentials.ClientSecret);

  public bool IsAuthenticated =>
      !string.IsNullOrEmpty(TwitchCredentials.AccessToken);

  public async Task<bool> AuthenticateAsync()
  {
    var state = Guid.NewGuid().ToString("N");

    var authUrl = $"{AuthBase}/authorize" +
        $"?client_id={Uri.EscapeDataString(_config.Twitch.ClientId)}" +
        $"&redirect_uri={Uri.EscapeDataString(RedirectUri)}" +
        $"&response_type=code" +
        $"&scope={Uri.EscapeDataString(Scopes)}" +
        $"&state={state}";

    using var listener = new HttpListener();
    listener.Prefixes.Add($"{RedirectUri}/");
    listener.Start();

    Process.Start(new ProcessStartInfo(authUrl) { UseShellExecute = true });

    var context = await listener.GetContextAsync();
    var code = context.Request.QueryString["code"];
    var returnedState = context.Request.QueryString["state"];

    var html = "<html><body><h1>Authentication successful!</h1>"
        + "<p>You can close this window.</p></body></html>";
    var bytes = System.Text.Encoding.UTF8.GetBytes(html);
    context.Response.ContentLength64 = bytes.Length;
    context.Response.ContentType = "text/html";
    await context.Response.OutputStream.WriteAsync(bytes);
    context.Response.Close();
    listener.Stop();

    if (returnedState != state || string.IsNullOrEmpty(code))
      return false;

    return await ExchangeCodeAsync(code);
  }

  private async Task<bool> ExchangeCodeAsync(string code)
  {
    var clientSecret = TwitchCredentials.ClientSecret;
    if (string.IsNullOrEmpty(clientSecret)) return false;

    var response = await _http.PostAsync($"{AuthBase}/token",
        new FormUrlEncodedContent(new Dictionary<string, string>
        {
          ["client_id"] = _config.Twitch.ClientId,
          ["client_secret"] = clientSecret,
          ["code"] = code,
          ["grant_type"] = "authorization_code",
          ["redirect_uri"] = RedirectUri
        }));

    if (!response.IsSuccessStatusCode) return false;

    var token = await response.Content.ReadFromJsonAsync<TokenResponse>();
    if (token is null) return false;

    TwitchCredentials.SaveTokens(token.AccessToken, token.RefreshToken);
    ApplyHeaders();

    var userId = await GetBroadcasterIdAsync();
    if (userId is not null)
    {
      _config.Twitch.BroadcasterId = userId;
      _config.Save();
    }

    return true;
  }

  public async Task<bool> RefreshTokenAsync()
  {
    var refreshToken = TwitchCredentials.RefreshToken;
    var clientSecret = TwitchCredentials.ClientSecret;
    if (string.IsNullOrEmpty(refreshToken) || string.IsNullOrEmpty(clientSecret))
      return false;

    var response = await _http.PostAsync($"{AuthBase}/token",
        new FormUrlEncodedContent(new Dictionary<string, string>
        {
          ["client_id"] = _config.Twitch.ClientId,
          ["client_secret"] = clientSecret,
          ["refresh_token"] = refreshToken,
          ["grant_type"] = "refresh_token"
        }));

    if (!response.IsSuccessStatusCode) return false;

    var token = await response.Content.ReadFromJsonAsync<TokenResponse>();
    if (token is null) return false;

    TwitchCredentials.SaveTokens(token.AccessToken, token.RefreshToken);
    ApplyHeaders();
    return true;
  }

  private async Task<string?> GetBroadcasterIdAsync()
  {
    var response = await _http.GetAsync($"{ApiBase}/users");
    if (!response.IsSuccessStatusCode) return null;

    var result = await response.Content.ReadFromJsonAsync<TwitchData<TwitchUser>>();
    return result?.Data?.FirstOrDefault()?.Id;
  }

  public async Task<SearchCategoryResult> SearchCategoryAsync(string name)
  {
    var response = await SendCategorySearch(name);

    if (response.StatusCode == HttpStatusCode.Unauthorized && await RefreshTokenAsync())
      response = await SendCategorySearch(name);

    if (response.StatusCode == HttpStatusCode.Unauthorized)
    {
      var body = await response.Content.ReadAsStringAsync();
      return SearchCategoryResult.Fail(response.StatusCode,
          "Twitch API returned 401 Unauthorized. Your access token is missing, expired, or lacks the required scopes. "
          + "Re-run `socli twitch-setup` to re-authenticate."
          + (string.IsNullOrWhiteSpace(body) ? "" : $" Details: {body}"));
    }

    if (!response.IsSuccessStatusCode)
    {
      var body = await response.Content.ReadAsStringAsync();
      return SearchCategoryResult.Fail(response.StatusCode,
          $"Twitch category search failed ({(int)response.StatusCode} {response.StatusCode}). {body}");
    }

    var result = await response.Content.ReadFromJsonAsync<TwitchData<TwitchCategory>>();
    var exact = result?.Data?.FirstOrDefault(c =>
        c.Name.Equals(name, StringComparison.OrdinalIgnoreCase));
    var category = exact ?? result?.Data?.FirstOrDefault();
    return SearchCategoryResult.Ok(category, response.StatusCode);
  }

  private async Task<HttpResponseMessage> SendCategorySearch(string name)
  {
    return await _http.GetAsync(
        $"{ApiBase}/search/categories?query={Uri.EscapeDataString(name)}");
  }

  public async Task<UpdateChannelResult> UpdateChannelAsync(string title, string gameId, List<string> tags)
  {
    var broadcasterId = _config.Twitch.BroadcasterId;
    if (string.IsNullOrEmpty(broadcasterId))
      return new UpdateChannelResult(false, null, "Twitch broadcasterId not configured.");

    var body = BuildChannelBody(title, gameId, tags);

    var response = await SendChannelPatch(broadcasterId, body);

    if (response.StatusCode == HttpStatusCode.Unauthorized && await RefreshTokenAsync())
      response = await SendChannelPatch(broadcasterId, body);

    if (response.IsSuccessStatusCode)
      return new UpdateChannelResult(true, response.StatusCode, null);

    var errorBody = await response.Content.ReadAsStringAsync();
    return new UpdateChannelResult(false, response.StatusCode, errorBody);
  }

  private static Dictionary<string, object> BuildChannelBody(
      string title, string gameId, List<string> tags)
  {
    var body = new Dictionary<string, object> { ["title"] = title, ["game_id"] = gameId };
    if (tags.Count > 0)
      body["tags"] = tags;
    return body;
  }

  private async Task<HttpResponseMessage> SendChannelPatch(
      string broadcasterId, Dictionary<string, object> body)
  {
    var request = new HttpRequestMessage(HttpMethod.Patch,
        $"{ApiBase}/channels?broadcaster_id={broadcasterId}")
    {
      Content = JsonContent.Create(body)
    };
    return await _http.SendAsync(request);
  }

  public void Dispose() => _http.Dispose();
}

public record UpdateChannelResult(bool Success, HttpStatusCode? StatusCode, string? Error);

public record SearchCategoryResult(
    bool Success, TwitchCategory? Category, HttpStatusCode? StatusCode, string? Error)
{
  public static SearchCategoryResult Ok(TwitchCategory? category, HttpStatusCode statusCode) =>
      new(true, category, statusCode, null);

  public static SearchCategoryResult Fail(HttpStatusCode? statusCode, string error) =>
      new(false, null, statusCode, error);
}

public class TokenResponse
{
  [JsonPropertyName("access_token")]
  public string AccessToken { get; set; } = "";

  [JsonPropertyName("refresh_token")]
  public string RefreshToken { get; set; } = "";
}

public class TwitchData<T>
{
  [JsonPropertyName("data")]
  public List<T> Data { get; set; } = [];
}

public class TwitchUser
{
  [JsonPropertyName("id")]
  public string Id { get; set; } = "";
}

public class TwitchCategory
{
  [JsonPropertyName("id")]
  public string Id { get; set; } = "";

  [JsonPropertyName("name")]
  public string Name { get; set; } = "";
}
