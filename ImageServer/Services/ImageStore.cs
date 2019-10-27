using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace ImageServer.Services
{
    public class ImageStoreService
    {
        const int DefaultUploadInterval = 10 * 60 * 1000;

        Dictionary<string, ImageStoreDataItem> _images = new Dictionary<string, ImageStoreDataItem>();

        public byte[] GetImage(string clientID)
        {
            lock (_images)
            {
                ImageStoreDataItem di;
                if (_images.TryGetValue(clientID, out di))
                {
                    di.LastImageRead[1] = di.LastImageRead[0];
                    di.LastImageRead[0] = DateTime.Now;

                    return (di.Image);
                }
                else
                {
                    return null;
                }
            }
        }

        public void UpdateImage(string clientID, byte[] image)
        {
            lock (_images)
            {
                ImageStoreDataItem di;
                if (_images.TryGetValue(clientID, out di))
                {
                    di.Image = image;
                }
                else
                {
                    di = new ImageStoreDataItem();
                    di.Image = image;
                    _images[clientID] = di;
                }
            }
        }

        public int GetUpdateInterval(string clientID)
        {
            lock (_images)
            {
                ImageStoreDataItem di;
                if (_images.TryGetValue(clientID, out di))
                {
                    // Ne reads lately. Use default value.
                    if (di.LastImageRead[0] == DateTime.MinValue || di.LastImageRead[0] == DateTime.MinValue)
                        return DefaultUploadInterval;

                    // Calculate difference of last to reads.
                    int result = (int)di.LastImageRead[0].Subtract(di.LastImageRead[1]).TotalMilliseconds;

                    // Limit to 500ms 
                    if (result < 500)
                        result = 500;

                    // Last read longer than default upload rate. Return to default values.
                    if (DateTime.UtcNow.Subtract(di.LastImageRead[0]).TotalMilliseconds > DefaultUploadInterval)
                    {
                        di.LastImageRead[0] = DateTime.MinValue;
                        di.LastImageRead[0] = DateTime.MinValue;
                    }

                    return result;
                }
                else
                {
                    di = new ImageStoreDataItem();
                    di.Image = null;
                    _images[clientID] = di;

                    return DefaultUploadInterval;
                }
            }
        }
    }
}
