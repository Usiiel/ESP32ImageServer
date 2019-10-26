using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using System.Collections.Generic;
using System.Net;

namespace ImageServer.Controllers
{
    [Route("/view")]
    [ApiController]
    public class ViewController : ControllerBase
    {
        IConfiguration _config;

        public ViewController(IConfiguration config)
        {
            _config = config;
        }

        [Route("/view/{clientID}/")] 
        public ActionResult<string> GetIndex(string clientID)
        {
            var html=System.IO.File.ReadAllText("html\\ViewTemplate.html").Replace("##ID##", clientID).Replace("##WWWROOT##", _config["Config:ImageServerWWWRoot"]);

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
            var img=Program.ImageStore.GetImage(clientID);

            if(img==null)
                return NotFound();

            return File(img, "image/jpeg");
        }
    }
}
