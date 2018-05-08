#include <contract_engine/native_contract.hpp>
#include <policy/policy.h>
#include <base58.h>
#include <script/standard.h>
#include <memory>
#include <cstdio>
#include <boost/lexical_cast.hpp>

namespace blockchain {
    namespace contract {
#define THROW_CONTRACT_ERROR(...) FC_ASSERT(false, __VA_ARGS__)

        bool native_contract_finder::has_native_contract_with_key(const std::string& key)
        {
			std::vector<std::string> native_contract_keys = {
					dgp_native_contract::native_contract_key()
            };
            return std::find(native_contract_keys.begin(), native_contract_keys.end(), key) != native_contract_keys.end();
        }
        std::shared_ptr<abstract_native_contract> native_contract_finder::create_native_contract_by_key(
			blockchain::contract::PendingState* pending_state, const std::string& key, const std::string& contract_address, const native_contract_sender& sender)
        {
            if (key == dgp_native_contract::native_contract_key()) {
				return std::make_shared<dgp_native_contract>(pending_state, contract_address, sender);
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
                change.storage_diff = diff->value();
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
                change.storage_diff = diff->value();
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

		void abstract_native_contract::set_error(int32_t error_code, const std::string& error_msg)
		{
			_contract_exec_result.exit_code = error_code;
			_contract_exec_result.error_message = error_msg;
		}

        bool abstract_native_contract::has_api(const std::string& api_name)
        {
            const auto& api_names = apis();
            return api_names.find(api_name) != api_names.end();
        }

		// dgp native contract
		std::string dgp_native_contract::contract_key() const
		{
			return dgp_native_contract::native_contract_key();
		}
		std::string dgp_native_contract::contract_address() const {
			return contract_id;
		}
		std::set<std::string> dgp_native_contract::apis() const {
			return { "init", "create_change_admin_proposal", "cancel_change_admin_proposal", "vote_admin", "admins",
				"current_change_admin_proposal", "current_change_params_proposal",
				"vote_change_param", "min_gas_price", "set_min_gas_price", "block_gas_limit", "set_block_gas_limit",
				"min_gas_count", "set_min_gas_count", "max_contract_bytecode_store_fee_gas_count", "set_max_contract_bytecode_store_fee_gas_count",
				"on_destroy", "on_upgrade", "on_deposit" };
		}
		std::set<std::string> dgp_native_contract::offline_apis() const {
			return {"admins", "current_change_admin_proposal", "current_change_params_proposal",
				"min_gas_price", "block_gas_limit", "min_gas_count", "max_contract_bytecode_store_fee_gas_count"};
		}
		std::set<std::string> dgp_native_contract::events() const {
			return {};
		}

		std::string dgp_native_contract::init_api(const std::string& api_name, const std::string& api_arg) {
			jsondiff::JsonArray admins;
			admins.push_back(sender.caller_address);
			set_contract_storage(contract_id, "admins", admins);
			jsondiff::JsonObject dgp_params;
			dgp_params["min_gas_price"] = DEFAULT_MIN_GAS_PRICE;
			dgp_params["block_gas_limit"] = DEFAULT_BLOCK_GAS_LIMIT;
			dgp_params["min_gas_count"] = DEFAULT_MIN_GAS_COUNT;
			dgp_params["max_contract_bytecode_store_fee_gas_count"] = MAX_CONTRACT_BYTECODE_STORE_FEE_GAS;
			set_contract_storage(contract_id, "dgp_params", dgp_params);
			set_contract_storage(contract_id, "current_change_admin_proposal", jsondiff::JsonValue());
			set_contract_storage(contract_id, "current_change_params_proposal", jsondiff::JsonValue());
			return "";
		}

		std::string dgp_native_contract::admins_api(const std::string& api_name, const std::string& api_arg) {
			const auto& admins = get_contract_storage(contract_id, "admins");
			return jsondiff::json_dumps(admins);
		}

		bool dgp_native_contract::is_address_in_admins(const jsondiff::JsonArray& admins, const std::string& addr) const
		{
			for (size_t i = 0; i < admins.size(); i++) {
				if (addr == admins[i].as_string()) {
					return true;
				}
			}
			return false;
		}

		std::string dgp_native_contract::create_change_admin_proposal_api(const std::string& api_name, const std::string& api_arg) {
			// only admin can call this api
			// api_arg: {address: string, add: bool, needAgreeCount: int}
			const auto& admins = get_contract_storage(contract_id, "admins").as<jsondiff::JsonArray>();
			if (!is_address_in_admins(admins, sender.caller_address)) {
				set_error(1, "only admin can call this api");
				return "";
			}
			const auto& current_change_admin_proposal = get_contract_storage(contract_id, "current_change_admin_proposal");
			if (!current_change_admin_proposal.is_null()) {
				set_error(1, "can't create new change-admin-proposal when a proposal already exists");
				return "";
			}
			std::string addr;
			bool add = false;
			uint64_t needAgreeCount = 1;
			try {
				const auto& arg_json = jsondiff::json_loads(api_arg);
				if (!arg_json.is_object())
					throw ::contract::storage::ContractStorageException("arg format error");
				const auto& arg_obj = arg_json.as<jsondiff::JsonObject>();
				if(arg_obj.find("address") == arg_obj.end() || !arg_obj["address"].is_string())
					throw ::contract::storage::ContractStorageException("arg format error");
				addr = arg_obj["address"].as_string();
				if (arg_obj.find("add") == arg_obj.end() || !arg_obj["add"].is_bool())
					throw ::contract::storage::ContractStorageException("arg format error");
				add = arg_obj["add"].as_bool();
				if (arg_obj.find("needAgreeCount") == arg_obj.end() || !arg_obj["needAgreeCount"].is_uint64())
					throw ::contract::storage::ContractStorageException("arg format error");
				needAgreeCount = arg_obj["needAgreeCount"].as_uint64();
			}
			catch (const std::exception& e) {
				set_error(1, "argument format error, need format: {address: string, add: bool, needAgreeCount: int}");
				return "";
			}
			if (add && is_address_in_admins(admins, addr)) {
				set_error(1, "this address is already admin, can't add again");
				return "";
			}
			if (!add && !is_address_in_admins(admins, addr)) {
				set_error(1, "this address is not admin, can't remove it");
				return "";
			}
			if (!add && admins.size() < 2) {
				set_error(1, "Can't remove admin if there is only one admin");
				return "";
			}
			CTxDestination addrDest = DecodeDestination(addr);
			if (!IsValidDestination(addrDest)) {
				set_error(1, "argument address format error");
				return "";
			}
			if (needAgreeCount<1 || needAgreeCount > admins.size())
			{
				set_error(1, "argument needAgreeCount must between 1 and admins count");
				return "";
			}
			if (admins.size() >= 2 && needAgreeCount < 2) {
				set_error(1, "argument needAgreeCount must between 2 and admins count when admins count >= 2");
				return "";
			}
			jsondiff::JsonObject proposal;
			proposal["creator"] = sender.caller_address;
			proposal["address"] = addr;
			proposal["add"] = add;
			proposal["needAgreeCount"] = needAgreeCount;
			proposal["votes"] = jsondiff::JsonObject();
			proposal["proposalStartBlockNumber"] = sender.block_number + 10; // only can vote when >= this block number
			set_contract_storage(contract_id, "current_change_admin_proposal", proposal);
			emit_event(contract_id, "AddedAdminChangeProposal", api_arg);
			return "";
		}
		std::string dgp_native_contract::vote_admin_api(const std::string& api_name, const std::string& api_arg) {
			
			auto admins = get_contract_storage(contract_id, "admins").as<jsondiff::JsonArray>();
			if (!is_address_in_admins(admins, sender.caller_address)) {
				set_error(1, "only admin can call this api");
				return "";
			}
			const auto& current_change_admin_proposal_json = get_contract_storage(contract_id, "current_change_admin_proposal");
			if (!current_change_admin_proposal_json.is_object()) {
				set_error(1, "change-admin-proposal not found to vote");
				return "";
			}
			auto current_change_admin_proposal = current_change_admin_proposal_json.as<jsondiff::JsonObject>();
			if (api_arg != "true" && api_arg != "false") {
				set_error(1, "argument format error, need format is: true or false");
				return "";
			}
			auto agree = ("true" == api_arg);
			bool add = current_change_admin_proposal["add"].as_bool();
			const auto& addr = current_change_admin_proposal["address"].as_string();
			vote_for_proposal(current_change_admin_proposal, agree, admins.size(),
				[&](const jsondiff::JsonObject&) {
				if (add) {
					admins.push_back(addr);
					set_contract_storage(contract_id, "admins", admins);
					set_contract_storage(contract_id, "current_change_admin_proposal", jsondiff::JsonValue()); // clear current admin-change proposal
					emit_event(contract_id, "AddedAdmin", addr);
				}
				else {
					// clear current admin-change and params-change proposal because of admins count's reduction
					jsondiff::JsonArray new_admins;
					for (const auto& item : admins) {
						if (item.as_string() != addr)
							new_admins.push_back(item.as_string());
					}
					set_contract_storage(contract_id, "admins", new_admins);
					set_contract_storage(contract_id, "current_change_admin_proposal", jsondiff::JsonValue());
					set_contract_storage(contract_id, "current_change_params_proposal", jsondiff::JsonValue());
					emit_event(contract_id, "RemovedAdmin", addr);
				}
			},
				[&](const jsondiff::JsonObject&) {
				set_contract_storage(contract_id, "current_change_admin_proposal", jsondiff::JsonValue());
				emit_event(contract_id, "VoteFailed", "");
			},
				[&](const jsondiff::JsonObject& proposal) {
				set_contract_storage(contract_id, "current_change_admin_proposal", proposal);
				emit_event(contract_id, "VoteUpdated", api_arg);
			});
			return "";
		}

		static jsondiff::JsonArray make_json_pair(const jsondiff::JsonValue& left, const jsondiff::JsonValue& right)
		{
			jsondiff::JsonArray result;
			result.push_back(left);
			result.push_back(right);
			return result;
		}

		static jsondiff::JsonArray parse_json_object_to_json_array(const jsondiff::JsonObject& obj) {
			jsondiff::JsonArray result;
			for (const auto& p : obj) {
				const auto& value = p.value();
				if (value.is_object()) {
					result.push_back(make_json_pair(p.key(), parse_json_object_to_json_array(value.as<jsondiff::JsonObject>())));
				}
				else {
					result.push_back(make_json_pair(p.key(), p.value()));
				}
			}
			return result;
		}

		std::string dgp_native_contract::current_change_admin_proposal_api(const std::string& api_name, const std::string& api_arg) {
			const auto& current_change_admin_proposal_json = get_contract_storage(contract_id, "current_change_admin_proposal");
			if (!current_change_admin_proposal_json.is_object()) {
				return "null";
			}
			const auto& current_change_admin_proposal = current_change_admin_proposal_json.as<jsondiff::JsonObject>();
			const auto& result = parse_json_object_to_json_array(current_change_admin_proposal);
			return jsondiff::json_dumps(result);
		}

		std::string dgp_native_contract::current_change_params_proposal_api(const std::string& api_name, const std::string& api_arg) {
			const auto& proposal_json = get_contract_storage(contract_id, "current_change_params_proposal");
			if (!proposal_json.is_object()) {
				return "null";
			}
			const auto& proposal = proposal_json.as<jsondiff::JsonObject>();
			const auto& result = parse_json_object_to_json_array(proposal);
			return jsondiff::json_dumps(result);
		}

		std::string dgp_native_contract::cancel_change_admin_proposal_api(const std::string& api_name, const std::string& api_arg)
		{
			const auto& current_change_admin_proposal_json = get_contract_storage(contract_id, "current_change_admin_proposal");
			if (!current_change_admin_proposal_json.is_object()) {
				set_error(1, "there is no change-admin-proposal now");
				return "";
			}
			const auto& current_change_admin_proposal = current_change_admin_proposal_json.as<jsondiff::JsonObject>();
			const auto& proposal_creator = current_change_admin_proposal["creator"].as_string();
			if (sender.caller_address != proposal_creator) {
				set_error(1, "only proposal creator can cancel it");
				return "";
			}
			set_contract_storage(contract_id, "current_change_admin_proposal", jsondiff::JsonValue());
			return "";
		}

		void dgp_native_contract::vote_for_proposal(jsondiff::JsonObject& proposal, bool agree, uint32_t admins_count,
			std::function<void(const jsondiff::JsonObject&)> proposal_success_handler,
			std::function<void(const jsondiff::JsonObject&)> proposal_fail_handler,
			std::function<void(const jsondiff::JsonObject&)> vote_handler)
		{
			auto votes = proposal["votes"].as<jsondiff::JsonObject>();
			if (votes.find(sender.caller_address) != votes.end()) {
				set_error(1, "you voted this proposal before");
				return;
			}
			auto proposalStartBlockNumber = proposal["proposalStartBlockNumber"].as_int64();
			if (sender.block_number < proposalStartBlockNumber) {
				set_error(1, "This proposal has not been effective and cannot vote");
				return;
			}
			votes[sender.caller_address] = agree;
			proposal["votes"] = votes;
			// check whether proposal ended
			int32_t agree_count = 0;
			int32_t disagree_count = 0;
			for (const auto& p : votes) {
				bool item = p.value().as_bool();
				if (item)
					agree_count++;
				else
					disagree_count++;
			}
			auto needAgreeCount = proposal["needAgreeCount"].as_uint64();

			if (agree_count >= needAgreeCount) {
				// vote successfully
				proposal_success_handler(proposal);
			}
			else if (disagree_count > admins_count - needAgreeCount) {
				// vote failed
				proposal_fail_handler(proposal);
			}
			else {
				// update votes
				vote_handler(proposal);
			}
		}

		std::string dgp_native_contract::vote_change_param_api(const std::string& api_name, const std::string& api_arg)
		{
			const auto& admins = get_contract_storage(contract_id, "admins").as<jsondiff::JsonArray>();
			if (!is_address_in_admins(admins, sender.caller_address)) {
				set_error(1, "only admin can call this api");
				return "";
			}
			const auto& current_change_params_proposal_json = get_contract_storage(contract_id, "current_change_params_proposal");
			if (!current_change_params_proposal_json.is_object()) {
				set_error(1, "can't find a change-params-proposal");
				return "";
			}
			if (api_arg != "true" && api_arg != "false") {
				set_error(1, "argument format error, need format is: true or false");
				return "";
			}
			auto agree = ("true" == api_arg);
			auto current_change_params_proposal = current_change_params_proposal_json.as<jsondiff::JsonObject>();

			std::string property_name = current_change_params_proposal["property"].as_string();
			const auto& property_value = current_change_params_proposal["value"];

			vote_for_proposal(current_change_params_proposal, agree, admins.size(),
				[&](const jsondiff::JsonObject& proposal) {
				set_dgp_param(property_name, property_value);
				set_contract_storage(contract_id, "current_change_params_proposal", jsondiff::JsonValue());
				emit_event(contract_id, "VoteSuccess", property_name);
			},
				[&](const jsondiff::JsonObject& proposal) {
				set_contract_storage(contract_id, "current_change_params_proposal", jsondiff::JsonValue());
				emit_event(contract_id, "ParamsChangeVoteFailed", property_name);
			},
				[&](const jsondiff::JsonObject& proposal) {
				set_contract_storage(contract_id, "current_change_params_proposal", proposal);
				emit_event(contract_id, "ParamsChangeVoteUpdated", api_arg);
			});
			return "";
		}

		std::string dgp_native_contract::get_dgp_param_json_string(const std::string& param_name)
		{
			const auto& dgp_params = get_contract_storage(contract_id, "dgp_params").as<jsondiff::JsonObject>();
			if (dgp_params.find(param_name) == dgp_params.end())
				return jsondiff::json_dumps(jsondiff::JsonValue());
			else
				return jsondiff::json_dumps(dgp_params[param_name]);
		}

		static const std::map<std::string, DgpChangeIntParamType> dgp_params_int_mapping = {
			{ "min_gas_price", DgpChangeIntParamType::DGP_MIN_GAS_PRICE_CHANGE_ITEM },
			{ "block_gas_limit", DgpChangeIntParamType::DGP_BLOCK_GAS_LIMIT_CHANGE_ITEM },
			{ "min_gas_count", DgpChangeIntParamType::DGP_MIN_GAS_COUNT_CHANGE_ITEM },
			{ "max_contract_bytecode_store_fee_gas_count", DgpChangeIntParamType::DGP_MAX_CONTRACT_BYTECODE_STORE_FEE_GAS_COUNT_CHANGE_ITEM }
		};

		void dgp_native_contract::set_dgp_param(const std::string& param_name, const jsondiff::JsonValue& value)
		{
			auto dgp_params = get_contract_storage(contract_id, "dgp_params").as<jsondiff::JsonObject>();
			dgp_params[param_name] = value;
			set_contract_storage(contract_id, "dgp_params", dgp_params);
			if (dgp_params_int_mapping.find(param_name) != dgp_params_int_mapping.end() && value.is_integer()) {
				auto& int_param_type = dgp_params_int_mapping.at(param_name);
				_contract_exec_result.dgp_int_params_changes[int_param_type] = value.as_int64();
			}
		}

		std::string dgp_native_contract::min_gas_price_api(const std::string& api_name, const std::string& api_arg)
		{
			const auto& value_json_str = get_dgp_param_json_string("min_gas_price");
			return value_json_str;
		}
		std::string dgp_native_contract::block_gas_limit_api(const std::string& api_name, const std::string& api_arg) {
			const auto& value_json_str = get_dgp_param_json_string("block_gas_limit");
			return value_json_str;
		}
		std::string dgp_native_contract::min_gas_count_api(const std::string& api_name, const std::string& api_arg) {
			const auto& value_json_str = get_dgp_param_json_string("min_gas_count");
			return value_json_str;
		}

		std::string dgp_native_contract::max_contract_bytecode_store_fee_gas_count_api(const std::string& api_name, const std::string& api_arg) {
			const auto& value_json_str = get_dgp_param_json_string("max_contract_bytecode_store_fee_gas_count");
			return value_json_str;
		}

		bool dgp_native_contract::create_int_param_proposal(const std::string& property_name, const std::string& api_arg, uint32_t admins_count)
		{
			// argument: integer_value,needAgreeCount
			const auto& current_change_params_proposal = get_contract_storage(contract_id, "current_change_params_proposal");
			if (!current_change_params_proposal.is_null()) {
				set_error(1, "can't create new change-params-proposal when a proposal already exists");
				return false;
			}
			int64_t value = 0;
			uint32_t needAgreeCount = 0;
			{
				auto pos = api_arg.find_first_of(',');
				if (pos <= 0) {
					set_error(1, "argument format error, need format is integer_value,needAgreeCount");
					return false;
				}
				const auto& value_str = api_arg.substr(0, pos);
				const auto& need_agree_count_str = api_arg.substr(pos + 1);
				try {
					value = boost::lexical_cast<int64_t>(value_str);
					needAgreeCount = boost::lexical_cast<uint32_t>(need_agree_count_str);
					if (needAgreeCount<1 || needAgreeCount > admins_count)
						throw ::contract::storage::ContractStorageException("need agree count illegal");
					if (admins_count >= 2 && needAgreeCount < 2)
						throw ::contract::storage::ContractStorageException("need agree count illegal");
				}
				catch (const std::exception& e) {
					set_error(1, "argument format error, need format is integer_value,needAgreeCount");
					return false;
				}
			}
			const auto& dgp_params = get_contract_storage(contract_id, "dgp_params").as<jsondiff::JsonObject>();
			if (dgp_params[property_name].as_int64() == value) {
				set_error(1, "param value not changed, proposal failed");
				return false;
			}
			jsondiff::JsonObject proposal;
			proposal["creator"] = sender.caller_address;
			proposal["property"] = property_name;
			proposal["needAgreeCount"] = needAgreeCount;
			proposal["value"] = value;
			proposal["votes"] = jsondiff::JsonObject();
			proposal["proposalStartBlockNumber"] = sender.block_number + 10; // only can vote when >= this block number
			set_contract_storage(contract_id, "current_change_params_proposal", proposal);
			emit_event(contract_id, "AddedChangeParamsProposal", api_arg);
			return true;
		}

		std::string dgp_native_contract::set_min_gas_price_api(const std::string& api_name, const std::string& api_arg)
		{
			// argument: integer_value,needAgreeCount
			const auto& admins = get_contract_storage(contract_id, "admins").as<jsondiff::JsonArray>();
			if (!is_address_in_admins(admins, sender.caller_address)) {
				set_error(1, "only admin can call this api");
				return "";
			}
			create_int_param_proposal("min_gas_price", api_arg, admins.size());
			return "";
		}

		std::string dgp_native_contract::set_block_gas_limit_api(const std::string& api_name, const std::string& api_arg)
		{
			// argument: integer_value,needAgreeCount
			const auto& admins = get_contract_storage(contract_id, "admins").as<jsondiff::JsonArray>();
			if (!is_address_in_admins(admins, sender.caller_address)) {
				set_error(1, "only admin can call this api");
				return "";
			}
			create_int_param_proposal("block_gas_limit", api_arg, admins.size());
			return "";
		}

		std::string dgp_native_contract::set_min_gas_count_api(const std::string& api_name, const std::string& api_arg)
		{
			// argument: integer_value,needAgreeCount
			const auto& admins = get_contract_storage(contract_id, "admins").as<jsondiff::JsonArray>();
			if (!is_address_in_admins(admins, sender.caller_address)) {
				set_error(1, "only admin can call this api");
				return "";
			}
			create_int_param_proposal("min_gas_count", api_arg, admins.size());
			return "";
		}

		std::string dgp_native_contract::set_max_contract_bytecode_store_fee_gas_count_api(const std::string& api_name, const std::string& api_arg) {
			// argument: integer_value,needAgreeCount
			const auto& admins = get_contract_storage(contract_id, "admins").as<jsondiff::JsonArray>();
			if (!is_address_in_admins(admins, sender.caller_address)) {
				set_error(1, "only admin can call this api");
				return "";
			}
			create_int_param_proposal("max_contract_bytecode_store_fee_gas_count", api_arg, admins.size());
			return "";
		}

		ContractExecResult dgp_native_contract::invoke(const std::string& api_name, const std::string& api_arg) {
			ContractExecResult result;
			std::map<std::string, std::function<std::string (const std::string&, const std::string&)> > apis_mapping = {
				{ "init", std::bind(&dgp_native_contract::init_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "admins", std::bind(&dgp_native_contract::admins_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "create_change_admin_proposal", std::bind(&dgp_native_contract::create_change_admin_proposal_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "vote_admin", std::bind(&dgp_native_contract::vote_admin_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "current_change_admin_proposal", std::bind(&dgp_native_contract::current_change_admin_proposal_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "current_change_params_proposal", std::bind(&dgp_native_contract::current_change_params_proposal_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "cancel_change_admin_proposal", std::bind(&dgp_native_contract::cancel_change_admin_proposal_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "vote_change_param", std::bind(&dgp_native_contract::vote_change_param_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "min_gas_price", std::bind(&dgp_native_contract::min_gas_price_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "set_min_gas_price", std::bind(&dgp_native_contract::set_min_gas_price_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "block_gas_limit", std::bind(&dgp_native_contract::block_gas_limit_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "set_block_gas_limit", std::bind(&dgp_native_contract::set_block_gas_limit_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "min_gas_count", std::bind(&dgp_native_contract::min_gas_count_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "set_min_gas_count", std::bind(&dgp_native_contract::set_min_gas_count_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "max_contract_bytecode_store_fee_gas_count", std::bind(&dgp_native_contract::max_contract_bytecode_store_fee_gas_count_api, this, std::placeholders::_1, std::placeholders::_2) },
				{ "set_max_contract_bytecode_store_fee_gas_count", std::bind(&dgp_native_contract::set_max_contract_bytecode_store_fee_gas_count_api, this, std::placeholders::_1, std::placeholders::_2) }
			};
			if (apis_mapping.find(api_name) != apis_mapping.end()) {
				result.api_result = apis_mapping[api_name](api_name, api_arg);
			} else {
				result.exit_code = 1;
				result.error_message = std::string("Can't find dgp api ") + api_name;
				return result;
			}

			merge_storage_changes_to_exec_result();
			result.contract_storage_changes = _contract_exec_result.contract_storage_changes;
			return result;
		}

    }
}