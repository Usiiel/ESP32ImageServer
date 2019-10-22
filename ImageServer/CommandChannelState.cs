using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

public class CommandChannelState
{
    // Client  socket.  
    public Socket workSocket = null;
    // Size of receive buffer.  
    public const int BufferSize = 1024;
    // Receive buffer.  
    public byte[] buffer = new byte[BufferSize];
    // Received data string.  
    public StringBuilder sb = new StringBuilder();
    public string ClientID;
    public DateTime[] LastImageRead = new [] {DateTime.MinValue,DateTime.MinValue };
}