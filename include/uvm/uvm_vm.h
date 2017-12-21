#ifndef uvm_vm_h
#define uvm_vm_h

#include <uvm/lprefix.h>
#include <memory>

namespace uvm
{
	namespace vm
	{

		class VMCallinfo
		{
			
		};
		
		class VMState
		{
			
		};

		typedef std::shared_ptr<VMState> VMStateP;

		class VM
		{
		public:
			VM();
			virtual ~VM();

			// void load();

			void execute(VMStateP state);
		};
	}
}

#endif