using System;
using System.Net.Sockets;
using System.Text;

public class CPHInline
{
    public bool Execute()
    {
        string host = "127.0.0.1";
        int port = 7777;

        // Determine command type from arguments (default: "popup")
        string commandType = args.ContainsKey("commandType")
            ? args["commandType"].ToString()
            : "popup";

        string message;

        switch (commandType)
        {
            case "spawn_ball":
                message = BuildSpawnBallMessage();
                break;
            case "bottompopup":
                message = BuildPopupMessage("bottompopup");
                break;
            case "popup":
            default:
                message = BuildPopupMessage("popup");
                break;
        }

        try
        {
            using (TcpClient client = new TcpClient(host, port))
            {
                NetworkStream stream = client.GetStream();
                byte[] data = Encoding.UTF8.GetBytes(message + "\n");
                stream.Write(data, 0, data.Length);
                CPH.LogInfo("Overlay message sent: " + message);
            }
        }
        catch (Exception ex)
        {
            CPH.LogError("Overlay send failed: " + ex.Message);
        }

        return true;
    }

    private string BuildPopupMessage(string command)
    {
        // Format: popup Title|Body[|BorderColor][|BgColor]
        string title = args.ContainsKey("title")
            ? args["title"].ToString()
            : "Alert";

        string body = args.ContainsKey("body")
            ? args["body"].ToString()
            : "";

        string payload = title + "|" + body;

        if (args.ContainsKey("borderColor"))
            payload += "|" + args["borderColor"].ToString();

        if (args.ContainsKey("bgColor"))
            payload += "|" + args["bgColor"].ToString();

        return command + " " + payload;
    }

    private string BuildSpawnBallMessage()
    {
        // Format: spawn_ball [cx cy vx vy r g b]
        string msg = "spawn_ball";

        if (args.ContainsKey("cx") && args.ContainsKey("cy"))
        {
            msg += " " + args["cx"].ToString()
                 + " " + args["cy"].ToString();

            if (args.ContainsKey("vx") && args.ContainsKey("vy"))
            {
                msg += " " + args["vx"].ToString()
                     + " " + args["vy"].ToString();
            }

            if (args.ContainsKey("r") && args.ContainsKey("g") && args.ContainsKey("b"))
            {
                msg += " " + args["r"].ToString()
                     + " " + args["g"].ToString()
                     + " " + args["b"].ToString();
            }
        }

        return msg;
    }
}
