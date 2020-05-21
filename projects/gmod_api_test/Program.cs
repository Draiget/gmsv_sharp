using System;
using log4net;

namespace gmod_api_test
{
    public class Program
    {
        private static readonly ILog Logger = LogManager.GetLogger(typeof(Program));

        public static void Main(string[] args) {
            GmodSharpCore.Core.Initialize(string.Empty, @"A:\work\coreclr_integration\output\Debug\rt", @"A:\work\gmsv_sharp\output\Debug", "gmod_api.dll");
            GmodSharpCore.Core.CompileInline("Log.Printf(\"test!\");");
        }
    }
}
