#include <contract_engine/contract_helper.hpp>

int gpcread(void* ptr, size_t element_size, size_t count, GpcBuffer* gpc_buffer)
{
    if(!ptr)
        return 0;
    if(gpc_buffer->eof() || ((gpc_buffer->data.size()-gpc_buffer->pos) < element_size))
        return 0;
    if(element_size*count <= (gpc_buffer->data.size()-gpc_buffer->pos))
    {
        memcpy(ptr, gpc_buffer->data.data()+gpc_buffer->pos, element_size*count);
        gpc_buffer->pos += element_size*count;
        return count;
    }
    size_t available_count = (gpc_buffer->data.size()-gpc_buffer->pos) / element_size;
    memcpy(ptr, gpc_buffer->data.data()+gpc_buffer->pos, element_size * available_count);
    gpc_buffer->pos += element_size * available_count;
    return available_count;
}

int ContractHelper::common_fread_int(GpcBuffer* fp, int* dst_int)
{
    int ret;
    unsigned char uc4, uc3, uc2, uc1;

    ret = (int)gpcread(&uc4, sizeof(unsigned char), 1, fp);
    if (ret != 1)
        return ret;
    ret = (int)gpcread(&uc3, sizeof(unsigned char), 1, fp);
    if (ret != 1)
        return ret;
    ret = (int)gpcread(&uc2, sizeof(unsigned char), 1, fp);
    if (ret != 1)
        return ret;
    ret = (int)gpcread(&uc1, sizeof(unsigned char), 1, fp);
    if (ret != 1)
        return ret;

    *dst_int = (uc4 << 24) + (uc3 << 16) + (uc2 << 8) + uc1;

    return 1;
}

int ContractHelper::common_fread_octets(GpcBuffer* fp, void* dst_stream, int len)
{
    return (int)gpcread(dst_stream, len, 1, fp);
}


#define PRINTABLE_CHAR(chr) \
if (chr >= 0 && chr <= 9)  \
    chr = chr + '0'; \
else \
    chr = chr + 'a' - 10;

std::string ContractHelper::to_printable_hex(unsigned char chr)
{
    unsigned char high = chr >> 4;
    unsigned char low = chr & 0x0F;
    char tmp[16];

    PRINTABLE_CHAR(high);
    PRINTABLE_CHAR(low);

    snprintf(tmp, sizeof(tmp), "%c%c", high, low);
    return std::string(tmp);
}

#define INIT_API_FROM_FILE(dst_set, except_1, except_2, except_3)\
{\
read_count = common_fread_int(f, &api_count); \
if (read_count != 1)\
{\
throw uvm::core::UvmException(except_1); \
}\
for (int i = 0; i < api_count; i++)\
{\
int api_len = 0; \
read_count = common_fread_int(f, &api_len); \
if (read_count != 1)\
{\
throw uvm::core::UvmException(except_2); \
}\
api_buf = (char*)malloc(api_len + 1); \
if (api_buf == NULL) \
{ \
FC_ASSERT(api_buf == NULL, "malloc fail!"); \
}\
read_count = common_fread_octets(f, api_buf, api_len); \
if (read_count != 1)\
{\
free(api_buf); \
throw uvm::core::UvmException(except_3); \
}\
api_buf[api_len] = '\0'; \
dst_set.insert(std::string(api_buf)); \
free(api_buf); \
}\
}

#define INIT_STORAGE_FROM_FILE(dst_map, except_1, except_2, except_3, except_4)\
{\
read_count = common_fread_int(f, &storage_count); \
if (read_count != 1)\
{\
throw uvm::core::UvmException(except_1); \
}\
for (int i = 0; i < storage_count; i++)\
{\
int storage_name_len = 0; \
read_count = common_fread_int(f, &storage_name_len); \
if (read_count != 1)\
{\
throw uvm::core::UvmException(except_2); \
}\
storage_buf = (char*)malloc(storage_name_len + 1); \
if (storage_buf == NULL) \
{ \
FC_ASSERT(storage_buf == NULL, "malloc fail!"); \
}\
read_count = common_fread_octets(f, storage_buf, storage_name_len); \
if (read_count != 1)\
{\
free(storage_buf); \
throw uvm::core::UvmException(except_3); \
}\
storage_buf[storage_name_len] = '\0'; \
read_count = common_fread_int(f, (int*)&storage_type); \
if (read_count != 1)\
{\
free(storage_buf); \
throw uvm::core::UvmException(except_4); \
}\
dst_map.insert(std::make_pair(std::string(storage_buf), storage_type)); \
free(storage_buf); \
}\
}

uvm::blockchain::Code ContractHelper::load_contract_from_gpc_data(const std::vector<unsigned char>& gpc_data)
{
    GpcBuffer gpc_buffer;
    gpc_buffer.data = gpc_data;
    gpc_buffer.pos = 0;
    auto f = &gpc_buffer;
    uvm::blockchain::Code code;

    unsigned int digest[5];
    int read_count = 0;
    for (int i = 0; i < 5; ++i)
    {
        read_count = common_fread_int(f, (int*)&digest[i]);
        if (read_count != 1)
        {
            throw uvm::core::UvmException("Read verify code fail!");
        }
    }

    int len = 0;
    read_count = common_fread_int(f, &len);
    if (read_count != 1 || len < 0 || (len >= (f->size() - f->pos)))
    {
        throw uvm::core::UvmException("Read bytescode len fail!");
    }

    code.code.resize(len);
    read_count = common_fread_octets(f, code.code.data(), len);
    if (read_count != 1)
    {
        throw uvm::core::UvmException("Read bytescode fail!");
    }

    boost::uuids::detail::sha1 sha;
    unsigned int check_digest[5];
    sha.process_bytes(code.code.data(), code.code.size());
    sha.get_digest(check_digest);
    if (memcmp((void*)digest, (void*)check_digest, sizeof(unsigned int) * 5))
    {
        throw uvm::core::UvmException("Verify bytescode SHA1 fail!");
    }

    for (int i = 0; i < 5; ++i)
    {
        unsigned char chr1 = (check_digest[i] & 0xFF000000) >> 24;
        unsigned char chr2 = (check_digest[i] & 0x00FF0000) >> 16;
        unsigned char chr3 = (check_digest[i] & 0x0000FF00) >> 8;
        unsigned char chr4 = (check_digest[i] & 0x000000FF);

        code.code_hash = code.code_hash + to_printable_hex(chr1) + to_printable_hex(chr2) +
                         to_printable_hex(chr3) + to_printable_hex(chr4);
    }

    int api_count = 0;
    char* api_buf = nullptr;

    INIT_API_FROM_FILE(code.abi, "read_api_count_fail", "read_api_len_fail", "read_api_fail");
    INIT_API_FROM_FILE(code.offline_abi, "read_offline_api_count_fail", "read_offline_api_len_fail", "read_offline_api_fail");
    INIT_API_FROM_FILE(code.events, "read_events_count_fail", "read_events_len_fail", "read_events_fail");

    int storage_count = 0;
    char* storage_buf = nullptr;
    uvm::blockchain::StorageValueTypes storage_type;

    INIT_STORAGE_FROM_FILE(code.storage_properties, "read_storage_count_fail", "read_storage_name_len_fail", "read_storage_name_fail", "read_storage_type_fail");

    return code;
}

std::string ContractHelper::generate_contract_address(const uvm::blockchain::Code& code, const std::string& caller_address, int32_t chain_height)
{
    return "abc"; // FIXME
}