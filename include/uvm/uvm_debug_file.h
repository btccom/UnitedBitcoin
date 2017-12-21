#ifndef uvm_debug_file_h
#define uvm_debug_file_h

#include <uvm/lprefix.h>
#include <unordered_map>
#include <map>
#include <string>
#include <vector>


namespace uvm {
	namespace lua {
		namespace core {


			// .ldb文件中的内容的数据结构
			class LuaDebugFileInfo
			{
			private:
				std::map<size_t, size_t> _source_line_mappings; // uvm源文件到lua中间文件的line信息的映射关系,这里需要排序
				std::vector<std::string> _proto_names; // 程序按深度遍历按出现顺序排列的proto的名称，匿名函数用空字符串
			public:
				LuaDebugFileInfo();
				LuaDebugFileInfo(const LuaDebugFileInfo &other);
				std::string serialize() const;
				void serialize_to_file(FILE *file) const;
				static LuaDebugFileInfo deserialize(const std::string &content);
				static LuaDebugFileInfo deserialize_from_file(FILE *file);

				void set_source_line_mapping(size_t uvm_line, size_t lua_line);
				void add_proto_name(const std::string &proto_name);

				size_t find_uvm_line_by_lua_line(size_t lua_line);
				size_t find_lua_line_by_uvm_line(size_t uvm_line);
			};

		} // end namespace
	}
}

#endif