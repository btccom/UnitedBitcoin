#include <contract_engine/pending_state.hpp>

namespace blockchain {
    namespace contract {
        PendingState::PendingState(::contract::storage::ContractStorageService* _storage_service)
        {
			this->storage_service = _storage_service;
        }
    }
}