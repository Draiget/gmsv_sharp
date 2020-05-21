using System;
using System.Collections.Generic;
using System.Text;
using GmodSharpCore;

namespace GmodApi
{
    public class Time
    {
        [LuaFunction("SysTime")]
        public delegate double LuaFnSysTime();

        public static double SysTime(string msg) {
            return LuaCore.CallGeneric<double, LuaFnSysTime>(msg);
        }
    }
}
