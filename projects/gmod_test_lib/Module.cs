using System;
using GmodApi;

namespace gmod_test_lib
{
    public class Module : ICoreModule
    {
        public bool Open() {
            return true;
        }

        public bool Shutdown() {
            return true;
        }
    }
}
