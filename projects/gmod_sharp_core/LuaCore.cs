using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace GmodSharpCore
{
    public static class LuaCore
    {
        private const string BinaryLibName = "gmsv_sharp_win32.dll";

        public static TR CallGeneric<TR, T>(params object[] args) where T : Delegate {
            var delegateType = typeof(T);
            var returnType = typeof(TR);
            var method = delegateType.GetMethod("Invoke");
            if (method == null) {
                return default;
            }

            var attribute = delegateType.GetCustomAttribute<LuaFunctionAttribute>();
            if (attribute == null) {
                return default;
            }

            CsPushSpecial(attribute.Table, out _);
            CsGetField(-1, attribute.Name, out _);

            var paramCount = method.GetParameters().Length;
            for (var i = 0; i < paramCount; i++) {
                if (args[i] == null) {
                    CsPushNil(out _);
                    continue;
                }

                var param = method.GetParameters()[i];
                if (param.ParameterType == typeof(string)) {
                    CsPushString((string)args[i], out _);
                } else if (param.ParameterType == typeof(int) ||
                           param.ParameterType == typeof(double) ||
                           param.ParameterType == typeof(float) ||
                           param.ParameterType == typeof(long) ||
                           param.ParameterType == typeof(uint) ||
                           param.ParameterType == typeof(short) ||
                           param.ParameterType == typeof(byte)) {
                    CsPushNumber((double)args[i], out _);
                } else if (param.ParameterType == typeof(bool)) {
                    CsPushBool((bool)args[i], out _);
                } else {
                    CsPushNil(out _);
                }
            }

            CsCall(paramCount, 1, out _);
            object returnValue = default;
            if (returnType == typeof(string)) {
                returnValue = CsGetString(-1, out _);
            } else if (returnType == typeof(int) ||
                       returnType == typeof(double) ||
                       returnType == typeof(float) ||
                       returnType == typeof(long) ||
                       returnType == typeof(uint) ||
                       returnType == typeof(short) ||
                       returnType == typeof(byte)) 
            {
                returnValue = CsGetNumber(-1, out _);
            } else if (returnType == typeof(bool)) {
                returnValue = CsGetBool(-1, out _);
            }

            CsPop(1, out _);
            return (TR)returnValue;
        }

        public static bool CallGeneric<T>(params object[] args) where T : Delegate {
            var delegateType = typeof(T);
            var method = delegateType.GetMethod("Invoke");
            if (method == null) {
                return false;
            }

            var attribute = delegateType.GetCustomAttribute<LuaFunctionAttribute>();
            if (attribute == null) {
                return false;
            }

            CsPushSpecial(attribute.Table, out _);
            CsGetField(-1, attribute.Name, out _);

            var paramCount = method.GetParameters().Length;
            for (var i = 0; i < paramCount; i++) {
                if (args[i] == null) {
                    CsPushNil(out _);
                    continue;
                }

                var param = method.GetParameters()[i];
                if (param.ParameterType == typeof(string)) {
                    CsPushString((string)args[i], out _);
                } else if (param.ParameterType == typeof(int) ||
                           param.ParameterType == typeof(double) ||
                           param.ParameterType == typeof(float) ||
                           param.ParameterType == typeof(long) ||
                           param.ParameterType == typeof(uint) ||
                           param.ParameterType == typeof(short) ||
                           param.ParameterType == typeof(byte)) 
                {
                    CsPushNumber((double)args[i], out _);
                } else if (param.ParameterType == typeof(bool)) {
                    CsPushBool((bool)args[i], out _);
                } else {
                    CsPushNil(out _);
                }
            }

            CsCall(paramCount, 0, out _);
            CsPop(1, out _);
            return true;
        }

        public static bool PushNil() {
            CsPushNil(out var result);
            return result;
        }

        public static bool PushSpecial(EGmodSpecial type) {
            CsPushSpecial(type, out var result);
            return result;
        }

        public static bool PushString(string value) {
            CsPushString(value, out var result);
            return result;
        }

        public static bool PushNumber(double value) {
            CsPushNumber(value, out var result);
            return result;
        }

        public static bool PushBool(bool value) {
            CsPushBool(value, out var result);
            return result;
        }

        public static bool Pop(int offset = 1) {
            CsPop(offset, out var result);
            return result;
        }

        public static bool Call(int numArgs, int numResults) {
            CsCall(numArgs, numResults, out var result);
            return result;
        }

        public static bool GetField(int offset, string name) {
            CsGetField(offset, name, out var result);
            return result;
        }

        public static string GetString(int stackPos = -1) {
            return CsGetString(stackPos, out _);
        }

        public static string GetString(out bool result, int stackPos = -1) {
            return CsGetString(stackPos, out result);
        }

        public static double GetNumber(int stackPos = -1) {
            return CsGetNumber(stackPos, out _);
        }

        public static double GetNumber(out bool result, int stackPos = -1) {
            return CsGetNumber(stackPos, out result);
        }

        public static bool GetBool(int stackPos = -1) {
            return CsGetBool(stackPos, out _);
        }

        public static bool GetBool(out bool result, int stackPos = -1) {
            return CsGetBool(stackPos, out result);
        }

        [DllImport(BinaryLibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "CS_PushSpecial")]
        internal static extern void CsPushSpecial(EGmodSpecial type, out bool result);

        [DllImport(BinaryLibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "CS_PushString")]
        internal static extern void CsPushString(string value, out bool result);

        [DllImport(BinaryLibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "CS_PushNil")]
        internal static extern void CsPushNil(out bool result);

        [DllImport(BinaryLibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "CS_PushNumber")]
        internal static extern void CsPushNumber(double value, out bool result);

        [DllImport(BinaryLibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "CS_PushBool")]
        internal static extern void CsPushBool(bool value, out bool result);

        [DllImport(BinaryLibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "CS_Call")]
        internal static extern void CsCall(int numArgs, int numResults, out bool result);

        [DllImport(BinaryLibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "CS_Pop")]
        internal static extern void CsPop(int offset, out bool result);

        [DllImport(BinaryLibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "CS_GetField")]
        internal static extern void CsGetField(int offset, string name, out bool result);

        [DllImport(BinaryLibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "CS_GetString")]
        internal static extern string CsGetString(int stackPos, out bool result);

        [DllImport(BinaryLibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "CS_GetNumber")]
        internal static extern double CsGetNumber(int stackPos, out bool result);

        [DllImport(BinaryLibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "CS_GetBool")]
        internal static extern bool CsGetBool(int stackPos, out bool result);
    }
}
