using GmodSharpCore;

namespace GmodApi
{
    public static class Log
    {
        public static void Printf(string format, params string[] args) {
            System.Console.WriteLine(format, args);
        }

        public static void Printf(string msg) {
            System.Console.WriteLine(msg);
        }

        [LuaFunction("ServerLog")]
        public delegate void LuaFnServerLog(string message);

        public static void Server(string msg) {
            LuaCore.CallGeneric<LuaFnServerLog>(msg);
        }
    }
}
