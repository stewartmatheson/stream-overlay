using System.Runtime.Versioning;
using Meziantou.Framework.Win32;

namespace Socli;

[SupportedOSPlatform("windows5.1.2600")]
public static class TwitchCredentials
{
    private const string ClientSecretKey = "socli:twitch:clientSecret";
    private const string AccessTokenKey = "socli:twitch:accessToken";
    private const string RefreshTokenKey = "socli:twitch:refreshToken";

    public static string? ClientSecret => Read(ClientSecretKey);
    public static string? AccessToken => Read(AccessTokenKey);
    public static string? RefreshToken => Read(RefreshTokenKey);

    public static void SaveClientSecret(string value) =>
        Write(ClientSecretKey, value);

    public static void SaveTokens(string accessToken, string refreshToken)
    {
        Write(AccessTokenKey, accessToken);
        Write(RefreshTokenKey, refreshToken);
    }

    public static void Clear()
    {
        Delete(ClientSecretKey);
        Delete(AccessTokenKey);
        Delete(RefreshTokenKey);
    }

    private static string? Read(string key)
    {
        var cred = CredentialManager.ReadCredential(key);
        return cred?.Password;
    }

    private static void Write(string key, string value) =>
        CredentialManager.WriteCredential(key, "socli", value, CredentialPersistence.LocalMachine);

    private static void Delete(string key)
    {
        try { CredentialManager.DeleteCredential(key); }
        catch { /* credential may not exist */ }
    }
}
