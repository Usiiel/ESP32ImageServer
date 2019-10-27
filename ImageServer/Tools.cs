using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace ImageServer
{
    public class Tools
    {
        public static void ValidateClientID(string clientID)
        {
            if (clientID==null || clientID.Length!=12 || !System.Text.RegularExpressions.Regex.IsMatch(clientID, @"\A\b[0-9a-fA-F]+\b\Z"))
                throw new Exception("Invalid client ID!");
        }
    }
}
