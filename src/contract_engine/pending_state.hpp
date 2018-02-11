#pragma once

#include <validation.h>

namespace blockchain {
    namespace contract {
        struct PendingState
        {
            PendingState();
            virtual ~PendingState(){}


            std::unordered_map<std::string, ContractInfo> pending_contracts_to_create;
        };
    }
}