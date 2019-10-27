using System;
using System.IO;
using System.Reflection;
using System.Threading.Tasks;
using ImageServer.Services;
using log4net;
using log4net.Config;
using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.DependencyInjection;

namespace ImageServer
{
    public class Program
    {
        private static readonly log4net.ILog log = log4net.LogManager.GetLogger(typeof(Program));


        public static void Main(string[] args)
        {
            var logRepository = LogManager.GetRepository(Assembly.GetEntryAssembly());
            XmlConfigurator.Configure(logRepository, new FileInfo("log4net.config"));

            var webHost = CreateWebHostBuilder(args).Build();
            Task.Run(() => StartCommandService(webHost.Services));
            webHost.Run();
        }


        static void StartCommandService(IServiceProvider provider)
        {
            using (IServiceScope scope = provider.CreateScope())
            {
                var ccs=scope.ServiceProvider.GetRequiredService<CommandChannelService>();
                ccs.StartListening();
            }
        }


        public static IWebHostBuilder CreateWebHostBuilder(string[] args) =>
    WebHost.CreateDefaultBuilder(args)
            .UseStartup<Startup>();
    }
}
