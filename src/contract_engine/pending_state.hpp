#pragma once

#include <validation.h>
#include <contract_storage/contract_storage.hpp>
#include <amount.h>


namespace blockchain {
    namespace contract {
        struct PendingState
        {
            PendingState(::contract::storage::ContractStorageService* _storage_service);
            ~PendingState(){}

			::contract::storage::ContractStorageService* storage_service;
            uint256 tx_id;
            CAmount nTxFee;

            std::unordered_map<std::string, ContractInfo> pending_contracts_to_create;
			std::vector<std::pair<std::string, StorageChanges>> contract_storage_changes; // contract_id => changes
            std::vector<ContractResultTransferInfo> balance_changes;
            std::vector<ContractBaseInfoForUpdate> contract_upgrade_infos;
			std::vector<::contract::storage::ContractEventInfo> events;

            void add_balance_change(const std::string& address, bool is_contract, bool add, uint64_t amount);

			uint64_t get_contract_balance(const std::string& address) const;
        };
    }
}