using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Reflection;
using System.Threading.Tasks;
using log4net;
using log4net.Config;
using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;

namespace ImageServer
{
    public class Program
    {
        private static readonly log4net.ILog log = log4net.LogManager.GetLogger(typeof(Program));

        static CommandChannelServer _commandServer = new CommandChannelServer(); 

        public static void Main(string[] args)
        {
            var logRepository = LogManager.GetRepository(Assembly.GetEntryAssembly());
            XmlConfigurator.Configure(logRepository, new FileInfo("log4net.config"));

            Task.Run(() => _commandServer.StartListening());

            Console.ReadLine();

            //CreateWebHostBuilder(args).Build().Run();
        }

        public static IWebHostBuilder CreateWebHostBuilder(string[] args) =>
            WebHost.CreateDefaultBuilder(args)
                .UseStartup<Startup>();


        /*
        public static void SocketServer()
        {
            Socket listenSocket = new Socket(AddressFamily.InterNetwork,
                                 SocketType.Stream,
                                 ProtocolType.Tcp);

            // bind the listening socket to the port
            Dns.GetHostEntry()
            IPAddress hostIP = (Dns.Resolve(IPAddress.Any.ToString())).AddressList[5];
            IPEndPoint ep = new IPEndPoint(hostIP, 62101);
            listenSocket.Bind(ep);

            // start listening
            listenSocket.Listen(100);
            while(true)
            {
                byte[] buffer = new byte[100];

                var s=listenSocket.Accept();
                while (s.Connected)
                {
                    System.Threading.Thread.Sleep(10 * 1000);
                    int data;
                    if (s.Available > 0)
                    {
                        data = s.Receive(buffer, 0, s.Available, SocketFlags.None);
                        Console.WriteLine(System.Text.Encoding.ASCII.GetString(buffer, 0, data));
                    }
                    s.Send(System.Text.Encoding.ASCII.GetBytes("Interval=1000\r\n"));
                }
            }
        }
        */
    }
}
