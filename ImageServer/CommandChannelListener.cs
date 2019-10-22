using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

public class CommandChannelServer
{
    private static readonly log4net.ILog log = log4net.LogManager.GetLogger(typeof(CommandChannelServer));

    public volatile Semaphore ShouldStop = new Semaphore(0, 1000);

    public  ManualResetEvent allDone = new ManualResetEvent(false);
    public List<CommandChannelState> Connections = new List<CommandChannelState>();


    public CommandChannelServer()
    {
        Task.Run(() =>
        {
            while (!ShouldStop.WaitOne(10 * 1000))
            {
                SendIntervalUpdates();
            }
        });
    }



    public void Stop()
    {
        ShouldStop.Release(1000);
    }

    int CalculateInterval(CommandChannelState state)
    {
        if (state.LastImageRead[0] == DateTime.MinValue || state.LastImageRead[0] == DateTime.MinValue)
            return 60 * 1000;

        int result = (int)state.LastImageRead[0].Subtract(state.LastImageRead[1]).TotalMilliseconds;
        if (result < 500)
            result = 500;

        return result;
    }


    void SendIntervalUpdates()
    {
        lock (Connections)
        {
            for(int i=Connections.Count-1;i>0;i--)
            {
                if (!Connections[i].workSocket.Connected)
                    Connections.RemoveAt(i);
            }

            foreach(var c in Connections)
            {
                try
                {
                    int interval = CalculateInterval(c);
                    Send(c.workSocket, "Interval = " + interval + "\r\n");
                    log.Debug("Sending interval " + interval + " to client " + c.ClientID);
                }
                catch(Exception ex)
                {
                    log.Warn(ex);
                }
            }
        }
    }


    public  void StartListening()
    {
        // Create a TCP/IP socket.  
        Socket listener = new Socket(IPAddress.Any.AddressFamily,
            SocketType.Stream, ProtocolType.Tcp);

        // Bind the socket to the local endpoint and listen for incoming connections.  
        try
        {
            listener.Bind(new IPEndPoint(IPAddress.Any,11000));
            listener.Listen(100);

            while (!ShouldStop.WaitOne(0))
            {
                // Set the event to nonsignaled state.  
                allDone.Reset();

                // Start an asynchronous socket to listen for connections.  
                Console.WriteLine("Waiting for a connection...");
                listener.BeginAccept(
                    new AsyncCallback(AcceptCallback),
                    listener);

                // Wait until a connection is made before continuing.  
                allDone.WaitOne();
            }

        }
        catch (Exception e)
        {
            Console.WriteLine(e.ToString());
        }

        Console.WriteLine("\nPress ENTER to continue...");
        Console.Read();

    }


    public  void AcceptCallback(IAsyncResult ar)
    {
        // Signal the main thread to continue.  
        allDone.Set();

        // Get the socket that handles the client request.  
        Socket listener = (Socket)ar.AsyncState;
        Socket handler = listener.EndAccept(ar);

        // Create the state object.  
        CommandChannelState state = new CommandChannelState();
        state.workSocket = handler;
        lock(Connections)
        {
            Connections.Add(state);
        }
        handler.BeginReceive(state.buffer, 0, CommandChannelState.BufferSize, 0,
            new AsyncCallback(ReadCallback), state);
    }


    public  void ReadCallback(IAsyncResult ar)
    {
        String content = String.Empty;

        // Retrieve the state object and the handler socket  
        // from the asynchronous state object.  
        CommandChannelState state = (CommandChannelState)ar.AsyncState;
        Socket handler = state.workSocket;

        // Read data from the client socket.   
        int bytesRead = handler.EndReceive(ar);

        if (bytesRead > 0)
        {
            // There  might be more data, so store the data received so far.  
            state.sb.Append(Encoding.ASCII.GetString(
                state.buffer, 0, bytesRead));

            // Check for end-of-file tag. If it is not there, read   
            // more data.  
            content = state.sb.ToString();
            int pos = content.IndexOf("\n");
            if (pos != -1)
            {
                state.sb.Remove(0, pos + 1);
                string line = content.Substring(0, pos).Replace("\r", "");
                ProcessLineFromClient(line, state);
            }
        }
        else
        {
            handler.Disconnect(false);
            return;
        }
        // Not all data received. Get more.  
        handler.BeginReceive(state.buffer, 0, CommandChannelState.BufferSize, 0,
        new AsyncCallback(ReadCallback), state);
    }


    private  void ProcessLineFromClient(string line, CommandChannelState state)
    {
        if(line.StartsWith("Client-ID:"))
        {
            string id = line.Substring(10);
            state.ClientID = id.Trim();
        }
    }


    private  void Send(Socket handler, String data)
    {
        // Convert the string data to byte data using ASCII encoding.  
        byte[] byteData = Encoding.ASCII.GetBytes(data);

        // Begin sending the data to the remote device.  
        handler.BeginSend(byteData, 0, byteData.Length, 0,
            new AsyncCallback(SendCallback), handler);
    }


    private  void SendCallback(IAsyncResult ar)
    {
        try
        {
            // Retrieve the socket from the state object.  
            Socket handler = (Socket)ar.AsyncState;

            // Complete sending the data to the remote device.  
            int bytesSent = handler.EndSend(ar);
            Console.WriteLine("Sent {0} bytes to client.", bytesSent);
        }
        catch (Exception e)
        {
            Console.WriteLine(e.ToString());
        }
    }
}
