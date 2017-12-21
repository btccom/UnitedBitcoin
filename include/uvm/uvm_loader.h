
#ifndef uvm_loader_h
#define uvm_loader_h

#include "uvm/lprefix.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <list>

#include "uvm/llimits.h"
#include "uvm/lstate.h"
#include "uvm/lua.h"
#include "uvm/uvm_api.h"

namespace uvm
{
    namespace lua
    {
        namespace parser
        {

            class LuaLoader
            {
            private:
                GluaModuleByteStream *_stream; // byte code stream
            public:
                LuaLoader(GluaModuleByteStream *stream);
                ~LuaLoader();

                void load_bytecode(); // load bytecode stream to AST
            };

        }
    }
}

#endif