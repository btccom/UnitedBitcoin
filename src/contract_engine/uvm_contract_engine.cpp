#include <contract_engine/uvm_contract_engine.hpp>
#include <util.h>

namespace uvm
{
	UvmContractEngine::UvmContractEngine(bool use_contract)
	{
        auto allow_print = gArgs.GetBoolArg("-contractprint", false);
		_scope = std::make_shared<lua::lib::UvmStateScope>(use_contract);
        if(!allow_print)
        {
			_scope->L()->out = nullptr;
			_scope->L()->err = nullptr;
        }
	}
	UvmContractEngine::~UvmContractEngine()
	{

	}

	bool UvmContractEngine::has_gas_limit() const
	{
		return _scope->get_instructions_limit() >= 0;
	}
	int64_t UvmContractEngine::gas_limit() const {
		return _scope->get_instructions_limit();
	}
	int64_t UvmContractEngine::gas_used() const
	{
		return _scope->get_instructions_executed_count();
	}
	void UvmContractEngine::set_gas_limit(int64_t gas_limit)
	{
		_scope->set_instructions_limit(gas_limit);
	}
	void UvmContractEngine::set_no_gas_limit()
	{
		set_gas_limit(-1);
	}
	void UvmContractEngine::set_gas_used(int64_t gas_used)
	{
		int *insts_executed_count = lua::lib::get_lua_state_value(_scope->L(), INSTRUCTIONS_EXECUTED_COUNT_LUA_STATE_MAP_KEY).int_pointer_value;
		if (insts_executed_count)
		{
			*insts_executed_count = gas_used;
		}
		else
		{
			// instruction not executed yet, can't chagne gas_used
		}
	}
	void UvmContractEngine::add_gas_used(int64_t delta_used)
	{
		set_gas_used(gas_used() + delta_used);
	}

	void UvmContractEngine::stop()
	{
		_scope->notify_stop();
	}

	void UvmContractEngine::add_global_bool_variable(std::string name, bool value)
	{
		_scope->add_global_bool_variable(name.c_str(), value);
	}
	void UvmContractEngine::add_global_int_variable(std::string name, int64_t value)
	{
		_scope->add_global_int_variable(name.c_str(), value);
	}
	void UvmContractEngine::add_global_string_variable(std::string name, std::string value)
	{
		_scope->add_global_string_variable(name.c_str(), value.c_str());
	}

	void UvmContractEngine::set_caller(std::string caller, std::string caller_address)
	{
		add_global_string_variable("caller", caller);
		add_global_string_variable("caller_address", caller_address);
	}

	void UvmContractEngine::set_state_pointer_value(std::string name, void *addr)
	{
		UvmStateValue statevalue;
		statevalue.pointer_value = addr;
		lua::lib::set_lua_state_value(_scope->L(), name.c_str(), statevalue, UvmStateValueType::LUA_STATE_VALUE_POINTER);
	}

	void UvmContractEngine::clear_exceptions()
	{
		lua::api::global_uvm_chain_api->clear_exceptions(_scope->L());
	}

	void UvmContractEngine::execute_contract_api_by_address(std::string contract_id, std::string method, std::string argument, std::string *result_json_string)
	{
		clear_exceptions();
		lua::lib::execute_contract_api_by_address(_scope->L(), contract_id.c_str(), method.c_str(), argument.c_str(), result_json_string);
		if (_scope->L()->force_stopping == true && _scope->L()->exit_code == LUA_API_INTERNAL_ERROR)
			throw uvm::core::UvmException("execute contract internal error");
		int exception_code = lua::lib::get_lua_state_value(_scope->L(), "exception_code").int_value;
		char* exception_msg = (char*)lua::lib::get_lua_state_value(_scope->L(), "exception_msg").string_value;
		if (exception_code > 0)
		{
			if (exception_code == UVM_API_LVM_LIMIT_OVER_ERROR)
				throw uvm::core::UvmException("execute contract out of memory");
			else
			{
				throw uvm::core::UvmException(exception_msg);
			}
		}
	}

	void UvmContractEngine::execute_contract_init_by_address(std::string contract_id, std::string argument, std::string *result_json_string)
	{
		clear_exceptions();
		lua::lib::execute_contract_init_by_address(_scope->L(), contract_id.c_str(), argument.c_str(), result_json_string);
		if (_scope->L()->force_stopping == true && _scope->L()->exit_code == LUA_API_INTERNAL_ERROR)
			throw uvm::core::UvmException("execute contract internal error");
		int exception_code = lua::lib::get_lua_state_value(_scope->L(), "exception_code").int_value;
		char* exception_msg = (char*)lua::lib::get_lua_state_value(_scope->L(), "exception_msg").string_value;
		if (exception_code > 0)
		{
			if (exception_code == UVM_API_LVM_LIMIT_OVER_ERROR)
				throw uvm::core::UvmException("execute contract out of memory");
			else
			{
				throw uvm::core::UvmException(exception_msg);
			}
		}
	}

	void UvmContractEngine::load_and_run_stream(void *stream)
	{
		lua::lib::run_compiled_bytestream(_scope->L(), (UvmModuleByteStream*)stream);
	}

	std::shared_ptr<::blockchain::contract_engine::VMModuleByteStream> UvmContractEngine::get_bytestream_from_code(const uvm::blockchain::Code& code)
	{
		auto code_stream = lua::api::global_uvm_chain_api->get_bytestream_from_code(_scope->L(), code);
		return code_stream;
	}

}
