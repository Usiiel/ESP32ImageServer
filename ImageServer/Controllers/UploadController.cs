using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;

namespace ImageServer.Controllers
{
    [Route("/posts")]
    [ApiController]
    public class UploadController : ControllerBase
    {
        // POST api/values
        [HttpPost]
        public void Post()
        {
            var clientID = Request.Headers["Client-ID"].First();
            using (MemoryStream ms = new MemoryStream())
            {
                Request.Body.CopyTo(ms);
                Program.ImageStore.UpdateImage(clientID, ms.GetBuffer());
                Console.WriteLine("Upload");
            }
        }
    }
}
