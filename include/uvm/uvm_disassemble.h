#ifndef uvm_disassemble_h
#define uvm_disassemble_h

#include <uvm/lprefix.h>
#include <uvm/lua.h>
#include <uvm/lobject.h>
#include <string>
#include <uvm/uvm_proto_info.h>

namespace uvm {
	namespace decompile {

#define CC(r) (ISK((r)) ? 'K' : 'R')
#define CV(r) (ISK((r)) ? INDEXK(r) : r)

#define RK(r) (RegOrConst(f, r))

#define MAXCONSTSIZE 1024

		std::string luadec_disassemble(GluaDecompileContextP ctx, Proto* fwork, int dflag, std::string name);

		void luadec_disassembleSubFunction(GluaDecompileContextP ctx, Proto* f, int dflag, const char* funcnumstr);

		std::string RegOrConst(const Proto* f, int r);

	}
}

#endif // #ifndef uvm_disassemble_h
