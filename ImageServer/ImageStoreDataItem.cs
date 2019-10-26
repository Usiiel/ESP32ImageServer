using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace ImageServer
{
    public class ImageStoreDataItem
    {
        public byte[] Image;
        public DateTime[] LastImageRead = new[] { DateTime.MinValue, DateTime.MinValue };
    }
}
