#pragma once

#include <string>
#include <memory>
#include <map>
#include <validation.h>
#include <amount.h>
#include <contract_engine/pending_state.hpp>
#include <uvm/uvm_api.h>
#include <uvm/uvm_lib.h>
#include <uvm/uvm_lutil.h>

namespace blockchain {
    namespace contract {

		using namespace jsondiff;

		struct StorageDataChangeType
		{
			JsonValue storage_diff;
			JsonValue before;
			JsonValue after;
		};

		struct native_contract_sender {
			std::string caller_address;
			int32_t block_number = 0;
		};


        class abstract_native_contract
        {
        protected:
            blockchain::contract::PendingState* _pending_state;
            std::string contract_id;
			native_contract_sender sender;
            ContractExecResult _contract_exec_result;
			std::map<std::string, std::map<std::string, StorageDataChangeType>> _contract_storage_changes;
        public:
            abstract_native_contract(blockchain::contract::PendingState* pending_state, const std::string& _contract_id, const native_contract_sender& _sender)
				: _pending_state(pending_state), contract_id(_contract_id), sender(_sender) {}
            virtual ~abstract_native_contract() {}

            // unique key to identify native contract
            virtual std::string contract_key() const = 0;
            virtual std::string contract_address() const = 0;
            virtual std::set<std::string> apis() const = 0;
            virtual std::set<std::string> offline_apis() const = 0;
            virtual std::set<std::string> events() const = 0;

            virtual ContractExecResult invoke(const std::string& api_name, const std::string& api_arg) = 0;

            virtual CAmount gas_count_for_api_invoke(const std::string& api_name) const
            {
                return 100; // now all native api call requires 100 gas count
            }
            bool has_api(const std::string& api_name);

            void set_contract_storage(const std::string& contract_address, const std::string& storage_name, const JsonValue& value);
			JsonValue get_contract_storage(const std::string& contract_address, const std::string& storage_name);
            void emit_event(const std::string& contract_address, const std::string& event_name, const std::string& event_arg);
		protected:
			void merge_storage_changes_to_exec_result();

			void set_error(int32_t error_code, const std::string& error_msg);
        };

        class native_contract_finder
        {
        public:
            static bool has_native_contract_with_key(const std::string& key);
            static std::shared_ptr<abstract_native_contract> create_native_contract_by_key(
				blockchain::contract::PendingState* pending_state, const std::string& key, const std::string& contract_address, const native_contract_sender& sender);
        };

		class dgp_native_contract final : public abstract_native_contract {
		public:
			static std::string native_contract_key() { return "dgp"; }

			dgp_native_contract(blockchain::contract::PendingState* pending_state, const std::string& _contract_id, const native_contract_sender& _sender)
				: abstract_native_contract(pending_state, _contract_id, _sender) {}
			virtual ~dgp_native_contract() {}
			virtual std::string contract_key() const;
			virtual std::string contract_address() const;
			virtual std::set<std::string> apis() const;
			virtual std::set<std::string> offline_apis() const;
			virtual std::set<std::string> events() const;

			virtual ContractExecResult invoke(const std::string& api_name, const std::string& api_arg);

		private:
			std::string init_api(const std::string& api_name, const std::string& api_arg);
			std::string admins_api(const std::string& api_name, const std::string& api_arg);
			std::string create_change_admin_proposal_api(const std::string& api_name, const std::string& api_arg);
			std::string vote_admin_api(const std::string& api_name, const std::string& api_arg);
			std::string current_change_admin_proposal_api(const std::string& api_name, const std::string& api_arg);
			std::string current_change_params_proposal_api(const std::string& api_name, const std::string& api_arg);
			std::string cancel_change_admin_proposal_api(const std::string& api_name, const std::string& api_arg);

			std::string vote_change_param_api(const std::string& api_name, const std::string& api_arg);

			std::string min_gas_price_api(const std::string& api_name, const std::string& api_arg);
			std::string set_min_gas_price_api(const std::string& api_name, const std::string& api_arg);
			std::string block_gas_limit_api(const std::string& api_name, const std::string& api_arg);
			std::string set_block_gas_limit_api(const std::string& api_name, const std::string& api_arg);
			std::string min_gas_count_api(const std::string& api_name, const std::string& api_arg);
			std::string set_min_gas_count_api(const std::string& api_name, const std::string& api_arg);
			std::string max_contract_bytecode_store_fee_gas_count_api(const std::string& api_name, const std::string& api_arg);
			std::string set_max_contract_bytecode_store_fee_gas_count_api(const std::string& api_name, const std::string& api_arg);
			
			bool is_address_in_admins(const jsondiff::JsonArray& admins, const std::string& addr) const;
			std::string get_dgp_param_json_string(const std::string& param_name);
			void set_dgp_param(const std::string& param_name, const jsondiff::JsonValue& value);
			void vote_for_proposal(jsondiff::JsonObject& proposal, bool agree, uint32_t admins_count,
				std::function<void (const jsondiff::JsonObject&)> proposal_success_handler,
				std::function<void (const jsondiff::JsonObject&)> proposal_fail_handler,
				std::function<void (const jsondiff::JsonObject&)> vote_handler);
			bool create_int_param_proposal(const std::string& property_name, const std::string& api_arg, uint32_t admins_count);
		};

    }
}