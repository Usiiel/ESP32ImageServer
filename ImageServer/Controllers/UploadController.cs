using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using ImageServer.Services;
using Microsoft.AspNetCore.Mvc;

namespace ImageServer.Controllers
{
    [Route("/upload")]
    [ApiController]
    public class UploadController : ControllerBase
    {
        ImageStoreService _imageStore;

        public UploadController(ImageStoreService imageStore)
        {
            _imageStore = imageStore;
        }

        // POST api/values
        [HttpPost]
        public void Post()
        {
            var clientID = Request.Headers["Client-ID"].First();

            Tools.ValidateClientID(clientID);

            using (MemoryStream ms = new MemoryStream())
            {
                Request.Body.CopyTo(ms);
                _imageStore.UpdateImage(clientID, ms.GetBuffer());
                Console.WriteLine("Upload");
            }
        }
    }
}
