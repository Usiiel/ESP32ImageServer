using ImageServer.Services;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using System.Collections.Generic;
using System.IO;
using System.Net;

namespace ImageServer.Controllers
{
    [Route("/view")]
    [ApiController]
    public class ViewController : ControllerBase
    {
        IConfiguration _config;
        ImageStoreService _imageStore;

        public ViewController(IConfiguration config, ImageStoreService imageStore)
        {
            _config = config;
            _imageStore = imageStore;
        }

        [Route("/view/{clientID}/")]
        public ActionResult<string> GetIndex(string clientID)
        {
            Tools.ValidateClientID(clientID);

            var html = System.IO.File.ReadAllText("html"+ Path.DirectorySeparatorChar+"ViewTemplate.html").Replace("##ID##", clientID).Replace("##WWWROOT##", _config["Config:ImageServerWWWRoot"]);

            return new ContentResult
            {
                ContentType = "text/html",
                StatusCode = (int)HttpStatusCode.OK,
                Content = html
            };
        }

        [Route("/view/{clientID}/current.jpg")]
        public ActionResult<string> Get(string clientID)
        {
            Tools.ValidateClientID(clientID);

            var img = _imageStore.GetImage(clientID);

            if (img == null)
                return NotFound();

            return File(img, "image/jpeg");
        }
    }
}
