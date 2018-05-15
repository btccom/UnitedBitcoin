#include <contract_engine/pending_state.hpp>

namespace blockchain {
    namespace contract {
        PendingState::PendingState(::contract::storage::ContractStorageService* _storage_service)
        {
			this->storage_service = _storage_service;
        }

        void PendingState::add_balance_change(const std::string& address, bool is_contract, bool add, uint64_t amount)
        {
            for(auto it=balance_changes.begin(); it!=balance_changes.end();it++) {
                auto& transfer_info = *it;
                if(transfer_info.address == address && transfer_info.is_contract == is_contract) {
                    if(transfer_info.add == add) {
                        transfer_info.amount = transfer_info.amount + amount;
                    } else {
                        if(transfer_info.amount > amount) {
                            transfer_info.amount -= amount;
                        } else if(transfer_info.amount < amount) {
                            transfer_info.add = !transfer_info.add;
                            transfer_info.amount = amount - transfer_info.amount;
                        } else {
                            // transfer_info.amount == amount
                            it = balance_changes.erase(it);
                        }
                    }
                    return;
                }
            }
            ContractResultTransferInfo transfer_info;
            transfer_info.address = address;
            transfer_info.is_contract = is_contract;
            transfer_info.add = add;
            transfer_info.amount = amount;
            balance_changes.push_back(transfer_info);
        }

		uint64_t PendingState::get_contract_balance(const std::string& address) const
		{
			const auto& balances = storage_service->get_contract_balances(address);
			uint64_t balance = 0;
			for (const auto& p : balances) {
				if (p.asset_id == 0) {
					balance = p.amount;
					break;
				}
			}
			for (const auto& transfer_info : balance_changes) {
				if (transfer_info.is_contract && transfer_info.address == address) {
					if (transfer_info.add) {
						balance += transfer_info.amount;
					}
					else {
						if (balance < transfer_info.amount)
						{
							return 0;
						}
						balance -= transfer_info.amount;
					}
				}
			}
			return balance;
		}

    }
}