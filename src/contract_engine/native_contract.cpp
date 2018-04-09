#include <contract_engine/native_contract.hpp>
#include <memory>
#include <cstdio>

namespace blockchain {
    namespace contract {
#define THROW_CONTRACT_ERROR(...) FC_ASSERT(false, __VA_ARGS__)

        bool native_contract_finder::has_native_contract_with_key(const std::string& key)
        {
			// FIXME: remove the demo native contract
            std::vector<std::string> native_contract_keys = {
                    demo_native_contract::native_contract_key()
            };
            return std::find(native_contract_keys.begin(), native_contract_keys.end(), key) != native_contract_keys.end();
        }
        std::shared_ptr<abstract_native_contract> native_contract_finder::create_native_contract_by_key(blockchain::contract::PendingState* pending_state, const std::string& key, const std::string& contract_address)
        {
            if (key == demo_native_contract::native_contract_key())
            {
                return std::make_shared<demo_native_contract>(pending_state, contract_address);
            }
            else
            {
                return nullptr;
            }
            return nullptr;
        }

        void abstract_native_contract::set_contract_storage(const std::string& contract_address, const std::string& storage_name, const JsonValue& value)
        {

            if (_contract_storage_changes.find(contract_address) == _contract_storage_changes.end())
            {
				_contract_storage_changes[contract_address] = std::map<std::string, StorageDataChangeType>();
            }
            auto& storage_changes = _contract_storage_changes[contract_address];
            if (storage_changes.find(storage_name) == storage_changes.end())
            {
                StorageDataChangeType change;
                change.after = value;
				const auto &before = _pending_state->storage_service->get_contract_storage(contract_address, storage_name);
                jsondiff::JsonDiff differ;
				const auto& before_json_str = jsondiff::json_dumps(before);
				const auto& after_json_str = jsondiff::json_dumps(change.after);
				auto diff = differ.diff(before, change.after);
                change.storage_diff = jsondiff::json_dumps(diff->value()); // TODO: json_ordered_dumps
				change.before = before;
                storage_changes[storage_name] = change;
            }
            else
            {
                auto& change = storage_changes[storage_name];
                auto before = change.before;
                auto after = value;
                change.after = after;
                jsondiff::JsonDiff differ;
				const auto& before_json_str = json_dumps(before);
                auto after_json_str = jsondiff::json_dumps(after);
				auto diff = differ.diff(before, after);
                change.storage_diff = jsondiff::json_dumps(diff->value()); // TODO: json_ordered_dumps
            }
        }
		JsonValue abstract_native_contract::get_contract_storage(const std::string& contract_address, const std::string& storage_name)
        {
            if (_contract_storage_changes.find(contract_address) == _contract_storage_changes.end())
            {
                return _pending_state->storage_service->get_contract_storage(contract_address, storage_name);
            }
            auto& storage_changes = _contract_storage_changes[contract_address];
            if (storage_changes.find(storage_name) == storage_changes.end())
            {
                return _pending_state->storage_service->get_contract_storage(contract_address, storage_name);
            }
            return storage_changes[storage_name].after;
        }

        void abstract_native_contract::emit_event(const std::string& contract_address, const std::string& event_name, const std::string& event_arg)
        {
            FC_ASSERT(!event_name.empty());
			::contract::storage::ContractEventInfo info;
            info.contract_id = contract_address;
            info.event_name = event_name;
            info.event_arg = event_arg;
            _contract_exec_result.events.push_back(info);
        }

		void abstract_native_contract::merge_storage_changes_to_exec_result()
		{
			for (const auto& p : _contract_storage_changes) {
				const auto& contract_id = p.first;
				if (p.second.empty()) {
					continue;
				}
				JsonObject diff_json;
				for (const auto& p2 : p.second) {
					const auto& key = p2.first;
					const auto& change = p2.second;
					diff_json[key] = change.storage_diff;
				}
				_contract_exec_result.contract_storage_changes.push_back(std::make_pair(contract_id, std::make_shared<DiffResult>(diff_json)));
			}
		}

        bool abstract_native_contract::has_api(const std::string& api_name)
        {
            const auto& api_names = apis();
            return api_names.find(api_name) != api_names.end();
        }

		// demo native contract
		std::string demo_native_contract::contract_key() const
		{
			return demo_native_contract::native_contract_key();
		}
		std::string demo_native_contract::contract_address() const {
			return contract_id;
		}
		std::set<std::string> demo_native_contract::apis() const {
			return { "init", "hello", "contract_balance", "withdraw", "on_deposit_asset" };
		}
		std::set<std::string> demo_native_contract::offline_apis() const {
			return {};
		}
		std::set<std::string> demo_native_contract::events() const {
			return {};
		}

		ContractExecResult demo_native_contract::invoke(const std::string& api_name, const std::string& api_arg) {
			ContractExecResult result;
			printf("demo native contract called\n");
			printf("api %s called with arg %s\n", api_name.c_str(), api_arg.c_str());
			merge_storage_changes_to_exec_result();
			result.contract_storage_changes = _contract_exec_result.contract_storage_changes;
			return result;
		}

    }
}