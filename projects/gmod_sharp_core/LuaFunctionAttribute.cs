using System;
using System.Collections.Generic;
using System.Text;

namespace GmodSharpCore
{
    [AttributeUsage(AttributeTargets.Delegate)]
    public class LuaFunctionAttribute : Attribute
    {
        public string Name { get; }
        public EGmodSpecial Table { get; }

        public LuaFunctionAttribute(string name, EGmodSpecial table = EGmodSpecial.Glob) {
            Name = name;
            Table = table;
        }
    }
}
