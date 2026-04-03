using System;
using System.Net.Sockets;
using System.Text;

public class CPHInline
{
    public bool Execute()
    {
        string title = args.ContainsKey("title") ? args["title"].ToString() : "Hello";
        string body = args.ContainsKey("body") ? args["body"].ToString() : "";
        string borderColor = args.ContainsKey("borderColor") ? args["borderColor"].ToString() : "";
        string bgColor = args.ContainsKey("bgColor") ? args["bgColor"].ToString() : "";
        int port = args.ContainsKey("port") ? int.Parse(args["port"].ToString()) : 7777;

        string msg = $"bottompopup {title}|{body}";
        if (!string.IsNullOrEmpty(borderColor)) msg += $"|{borderColor}";
        if (!string.IsNullOrEmpty(bgColor)) msg += $"|{bgColor}";

        try
        {
            using (var client = new TcpClient("127.0.0.1", port))
            using (var stream = client.GetStream())
            {
                byte[] bytes = Encoding.UTF8.GetBytes(msg + "\n");
                stream.Write(bytes, 0, bytes.Length);
                stream.Flush();
            }
            CPH.LogInfo($"BottomPopup sent: {msg}");
        }
        catch (Exception ex)
        {
            CPH.LogError($"BottomPopup failed: {ex.Message}");
            return false;
        }

        return true;
    }
}
