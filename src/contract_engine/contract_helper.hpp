#pragma once

#include <validation.h>
#include <jsondiff/jsondiff.h>
#include <jsondiff/exceptions.h>
#include <uvm/uvm_lib.h>
#include <boost/uuid/sha1.hpp>

#include <map>
#include <vector>
#include <unordered_map>
#include <string>

struct GpcBuffer
{
    std::vector<unsigned char> data;
    size_t pos = 0;

    bool eof() const{ return pos >= data.size(); }
    size_t size() const {return data.size();}
};

int gpcread(void* ptr, size_t element_size, size_t count, GpcBuffer* gpc_buffer);

class ContractHelper
{
public:
    static int common_fread_int(GpcBuffer* fp, int* dst_int);
    static int common_fread_octets(GpcBuffer* fp, void* dst_stream, int len);
    static std::string to_printable_hex(unsigned char chr);
    static uvm::blockchain::Code load_contract_from_gpc_data(const std::vector<unsigned char>& gpc_data);

    static std::string generate_contract_address(const uvm::blockchain::Code& code, const std::string& caller_address, const CTransaction& txBitcoin, size_t contract_op_vout_index);
    static bool is_valid_contract_address_format(const std::string& address);
};